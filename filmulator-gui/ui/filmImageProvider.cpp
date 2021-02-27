#include "filmImageProvider.h"
#include "../database/exifFunctions.h"
#include <iostream>
#include <QDir>
#include "../database/organizeModel.h"

using std::cout;
using std::endl;

#define TIMEOUT 0.1

FilmImageProvider::FilmImageProvider(ParameterManager * manager) :
    QObject(0),
    QQuickImageProvider(QQuickImageProvider::Image,
                        QQuickImageProvider::ForceAsynchronousImageLoading),
    pipeline(WithCache, WithHisto, HighQuality),
    quickPipe(WithCache, WithHisto, PreviewQuality),
    nextQuickPipe(WithCache, NoHisto, PreviewQuality),
    prevQuickPipe(WithCache, NoHisto, PreviewQuality)
{
    paramManager = manager;
    cloneParam = new ParameterManager;
    cloneParam->setClone();
    nextParam = new ParameterManager;
    prevParam = new ParameterManager;

    //Make changes to the paramManager cancel computations
    connect(paramManager, SIGNAL(updateClone(ParameterManager*)), cloneParam, SLOT(cloneParams(ParameterManager*)));
    connect(paramManager, SIGNAL(updateClone(ParameterManager*)), nextParam, SLOT(cancelComputation()));

    zeroHistogram(finalHist);
    zeroHistogram(postFilmHist);
    zeroHistogram(preFilmHist);

    worker->moveToThread(&workerThread);
    connect(this, SIGNAL(requestThumbnail(QString)), worker, SLOT(writeThumb(QString)));
    connect(worker, SIGNAL(doneWritingThumb()), this, SLOT(thumbDoneWriting()));

    //Check if we want the pipeline to cache.
    Settings settingsObject;
    if (settingsObject.getLowMemMode() == true)
    {
        pipeline.setCache(NoCache);
        useCache = false;
    }
    else
    {
        pipeline.setCache(WithCache);
        useCache = true;
    }

    previewResolution = settingsObject.getPreviewResolution();
    quickPipe.resolution = previewResolution;
    nextQuickPipe.resolution = previewResolution;
    prevQuickPipe.resolution = previewResolution;

    //Check if we want to use dual pipelines
    if (settingsObject.getQuickPreview())
    {
        useQuickPipe = true;
        pipeline.stealData = true;
        pipeline.stealVictim = &quickPipe;
    }
    else
    {
        useQuickPipe = false;
    }
}

FilmImageProvider::~FilmImageProvider()
{
}

QImage FilmImageProvider::requestImage(const QString& id,
                                       QSize *size,
                                       const QSize& /*requestedSize*/)
{
    gettimeofday(&request_start_time,NULL);
    cout << "FilmImageProvider::requestImage id: " << id.toStdString() << endl;

    //Copy out the filename.
    std::string filename;

    //Get just the file hash for file coherency purposes
    QString fileHash = id;
    fileHash.remove(0,1);
    fileHash.truncate(32);

    //Record whether to write this thumbnail
    writeThisThumbnail = thumbnailWriteEnabled;

    //Run the pipeline.
    Exiv2::ExifData data;
    matrix<unsigned short> image;
    if (!useQuickPipe)
    {
        shufflePipelines();//this'll just deal with the ID names
        filename = paramManager->getFullFilename();
        image = pipeline.processImage(paramManager, this, data, fileHash);
    }
    else
    {
        //need to check if we want the small or big image
        if (id[0] == "q")
        {
            filename = paramManager->getFullFilename();
            if (newID != currentID && useCache)//the image changed
            {
                shufflePipelines();
                if (paramManager->getValid() > Valid::none)
                {
                    quickPipe.rerunHistograms();
                }
            } else {
                shufflePipelines();
            }
            struct timeval quickTime;
            gettimeofday(&quickTime, nullptr);
            image = quickPipe.processImage(paramManager, this, data, fileHash);
            cout << "requestImage quickPipe time: " << timeDiff(quickTime) << endl;
        }
        else
        {
            //dummy stuff for the precomputation pipe
            Exiv2::ExifData exif;

            cloneParam->markStartOfProcessing();
            nextParam->markStartOfProcessing();

            //run precomputation, only if we're in high memory usage mode
            if (useCache)
            {
                struct timeval preTime;
                gettimeofday(&preTime, nullptr);
                cout << "requestImage nextParam valid " << nextParam->getValid() << endl;
                nextQuickPipe.processImage(nextParam, this, exif);
                cout << "requestImage preload time: " << timeDiff(preTime) << endl;
            }

            //run full pipeline of current image
            filename = cloneParam->getFullFilename();
            struct timeval fullTime;
            gettimeofday(&fullTime, nullptr);
            image = pipeline.processImage(cloneParam, this, data, fileHash);
            cout << "requestImage fullPipe time: " << timeDiff(fullTime) << endl;

            //Copy the high-res pipeline images back to low-res to deal with
            // softness from lens corrections or rotation
            if (image.nr() > 0 && useCache)//don't copy invalid data
            {
                quickPipe.copyAndDownsampleImages(&pipeline);
            }
        }
    }

    //make sure everything happens together
    processMutex.lock();

    //Ensure that the thumbnail writer writes matching filenames and images
    writeDataMutex.lock();
    //Prepare the exif data for output.
    exifData = data;
    //Prepare the output filename.
    outputFilename = filename.substr(0,filename.length()-4);
    outputFilename.append("-output");
    //Move the image over.
    last_image = std::move(image);
    writeDataMutex.unlock();
    processMutex.unlock();

    const int nrows = last_image.nr();
    const int ncols = last_image.nc();

    QImage output = QImage(ncols/3,nrows,QImage::Format_ARGB32);
    #pragma omp parallel for
    for(int i = 0; i < nrows; i++)
    {
        QRgb *line = (QRgb *)output.scanLine(i);
        for(int j = 0; j < ncols; j = j + 3)
        {
            *line = QColor(last_image(i,j)/256,
                           last_image(i,j+1)/256,
                           last_image(i,j+2)/256).rgb();
            line++;
        }
    }

    tout << "Request time: " << timeDiff(request_start_time) << " seconds" << endl;
    setProgress(1);
    *size = output.size();
    return output;
}

void FilmImageProvider::writeTiff()
{
    processMutex.lock();
    imwrite_tiff(last_image, outputFilename, exifData);
    processMutex.unlock();
}

void FilmImageProvider::writeJpeg()
{
    processMutex.lock();
    //Set up the thumbnail directory.
    QDir dir = QDir::home();
    dir.cd(OrganizeModel::thumbDir());
    QString thumbDir = currentID;
    thumbDir.truncate(4);
    if (!dir.cd(thumbDir))
    {
        dir.mkdir(thumbDir);
        dir.cd(thumbDir);
    }
    std::string thumbPath = dir.absoluteFilePath(currentID).toStdString();
    cout << "writejpeg current id: " << currentID.toStdString() << endl;
    thumbPath.append(".jpg");
    cout << "writejpeg thumb path: " << thumbPath << endl;
    imwrite_jpeg(last_image, outputFilename, exifData, 95, thumbPath);
    processMutex.unlock();
}

//TODO: There needs to be an OutputWriteWorker like the ThumbWriteWorker, but hq and with some
// flexibility to what stages of the pipeline get run
void FilmImageProvider::writeThumbnail(QString searchID)
{
    writeDataMutex.lock();
    if (writeThisThumbnail)//when we have the crop temporarily disabled, don't write the thumb
    {
        workerThread.start(QThread::LowPriority);
        worker->setImage(last_image, exifData);
        emit requestThumbnail(searchID);
    }
    else
    {
        emit thumbnailDone();//But tell the queue delegate that it was written so it stops waiting
    }
    writeDataMutex.unlock();
}

void FilmImageProvider::thumbDoneWriting()
{
    //clean up thread
    exitWorker();
    emit thumbnailDone();
}

void FilmImageProvider::setProgress(float percentDone_in)
{
    progress = percentDone_in;
    emit progressChanged();
}

void FilmImageProvider::updateFilmProgress(float percentDone_in)//Percent filmulation
{
    progress = 0.2 + percentDone_in*0.6;
    emit progressChanged();
}

float FilmImageProvider::getHistogramPoint(Histogram &hist, int index, int i, LogY isLog)
{
    if(hist.empty)
    {
        return 0.0f;
    }
    //index is 0 for L, 1 for R, 2 for G, and 3 for B.
    assert((index < 4) && (index >= 0));
    switch (index)
    {
    case 0: //luminance
        if (!isLog)
            return float(min(float(hist.lHist[i]),hist.lHistMax))/float(hist.lHistMax);
        else
            return float(log(hist.lHist[i]+1)/log(double(hist.lHistMax+1)));
    case 1: //red
        if (!isLog)
            return float(min(float(hist.rHist[i]),hist.rHistMax))/float(hist.rHistMax);
        else
            return float(log(hist.rHist[i]+1)/log(double(hist.rHistMax+1)));
    case 2: //green
        if (!isLog)
            return float(min(float(hist.gHist[i]),hist.gHistMax))/float(hist.gHistMax);
        else
            return float(log(hist.gHist[i]+1)/log(double(hist.gHistMax+1)));
    default://case 3: //blue
        if (!isLog)
            return float(min(float(hist.bHist[i]),hist.bHistMax))/float(hist.bHistMax);
        else
            return float(log(hist.bHist[i]+1)/log(double(hist.bHistMax+1)));
    }
    //xHistMax is the maximum height of any bin except the extremes.
}

QImage FilmImageProvider::emptyImage()
{
    return QImage(0,0,QImage::Format_ARGB32);
}

void FilmImageProvider::prepareShuffle(const QString newIDin, const QString newNextIDin)
{
    newID = newIDin;
    newNextID = newNextIDin;
    //cout << "prepareShuffle newID:     " << newID.toStdString() << endl;
    //cout << "prepareShuffle newNextID: " << newNextID.toStdString() << endl;
}

void FilmImageProvider::shufflePipelines()
{
    cout << "shuffle newID:     " << newID.toStdString() << endl;
    cout << "shuffle newNextID: " << newNextID.toStdString() << endl;
    cout << "shuffle prevID:    " << prevID.toStdString() << endl;
    cout << "shuffle currentID: " << currentID.toStdString() << endl;
    cout << "shuffle nextID:    " << nextID.toStdString() << endl;
    struct timeval shuffleTime;
    gettimeofday(&shuffleTime, nullptr);

    if (!useCache)
    {
        currentID = newID;
        return;
    }
    if (!useQuickPipe)
    {
        currentID = newID;
        return;
    }
    if (newID == "")
    {
        return;
    }
    if (newID == currentID)
    {
        return;
    }

    if (currentID == "")//If we have no currently selected image, no copying is necessary
    {
        cout << "shuffle: no current image" << endl;
        currentID = newID;
        if (newNextID != "")
        {
            nextID = newNextID;
            nextParam->selectImage(nextID);
        }
    } else if (newID == prevID)//swap new and old
    {
        cout << "shuffle: new matches old" << endl;
        //copy image data
        quickPipe.swapPipeline(&prevQuickPipe);

        //copy processing parameters and validity of computation
        cout << "shuffle: prevPipeline valid: " << prevParam->getValid() << " ==============================================================" << endl;
        cout << "shuffle: currPipeline valid: " << paramManager->getValid() << " ==============================================================" << endl;
        cout << "shuffle: currPipeline valid: " << paramManager->getValidityWhenCanceled() << " ==============================================================" << endl;
        tempValid = paramManager->getValidityWhenCanceled();//because we did selectImage the validity was canceled; we want the very latest
        paramManager->setValid(prevParam->getValid());
        //paramManager->selectImage(newID);//because we just set validity, this doesn't reset validity or notify qml
        prevParam->selectImage(currentID);
        prevParam->setValid(tempValid);
        cloneParam->setValid(Valid::none);

        //select the params for the next image for preload
        if (newNextID != "")
        {
            nextParam->selectImage(newNextID);
        }
        //cout << "shuffle: prevPipeline valid: " << prevParam->getValid() << " ==============================================================" << endl;
        //cout << "shuffle: currPipeline valid: " << paramManager->getValid() << " ==============================================================" << endl;
        //cout << "shuffle: currPipeline valid: " << paramManager->getValidityWhenCanceled() << " ==============================================================" << endl;

        //then update searchIDs (should be the same for all variants here)
        prevID = currentID;
        currentID = newID;
        nextID = newNextID;
    } else {
        cout << "shuffle: new does not match old" << endl;
        //just copy current to old to start...
        prevQuickPipe.swapPipeline(&quickPipe);
        prevParam->selectImage(currentID);
        prevParam->setValid(paramManager->getValidityWhenCanceled());

        //check whether to use our preloaded image
        if (newID == nextID)//copy the preloaded image to the current
        {
            cout << "shuffle: new matches next" << endl;
            cout << "shuffle: nextPipeline valid: " << nextParam->getValid() << " ==============================================================" << endl;
            cout << "shuffle: currPipeline valid: " << paramManager->getValid() << " ==============================================================" << endl;
            cout << "shuffle: currPipeline valid: " << paramManager->getValidityWhenCanceled() << " ==============================================================" << endl;
            quickPipe.swapPipeline(&nextQuickPipe);
            //we already selected the right image
            paramManager->setValid(nextParam->getValid());
            nextParam->setValid(Valid::none);
            cloneParam->setValid(Valid::none);
        } //else, we just let qml do the selectImage afresh

        //select the params for the next image for preload
        if (newNextID != "")
        {
            cout << "shuffle: there is a newNext" << endl;
            nextParam->selectImage(newNextID);
        }
        //cout << "shuffle: nextPipeline valid: " << nextParam->getValid() << " ==============================================================" << endl;
        //cout << "shuffle: currPipeline valid: " << paramManager->getValid() << " ==============================================================" << endl;
        //cout << "shuffle: currPipeline valid: " << paramManager->getValidityWhenCanceled() << " ==============================================================" << endl;

        //then update searchIDs (should be the same for all variants here)
        prevID = currentID;
        currentID = newID;
        nextID = newNextID;
    }
    cout << "shuffle finished duration: " << timeDiff(shuffleTime) << endl;
}

//called when settings are pasted, to ensure that the cached data gets invalidated if settings change
void FilmImageProvider::refreshParams(const QString IDin)
{
    if (prevParam->getImageIndex() == IDin)
    {
        prevParam->selectImage(IDin);
    }
    if (paramManager->getImageIndex() == IDin)
    {
        paramManager->selectImage(IDin);
    }
    if (nextParam->getImageIndex() == IDin)
    {
        nextParam->selectImage(IDin);
    }
}

//Coordinates relative to cropped image
//Directly communicates with the ParamManagers, I think.
void FilmImageProvider::customWB(const float xCoord, const float yCoord)
{
    QMutexLocker paramLocker(&(paramManager->paramMutex));
    const int rotation = paramManager->getRotation();
    const float cropHeight = paramManager->getCropHeight();
    const float cropAspect = paramManager->getCropAspect();
    const float cropVoffset = paramManager->getCropVoffset();
    const float cropHoffset = paramManager->getCropHoffset();

    float red;
    float green;
    float blue;
    pipeline.sampleWB(xCoord, yCoord,
                      rotation,
                      cropHeight, cropAspect,
                      cropVoffset, cropHoffset,
                      red, green, blue);

    float rMult = 1/red;
    float gMult = 1/green;
    float bMult = 1/blue;

    const float minMult = min(min(rMult, gMult), bMult);

    rMult /= minMult;
    gMult /= minMult;
    bMult /= minMult;

    const std::string filename = paramManager->getFullFilename();
    float temp;
    float tint;
    optimizeWBMults(filename, temp, tint, rMult, gMult, bMult);
    cout << "customWB temp: " << temp << endl;
    cout << "customWB tint: " << tint << endl;

    paramLocker.unlock();//done gathering info from the

    //limit temperature to prevent crashes when calculating planckian locus
    if (temp < 2000)
    {
        temp = 2000;
    } else if (temp > 20000)
    {
        temp = 20000;
    }
    //tint is just a multiplier for green so it won't cause crashes

    paramManager->setWB(temp, tint);

    //Now we need to tell the parametermanager to store the white balance value
    paramManager->saveCustomWb();
}
