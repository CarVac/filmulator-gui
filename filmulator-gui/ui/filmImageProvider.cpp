#include "filmImageProvider.h"
#include "../database/exifFunctions.h"
#include <iostream>

using std::cout;
using std::endl;

#define TIMEOUT 0.1

FilmImageProvider::FilmImageProvider(ParameterManager * manager) :
    QObject(0),
    QQuickImageProvider(QQuickImageProvider::Image,
                        QQuickImageProvider::ForceAsynchronousImageLoading),
    pipeline(WithCache, WithHisto, HighQuality),
    quickPipe(WithCache, WithHisto, PreviewQuality),
    nextQuickPipe(WithCache, WithHisto, PreviewQuality),
    prevQuickPipe(WithCache, WithHisto, PreviewQuality)
{
    paramManager = manager;
    cloneParam = new ParameterManager;
    cloneParam->setClone();
    connect(paramManager, SIGNAL(updateClone(ParameterManager*)), cloneParam, SLOT(cloneParams(ParameterManager*)));
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
    }
    else
    {
        pipeline.setCache(WithCache);
    }

    quickPipe.resolution = settingsObject.getPreviewResolution();
    nextQuickPipe.resolution = settingsObject.getPreviewResolution();
    prevQuickPipe.resolution = settingsObject.getPreviewResolution();

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
    cout << "FilmImageProvider::requestImage Here?" << endl;
    cout << "FilmImageProvider::requestImage id: " << id.toStdString() << endl;

    //Copy out the filename.
    std::string filename;

    //Record whether to write this thumbnail
    writeThisThumbnail = thumbnailWriteEnabled;

    //Run the pipeline.
    Exiv2::ExifData data;
    matrix<unsigned short> image;
    if (!useQuickPipe)
    {
        filename = paramManager->getFullFilename();
        image = pipeline.processImage(paramManager, this, data);
    }
    else
    {
        //need to check if we want the small or big image
        if (id[0] == "q")
        {
            filename = paramManager->getFullFilename();
            image = quickPipe.processImage(paramManager, this, data);
        }
        else
        {
            filename = cloneParam->getFullFilename();
            cout << "FilmImageProvider::requestImage filename: " << filename << endl;
            image = pipeline.processImage(cloneParam, this, data);
        }
    }

    //Ensure that the tiff and jpeg outputs don't write the previous image.
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
    imwrite_jpeg(last_image, outputFilename, exifData, 95);
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
