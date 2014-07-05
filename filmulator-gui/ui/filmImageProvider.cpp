#include "filmImageProvider.h"
#include "../database/exifFunctions.h"
#include <pwd.h>
#include <unistd.h>
#include <QTimer>
#include <cmath>
#define TIMEOUT 0.1

FilmImageProvider::FilmImageProvider(ParameterManager * manager) :
    QObject(0),
    QQuickImageProvider(QQuickImageProvider::Image,
                        QQuickImageProvider::ForceAsynchronousImageLoading)
{
    paramManager = manager;
    setHighlights( 0 );

    setExposureComp( 0.0 );

    setDevelTime( 100.0 );
    setAgitateCount( 1 );
    setDevelSteps( 12 );
    setFilmArea( 864.0 );
    setLayerMixConst( 0.2 );

    setBlackpoint( 0 );
    setWhitepoint( 0.002 );
    //defaultToneCurveEnabled = true;

    shadowsX = 0.25;
    param.shadowsX = 0.25;
    setShadowsY( 0.25 );
    highlightsX = 0.25;
    param.highlightsX = 0.25;
    setHighlightsY( 0.75 );

    setSaturation( 0 );
    setVibrance( 0 );

    setSaveTiff( false);
    setSaveJpeg( false );

    zeroHistogram(finalHist);
    //histFinalRoll++;//signal histogram updates
    zeroHistogram(postFilmHist);
    //histPostFilmRoll++;
    zeroHistogram(preFilmHist);
    //histPreFilmRoll++;

    cout << "Constructing FilmImageProvider" << endl;
}

FilmImageProvider::~FilmImageProvider()
{
}

QImage FilmImageProvider::requestImage(const QString &id,
                                       QSize *size, const QSize &requestedSize)
{
    gettimeofday(&request_start_time,NULL);
    QImage output = emptyImage();

    //Copy out the latest parameters.
    mutex.lock();
    ProcessingParameters tempParams = paramManager->getParams();
    abort = false;
    mutex.unlock();

    //Prepare the output filename.
    string outputFilename = tempParams.filenameList[0];
    outputFilename.append("-output");

    //Prepare an exif object for later applying to the output files.
    Exiv2::ExifData exif;

    //Run the pipeline.
    matrix<unsigned short> image = pipeline.processImage( tempParams, this, abort, exif );

    //Write out the images.
    if(saveJpeg)
    {
        imwrite_jpeg( image, outputFilename, exif);
        saveJpeg = false;
    }

    if(saveTiff)
    {
        imwrite_tiff( image, outputFilename, exif);
        saveTiff = false;
    }

    int nrows = image.nr();
    int ncols = image.nc();

    output = QImage(ncols/3,nrows,QImage::Format_ARGB32);
    for(int i = 0; i < nrows; i++)
    {
        QRgb *line = (QRgb *)output.scanLine(i);
        for(int j = 0; j < ncols; j = j + 3)
        {
            *line = QColor(image(i,j)/256,
                           image(i,j+1)/256,
                           image(i,j+2)/256).rgb();
            line++;
        }
    }

    tout << "Request time: " << timeDiff(request_start_time) << " seconds" << endl;
    setProgress(1);
    *size = output.size();
    return output;
}

//===After load, during demosaic===

//Automatic CA correction
void FilmImageProvider::setCaEnabled(bool caEnabledIn)
{
    QMutexLocker locker( &mutex );
    caEnabled = caEnabledIn;
    param.caEnabled = caEnabledIn;
    abort = true;
    emit caEnabledChanged();
}

//Highlight recovery parameter
void FilmImageProvider::setHighlights(int highlightsIn)
{
    QMutexLocker locker( &mutex );
    highlights = highlightsIn;
    param.highlights = highlightsIn;
    abort = true;
    emit highlightsChanged();
}

//===After demosaic, during prefilmulation===
//Exposure compensation of simulated film exposure
void FilmImageProvider::setExposureComp(float exposure)
{
    QMutexLocker locker( &mutex );
    exposureComp = exposure;
    std::vector<float> exposureList;
    exposureList.push_back( exposure );
    param.exposureComp = exposureList;
    abort = true;
    emit exposureCompChanged();
}

//Temperature of white balance correction
void FilmImageProvider::setTemperature(double temperatureIn)
{
    QMutexLocker locker( &mutex );
    temperature = temperatureIn;
    param.temperature = temperatureIn;
    abort = true;
    emit temperatureChanged();
}

//Tint of white balance correction
void FilmImageProvider::setTint(double tintIn)
{
    QMutexLocker locker( &mutex );
    tint = tintIn;
    param.tint = tintIn;
    abort = true;
    emit tintChanged();
}

//===After prefilmulation, during filmulation===
//Sets the simulated duration of film development.
void FilmImageProvider::setDevelTime(float develTimeIn)
{
    QMutexLocker locker( &mutex );
    develTime = develTimeIn;
    param.filmParams.totalDevelTime = develTimeIn;
    abort = true;
    emit develTimeChanged();
}

//Sets the number of times the tank is agitated
void FilmImageProvider::setAgitateCount(int agitateCountIn)
{
    QMutexLocker locker( &mutex );
    agitateCount = agitateCountIn;
    param.filmParams.agitateCount = agitateCountIn;
    abort = true;
    emit agitateCountChanged();
}

//Sets the number of simulation steps for development
void FilmImageProvider::setDevelSteps(int develStepsIn)
{
    QMutexLocker locker( &mutex );
    develSteps = develStepsIn;
    param.filmParams.developmentSteps = develStepsIn;
    abort = true;
    emit develStepsChanged();
}

//Sets the area of the film being simulated by filmulator
void FilmImageProvider::setFilmArea(float filmAreaIn)
{
    QMutexLocker locker( &mutex );
    filmArea = filmAreaIn;
    param.filmParams.filmArea = filmAreaIn;
    abort = true;
    emit filmAreaChanged();
}

//Sets the amount of developer mixing between the bulk
// developer reservoir and the active layer against the
// film.
void FilmImageProvider::setLayerMixConst(float layerMixConstIn)
{
    QMutexLocker locker( &mutex );
    layerMixConst = layerMixConstIn;
    param.filmParams.layerMixConst = layerMixConstIn;
    abort = true;
    emit layerMixConstChanged();
}

//===After filmulation, during whiteblack===
//Sets the black clipping point right after filmulation
void FilmImageProvider::setBlackpoint(float blackpointIn)
{
    QMutexLocker locker( &mutex );
    blackpoint = blackpointIn;
    param.blackpoint = blackpointIn;
    abort = true;
    emit blackpointChanged();
}

//Sets the white clipping point right after filmulation
void FilmImageProvider::setWhitepoint(float whitepointIn)
{
    QMutexLocker locker( &mutex );
    whitepoint = whitepointIn;
    param.whitepoint = whitepointIn;
    abort = true;
    emit whitepointChanged();
}

//===After whiteblack, during colorcurve===

//===After colorcurve, during filmlikecurve===
//Y value of shadow control curve point
void FilmImageProvider::setShadowsY(float shadowsYIn)
{
    QMutexLocker locker( &mutex );
    shadowsY = shadowsYIn;
    param.shadowsY = shadowsYIn;
    abort = true;
    emit shadowsYChanged();
}

//Y value of highlight control curve point
void FilmImageProvider::setHighlightsY(float highlightsYIn)
{
    QMutexLocker locker( &mutex );
    highlightsY = highlightsYIn;
    param.highlightsY = highlightsYIn;
    abort = true;
    emit highlightsYChanged();
}

void FilmImageProvider::setVibrance(float vibranceIn)
{
    QMutexLocker locker( &mutex );
    vibrance = vibranceIn;
    param.vibrance = vibranceIn;
    abort = true;
    emit vibranceChanged();
}

void FilmImageProvider::setSaturation(float saturationIn)
{
    QMutexLocker locker( &mutex );
    saturation = saturationIn;
    param.saturation = saturationIn;
    abort = true;
    emit saturationChanged();
}

void FilmImageProvider::setProgress(float percentDone_in)
{
    progress = percentDone_in;
    emit progressChanged();
}

void FilmImageProvider::setSaveTiff(bool saveTiffIn)
{
    QMutexLocker locker( &mutex );
    saveTiff = saveTiffIn;
    emit saveTiffChanged();
}

void FilmImageProvider::setSaveJpeg(bool saveJpegIn)
{
    QMutexLocker locker( &mutex );
    saveJpeg = saveJpegIn;
    emit saveJpegChanged();
}

void FilmImageProvider::updateFilmProgress(float percentDone_in)//Percent filmulation
{
    progress = 0.2 + percentDone_in*0.6;
    emit progressChanged();
}

void FilmImageProvider::invalidateImage()
{
    cout << "filmImageProvider::invalidateImage(). We probably shouldn't be using this." << endl;
    QMutexLocker locker( &mutex );
}

float FilmImageProvider::getHistogramPoint(Histogram &hist, int index, int i, LogY isLog)
{
    //index is 0 for L, 1 for R, 2 for G, and 3 for B.
    assert((index < 4) && (index >= 0));
    switch (index)
    {
    case 0: //luminance
        if (!isLog)
            return float(min(hist.lHist[i],hist.lHistMax))/float(hist.lHistMax);
        else
            return log(hist.lHist[i]+1)/log(hist.lHistMax+1);
    case 1: //red
        if (!isLog)
            return float(min(hist.rHist[i],hist.rHistMax))/float(hist.rHistMax);
        else
            return log(hist.rHist[i]+1)/log(hist.rHistMax+1);
    case 2: //green
        if (!isLog)
            return float(min(hist.gHist[i],hist.gHistMax))/float(hist.gHistMax);
        else
            return log(hist.gHist[i]+1)/log(hist.gHistMax+1);
    default://case 3: //blue
        if (!isLog)
            return float(min(hist.bHist[i],hist.bHistMax))/float(hist.bHistMax);
        else
            return log(hist.bHist[i]+1)/log(hist.bHistMax+1);
    }
    //xHistMax is the maximum height of any bin except the extremes.

    //return float(min(hist.allHist[i*4+index],hist.histMax[index]))/float(hist.histMax[index]); //maximum is the max of all elements except 0 and 127
}

QImage FilmImageProvider::emptyImage()
{
    return QImage(0,0,QImage::Format_ARGB32);
}

void FilmImageProvider::rotateLeft()
{
    QMutexLocker locker( &mutex );
    rotation++;
    if (rotation > 3)
    {
        rotation -= 4;
    }
    param.rotation = rotation;
    abort = true;
}

void FilmImageProvider::rotateRight()
{
    QMutexLocker locker( &mutex );
    rotation--;
    if (rotation < 0)
    {
        rotation += 4;
    }
    param.rotation = rotation;
    abort = true;
}
