#include "filmimageprovider.h"
#include <pwd.h>
#include <unistd.h>
#include <QTimer>
#define TIMEOUT 0.1

FilmImageProvider::FilmImageProvider(QQuickImageProvider::ImageType type) :
    QObject(0),
    QQuickImageProvider(type, QQuickImageProvider::ForceAsynchronousImageLoading)
{
    valid = none;
    blackpoint = 0;
    lutR.setUnity();
    lutG.setUnity();
    lutB.setUnity();
    //filmLikeLUT.setUnity();
    defaultToneCurveEnabled = true;
    shadowsX = 0.25;
    shadowsY = 0.9;
    highlightsX = 0.75;
    highlightsY = 0.1;
    filmLikeLUT.fill(this);
}

FilmImageProvider::FilmImageProvider() :
    QObject(0),
    QQuickImageProvider(QQuickImageProvider::Image,
                        QQuickImageProvider::ForceAsynchronousImageLoading)
{
    valid = none;
    blackpoint = 0;
    lutR.setUnity();
    lutG.setUnity();
    lutB.setUnity();
    defaultToneCurveEnabled = true;
    shadowsX = 0.25;
    shadowsY = 0.25;
    highlightsX = 0.75;
    highlightsY = 0.75;
    filmLikeLUT.fill(this);
    //filmLikeLUT.setUnity();
}

FilmImageProvider::~FilmImageProvider()
{
}

QImage FilmImageProvider::requestImage(const QString &id,
                                       QSize *size, const QSize &requestedSize)
{
    gettimeofday(&request_start_time,NULL);
    QImage output = emptyImage();
    QString tempID = id;
    tempID.remove(tempID.length()-1,1);

    switch (valid)
    {
        case none://Do demosiac
        {
            mutex.lock();
                valid = demosaic;
            mutex.unlock();
            std::vector<std::string> input_filename_list;
            input_filename_list.push_back(tempID.toStdString());
            cout << "Opening " << input_filename_list[0] << endl;
            std::vector<float> input_exposure_compensation;
            input_exposure_compensation.push_back(exposureComp);

            bool tiff_in = false;
            bool jpeg_in = false;
            int highlights = 0;
            if(imload(input_filename_list, input_exposure_compensation,
                      input_image, tiff_in, jpeg_in, exifData, highlights))
            {
                qDebug("Error loading images");
                mutex.lock();
                        valid = none;
                mutex.unlock();
                return emptyImage();
            }
        }
       case demosaic://Do filmulation
       {
            if(checkAbort(demosaic))
                return emptyImage();
            mutex.lock();
                    valid = filmulation;
            mutex.unlock();
            matrix<float> compImage = input_image * pow(2, exposureComp);
            //Read in from the configuration file
            //Get home directory
            int myuid = getuid();
            passwd *mypasswd = getpwuid(myuid);
            std::string input_configuration = std::string(mypasswd->pw_dir) +
                    "/.filmulator/configuration.txt";
            initialize(input_configuration, filmParams);
            filmParams.film_area = filmSize;

            //Here we do the film simulation on the image...
            if(filmulate(compImage, filmulated_image, filmParams, this))
                return emptyImage();//filmulate returns 1 if it detected an abort
        }
        case filmulation://Do whitepoint_blackpoint
        {
            setProgress(0.8);
            if(checkAbort(filmulation))
                return emptyImage();
            mutex.lock();
                valid = whiteblack;
            mutex.unlock();
            whitepoint_blackpoint(filmulated_image, contrast_image, whitepoint,
                                  blackpoint);
        }
        case whiteblack: // Do color_curve
        {
            setProgress(0.85);
            if(checkAbort(whiteblack))
                return emptyImage();
            mutex.lock();
                valid = colorcurve;
            mutex.unlock();
            color_curves(contrast_image, color_curve_image, lutR, lutG, lutB);
        }
        case colorcurve://Do flim-like curve
        {
            setProgress(0.9);
            if(checkAbort(colorcurve))
                return emptyImage();
            mutex.lock();
                valid = filmlikecurve;
            mutex.unlock();
            filmLikeLUT.fill(this);
            film_like_curve(color_curve_image,film_curve_image,filmLikeLUT);
        }
        case filmlikecurve: //output
        {
            setProgress(0.95);
            int nrows = film_curve_image.nr();
            int ncols = film_curve_image.nc();
            if(checkAbort(filmlikecurve))
                return emptyImage();
            //Normally, here we'd output the file. Instead, we write it to the QImage.
            output = QImage(ncols/3,nrows,QImage::Format_ARGB32);
            for(int i = 0; i < nrows; i++)
            {
                QRgb *line = (QRgb *)output.scanLine(i);
                for(int j = 0; j < ncols; j = j + 3)
                {
                    *line = QColor(film_curve_image(i,j)/256,film_curve_image(i,j+1)/256,film_curve_image(i,j+2)/256).rgb();
                    line++;
                }
            }
        }
    }//End switch
    updateHistograms();
    tout << "Request time: " << time_diff(request_start_time) << " seconds" << endl;
    setProgress(1);
    *size = output.size();
    return output;
}

void FilmImageProvider::setExposureComp(float exposure)
{
    QMutexLocker locker(&mutex);
    exposureComp = exposure;
    if (valid > demosaic)
        valid = demosaic;
    emit exposureCompChanged();
}

void FilmImageProvider::setFilmSize(float filmSizeIn)
{
    QMutexLocker locker (&mutex);
    filmSize = filmSizeIn;
    if (valid > demosaic)
        valid = demosaic;
    emit filmSizeChanged();
}

void FilmImageProvider::setWhitepoint(float whitepointIn)
{
    QMutexLocker locker (&mutex);
    whitepoint = whitepointIn;
    if (valid > filmulation)
        valid = filmulation;
    emit whitepointChanged();
}

void FilmImageProvider::setBlackpoint(float blackpointIn)
{
    QMutexLocker locker (&mutex);
    blackpoint = blackpointIn;
    if (valid > filmulation)
        valid = filmulation;
    emit blackpointChanged();
}

void FilmImageProvider::setShadowsY(float shadowsYIn)
{
    QMutexLocker locker (&mutex);
    shadowsY = shadowsYIn;
    if (valid > filmulation)
        valid = filmulation;
    emit shadowsYChanged();
}

void FilmImageProvider::setHighlightsY(float highlightsYIn)
{
    QMutexLocker locker (&mutex);
    highlightsY = highlightsYIn;
    if (valid > filmulation)
        valid = filmulation;
    emit highlightsYChanged();
}

void FilmImageProvider::setDefaultToneCurveEnabled(bool enabledIn)
{
    QMutexLocker locker (&mutex);
    defaultToneCurveEnabled = enabledIn;
    if (valid > filmulation)
        valid = filmulation;
    emit defaultToneCurveEnabledChanged();
}

void FilmImageProvider::setProgress(float percentDone_in)
{
    progress = percentDone_in;
    emit progressChanged();
}

void FilmImageProvider::updateProgress(float percentDone_in)//Percent filmulation
{
    progress = 0.2 + percentDone_in*0.6;
    emit progressChanged();
}

void FilmImageProvider::invalidateImage()
{
    QMutexLocker locker(&mutex);
    valid = none;
}

float FilmImageProvider::getHistogramPoint(long long * hist, long long maximum, int i)
{
    return float(min(hist[i],maximum))/float(maximum); //maximum is the max of all elements except 0 and 127
}

QImage FilmImageProvider::emptyImage()
{
    return QImage(0,0,QImage::Format_ARGB32);
}

bool FilmImageProvider::checkAbort(Valid currStep)
{
    if (valid < currStep && time_diff(request_start_time) > 0.1)
        return true;
    else
        return false;
}

unsigned short FilmImageProvider::lookup(unsigned short in)
{
    return 65535*default_tonecurve(
                 shadows_highlights(float(in)/65535.0,shadowsX,shadowsY,
                                   highlightsX,highlightsY)
                ,defaultToneCurveEnabled);
}
