#include "filmimageprovider.h"
#include <pwd.h>
#include <unistd.h>
#include <QTimer>
#include <cmath>
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
    tempID.remove(0,7);

    switch (valid)
    {
    case none://Do demosiac
    {
        //Mark that demosaicing has started.
        mutex.lock();
        valid = demosaic;
        mutex.unlock();

        //Here we trim the 'file://' from the url
        std::vector<std::string> input_filename_list;
        input_filename_list.push_back(tempID.toStdString());
        cout << "Opening " << input_filename_list[0] << endl;

        //We set the exposure comp from the Q_PROPERTY here.
        std::vector<float> input_exposure_compensation;
        input_exposure_compensation.push_back(exposureComp);

        //Only raws.
        bool tiff_in = false;
        bool jpeg_in = false;

        //No highlight recovery.
        int highlights = 0;

        //Reads in the photo.
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
        //If we're about to do work on invalidated demosaicing, we give up.
        //It'll restart automatically.
        if(checkAbort(demosaic))
            return emptyImage();

        //Mark that we've started to filmulate.
        mutex.lock();
        valid = filmulation;
        mutex.unlock();

        //Here we actually apply the exposure compensation.
        matrix<float> compImage = input_image * pow(2, exposureComp);

        //Read in from the configuration file
        //Get home directory
        int myuid = getuid();
        passwd *mypasswd = getpwuid(myuid);
        std::string input_configuration = std::string(mypasswd->pw_dir) +
                "/.filmulator/configuration.txt";
        initialize(input_configuration, filmParams);
        filmParams.film_area = filmArea;

        //Here we do the film simulation on the image...
        if(filmulate(compImage, filmulated_image, filmParams, this))
            return emptyImage();//filmulate returns 1 if it detected an abort

        //Histogram work
        updateFloatHistogram(postFilmHist, filmulated_image, .0025, histPostFilm);
        //cout << mean(filmulated_image) << endl;
        emit histPostFilmChanged();//must be run to notify QML
    }
    case filmulation://Do whitepoint_blackpoint
    {
        setProgress(0.8);

        //See if the filmulation has been invalidated.
        if(checkAbort(filmulation))
            return emptyImage();

        //Mark that we've begun clipping the image and converting to unsigned short.
        mutex.lock();
        valid = whiteblack;
        mutex.unlock();
        whitepoint_blackpoint(filmulated_image, contrast_image, whitepoint,
                              blackpoint);
    }
    case whiteblack: // Do color_curve
    {
        setProgress(0.85);

        //See if the clipping has been invalidated.
        if(checkAbort(whiteblack))
            return emptyImage();

        //Mark that we've begun running the individual color curves.
        mutex.lock();
        valid = colorcurve;
        mutex.unlock();
        color_curves(contrast_image, color_curve_image, lutR, lutG, lutB);
    }
    case colorcurve://Do flim-like curve
    {
        setProgress(0.9);

        //See if the color curves applied are now invalid.
        if(checkAbort(colorcurve))
            return emptyImage();

        //Mark that we've begun applying the all-color tone curve.
        mutex.lock();
        valid = filmlikecurve;
        mutex.unlock();
        filmLikeLUT.fill(this);
        film_like_curve(color_curve_image,film_curve_image,filmLikeLUT);
    }
    case filmlikecurve: //output
    {
        setProgress(0.95);

        //See if the tonecurve has changed since it was applied.
        if(checkAbort(filmlikecurve))
            return emptyImage();

        //We would mark our progress, but this is the very last step.

        int nrows = film_curve_image.nr();
        int ncols = film_curve_image.nc();
        switch((int) exifData["Exif.Image.Orientation"].value().toLong())
        {
        case 3://upside-down
        {
            //Yes, some cameras do in fact implement this direction.
            output = QImage(ncols/3,nrows,QImage::Format_ARGB32);
            for(int i = 0; i < nrows; i++)
            {
                // out   in
                /* 12345 00000
                 * 00000 00000
                 * 00000 54321
                 */
                //Reversing the row index
                int r = nrows-1-i;
                QRgb *line = (QRgb *)output.scanLine(i);
                for(int j = 0; j < ncols; j = j + 3)
                {
                    //Reversing the column index
                    int c = ncols - 3 - j;
                    *line = QColor(film_curve_image(r,c)/256,
                                   film_curve_image(r,c+1)/256,
                                   film_curve_image(r,c+2)/256).rgb();
                    line++;
                }
            }
            break;
        }
        case 6://right side of camera down
        {
            output = QImage(nrows,ncols/3,QImage::Format_ARGB32);
            for(int j = 0; j < ncols/3; j++)
            {
                //index of an output row as a column on the input matrix
                // out   in
                /* 123   30000
                 * 000   20000
                 * 000   10000
                 * 000
                 * 000
                 */
                //Remember that the columns are interlaced.
                int c = j*3;
                QRgb *line = (QRgb *)output.scanLine(j);
                for(int i = 0; i < nrows; i++)
                {
                    //Also, in this case, the order is reversed for the rows of the input.
                    int r = nrows-1-i;
                    *line = QColor(film_curve_image(r,c)/256,
                                   film_curve_image(r,c+1)/256,
                                   film_curve_image(r,c+2)/256).rgb();
                    line++;
                }
            }
            break;
        }
        case 8://right side of camera up
        {
            output = QImage(nrows,ncols/3,QImage::Format_ARGB32);
            for(int j = 0; j < ncols/3; j++)
            {
                //index of an output row as a column on the input matrix
                // out   in
                /* 123   00001
                 * 000   00002
                 * 000   00003
                 * 000
                 * 000
                 */
                //Remember that the columns are interlaced, and scanned in reverse.
                int c = ncols - 3 - j*3;
                QRgb *line = (QRgb *)output.scanLine(j);
                for(int i = 0; i < nrows; i++)
                {
                    //Here the order is not reversed for the rows of the input.
                    int r = i;
                    *line = QColor(film_curve_image(r,c)/256,
                                   film_curve_image(r,c+1)/256,
                                   film_curve_image(r,c+2)/256).rgb();
                    line++;
                }
            }
            break;
        }
        default://standard orientation
        {
            output = QImage(ncols/3,nrows,QImage::Format_ARGB32);
            for(int i = 0; i < nrows; i++)
            {
                QRgb *line = (QRgb *)output.scanLine(i);
                for(int j = 0; j < ncols; j = j + 3)
                {
                    *line = QColor(film_curve_image(i,j)/256,
                                   film_curve_image(i,j+1)/256,
                                   film_curve_image(i,j+2)/256).rgb();
                    line++;

                }
            }
        }
        }//End orientation switch
    }
    }//End task switch

    emit histPostFilmChanged();
    updateShortHistogram(finalHist, film_curve_image, histFinal);
    emit histFinalChanged();//This must be run immediately after updateHistFinal in order to notify QML.

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

void FilmImageProvider::setFilmArea(float filmAreaIn)
{
    QMutexLocker locker (&mutex);
    filmArea = filmAreaIn;
    if (valid > demosaic)
        valid = demosaic;
    emit filmAreaChanged();
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

float FilmImageProvider::getHistogramPoint(histogram &hist, int index, int i, LogY isLog)
{
    //index is 0 for L, 1 for R, 2 for G, and 3 for B.
    assert((index < 4) && (index >= 0));
    switch (index)
    {
    case 0: //luminance
        if (!isLog)
            return float(min(hist.lHist[i],hist.lHistMax))/float(hist.lHistMax);
        else
            return max(0.0,min(log(hist.lHist[i]),log(hist.lHistMax))/log(hist.lHistMax));
    case 1: //red
        if (!isLog)
            return float(min(hist.rHist[i],hist.rHistMax))/float(hist.rHistMax);
        else
            return max(0.0,min(log(hist.rHist[i]),log(hist.rHistMax))/log(hist.rHistMax));
    case 2: //green
        if (!isLog)
            return float(min(hist.gHist[i],hist.gHistMax))/float(hist.gHistMax);
        else
            return max(0.0,min(log(hist.gHist[i]),log(hist.gHistMax))/log(hist.gHistMax));
    case 3: //blue
        if (!isLog)
            return float(min(hist.bHist[i],hist.bHistMax))/float(hist.bHistMax);
        else
            return max(0.0,min(log(hist.bHist[i]),log(hist.bHistMax))/log(hist.bHistMax));
    }
    //xHistMax is the maximum height of any bin except the extremes.

    //return float(min(hist.allHist[i*4+index],hist.histMax[index]))/float(hist.histMax[index]); //maximum is the max of all elements except 0 and 127
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
