#ifndef FILMIMAGEPROVIDER_H
#define FILMIMAGEPROVIDER_H

#include <QObject>
#include <QColor>
#include <QImage>
#include <QQuickImageProvider>
#include <QMutex>
#include <QMutexLocker>
#include <QList>
#include "../core/imagePipeline.h"
#include <assert.h>


class FilmImageProvider : public QObject, public QQuickImageProvider, public Interface
{
    Q_OBJECT
    Q_PROPERTY(bool caEnabled READ getCaEnabled WRITE setCaEnabled NOTIFY caEnabledChanged)
    Q_PROPERTY(int highlights READ getHighlights WRITE setHighlights NOTIFY highlightsChanged)

    Q_PROPERTY(float exposureComp READ getExposureComp WRITE setExposureComp NOTIFY exposureCompChanged)
    Q_PROPERTY(double temperature READ getTemperature WRITE setTemperature NOTIFY temperatureChanged)
    Q_PROPERTY(double tint READ getTint WRITE setTint NOTIFY tintChanged)

    Q_PROPERTY(float develTime READ getDevelTime WRITE setDevelTime NOTIFY develTimeChanged)
    Q_PROPERTY(int agitateCount READ getAgitateCount WRITE setAgitateCount NOTIFY agitateCountChanged)
    Q_PROPERTY(int develSteps READ getDevelSteps WRITE setDevelSteps NOTIFY develStepsChanged)
    Q_PROPERTY(float filmArea READ getFilmArea WRITE setFilmArea NOTIFY filmAreaChanged)
    Q_PROPERTY(float layerMixConst READ getLayerMixConst WRITE setLayerMixConst NOTIFY layerMixConstChanged)

    Q_PROPERTY(float blackpoint READ getBlackpoint WRITE setBlackpoint NOTIFY blackpointChanged)
    Q_PROPERTY(float whitepoint READ getWhitepoint WRITE setWhitepoint NOTIFY whitepointChanged)
    Q_PROPERTY(bool defaultToneCurveEnabled READ getDefaultToneCurveEnabled WRITE setDefaultToneCurveEnabled NOTIFY defaultToneCurveEnabledChanged)
    Q_PROPERTY(float shadowsY READ getShadowsY WRITE setShadowsY NOTIFY shadowsYChanged)
    Q_PROPERTY(float highlightsY READ getHighlightsY WRITE setHighlightsY NOTIFY highlightsYChanged)

    Q_PROPERTY(float vibrance READ getVibrance WRITE setVibrance NOTIFY vibranceChanged)
    Q_PROPERTY(float saturation READ getSaturation WRITE setSaturation NOTIFY saturationChanged)

    Q_PROPERTY(float progress READ getProgress WRITE setProgress NOTIFY progressChanged)

    Q_PROPERTY(bool saveTiff READ getSaveTiff WRITE setSaveTiff NOTIFY saveTiffChanged)
    Q_PROPERTY(bool saveJpeg READ getSaveJpeg WRITE setSaveJpeg NOTIFY saveJpegChanged)

/*    //Dummy properties to signal histogram updates
//    Q_PROPERTY(int histFinal READ getHistFinal NOTIFY histFinalChanged)
//    Q_PROPERTY(int histPostFilm READ getHistPostFilm NOTIFY histPostFilmChanged)
//    Q_PROPERTY(int histPreFilm READ getHistPreFilm NOTIFY histPreFilmChanged)*/
public:
    FilmImageProvider(QQuickImageProvider::ImageType type);
    FilmImageProvider();
    ~FilmImageProvider();
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize);


    //Setter methods
    //After load, during demosaic
    void setCaEnabled(bool caEnabledIn);
    void setHighlights(int highlightsIn);

    //After demosaic, during prefilmulation
    void setExposureComp(float exposureIn);
    void setTemperature(double temperatureIn);
    void setTint(double tintIn);

    //After prefilmulation, during filmulation
    void setDevelTime(float develTimeIn);
    void setAgitateCount(int agitateCountIn);
    void setDevelSteps(int develStepsIn);
    void setFilmArea(float filmAreaIn);
    void setLayerMixConst(float layerMixConstIn);

    //After filmulation, during whiteblack
    void setBlackpoint(float blackpointIn);
    void setWhitepoint(float whitepointIn);
    void setDefaultToneCurveEnabled(bool enabledIn);

    //After whiteblack, during colorcurve
    //nothing yet

    //After colorcurve, during filmlikecurve
    void setShadowsY(float shadowsYIn);
    void setHighlightsY(float highlightsYIn);

    void setVibrance(float vibranceIn);
    void setSaturation(float saturationIn);

    void setProgress(float progressIn);

    void setSaveTiff(bool saveTiffIn);
    void setSaveJpeg(bool saveJpegIn);

    //Getter methods
    bool getCaEnabled(){return caEnabled;}
    int getHighlights(){return highlights;}

    float getExposureComp(){return exposureComp;}
    double getTemperature(){return temperature;}
    double getTint(){return tint;}

    float getDevelTime(){return develTime;}
    int getAgitateCount(){return agitateCount;}
    int getDevelSteps(){return develSteps;}
    float getFilmArea(){return filmArea;}
    float getLayerMixConst(){return layerMixConst;}

    float getBlackpoint(){return blackpoint;}
    float getWhitepoint(){return whitepoint;}
    bool getDefaultToneCurveEnabled(){return defaultToneCurveEnabled;}
    float getShadowsY(){return shadowsY;}
    float getHighlightsY(){return highlightsY;}

    float getVibrance(){return vibrance;}
    float getSaturation(){return saturation;}

    float getProgress(){return progress;}
    float getSaveTiff(){return saveTiff;}
    float getSaveJpeg(){return saveJpeg;}
/*    int getHistFinal(){return histFinal;}
    int getHistPostFilm(){return histPostFilm;}
    int getHistPreFilm(){return histPreFilm;}*/

    void updateFilmProgress(float);
    bool checkAbort(Valid currStep);
    bool checkAbort(){return checkAbort(filmulation);}
    bool isGUI(){return true;}
    void setValid( Valid );

    Q_INVOKABLE void invalidateImage();
    Q_INVOKABLE float getHistFinalPoint(int index, int i){return getHistogramPoint(finalHist,index,i,LogY::no);}
    Q_INVOKABLE float getHistPostFilmPoint(int index, int i){return getHistogramPoint(postFilmHist,index,i,LogY::yes);}
    Q_INVOKABLE float getHistPreFilmPoint(int index, int i){return getHistogramPoint(preFilmHist,index,i,LogY::yes);}
    Q_INVOKABLE void rotateRight();
    Q_INVOKABLE void rotateLeft();

    void updateHistPreFilm( const matrix<float> image, float maximum );
    void updateHistPostFilm( const matrix<float> image, float maximum );
    void updateHistFinal( const matrix<unsigned short> image);

protected:
    QMutex mutex;

    bool caEnabled;
    int highlights;

    float exposureComp;
    double temperature;
    double tint;

    float develTime;
    int agitateCount;
    int develSteps;
    float filmArea;
    float layerMixConst;

    float blackpoint;
    float whitepoint;
    bool defaultToneCurveEnabled;
    float shadowsX, shadowsY, highlightsX, highlightsY;
    float vibrance,saturation;
    int rotation;

    float wbRMultiplier, wbGMultiplier, wbBMultiplier;

    float progress;

    bool saveTiff;
    bool saveJpeg;

    ProcessingParameters param;

    struct timeval request_start_time;

    Valid valid;

    matrix<float> input_image;
    matrix<float> pre_film_image;
    Exiv2::ExifData exifData;
    matrix<float> filmulated_image;
    matrix<unsigned short> contrast_image;
    matrix<unsigned short> color_curve_image;
    matrix<unsigned short> vibrance_saturation_image;

    Histogram finalHist;
    int histFinalRoll;//dummy to signal histogram updates

    Histogram postFilmHist;
    int histPostFilmRoll;//dummy to signal histogram updates

    Histogram preFilmHist;
    int histPreFilmRoll;//dummy to signal histogram updates

    float getHistogramPoint(Histogram &hist, int index, int i, LogY isLog);
    QImage emptyImage();

    void updateShortHistogram(Histogram &hist, const matrix<unsigned short> image, int &roll);
    void updateFloatHistogram(Histogram &hist, const matrix<float> image, float maximum, int &roll);
    int histIndex(float value, float max);
    void zeroHistogram(Histogram &hist);

signals:
    void caEnabledChanged();
    void highlightsChanged();

    void exposureCompChanged();
    void temperatureChanged();
    void tintChanged();

    void develTimeChanged();
    void agitateCountChanged();
    void develStepsChanged();
    void filmAreaChanged();
    void layerMixConstChanged();

    void whitepointChanged();
    void blackpointChanged();
    void defaultToneCurveEnabledChanged();
    void shadowsYChanged();
    void highlightsYChanged();

    void vibranceChanged();
    void saturationChanged();

    void progressChanged();
    void saveTiffChanged();
    void saveJpegChanged();

    //Notifications for the histograms
    void histFinalChanged();
    void histPostFilmChanged();
    void histPreFilmChanged();

public slots:

};

#endif // FILMIMAGEPROVIDER_H

