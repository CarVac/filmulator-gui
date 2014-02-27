#ifndef FILMIMAGEPROVIDER_H
#define FILMIMAGEPROVIDER_H

#include <QObject>
#include <QColor>
#include <QImage>
#include <QQuickImageProvider>
#include <QMutex>
#include <QMutexLocker>
#include <QList>
#include "../core/filmsim.hpp"
#include <assert.h>

enum Valid {none, load, demosaic, prefilmulation, filmulation, whiteblack, colorcurve, filmlikecurve};

enum LogY {no, yes};

struct histogram {
    long long lHist[128];
    long long rHist[128];
    long long gHist[128];
    long long bHist[128];

    long long lHistMax;
    long long rHistMax;
    long long gHistMax;
    long long bHistMax;
};

class FilmImageProvider : public QObject, public QQuickImageProvider, public Interface
{
    Q_OBJECT
    Q_PROPERTY(int highlights READ getHighlights WRITE setHighlights NOTIFY highlightsChanged)

    Q_PROPERTY(float exposureComp READ getExposureComp WRITE setExposureComp NOTIFY exposureCompChanged)

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

    Q_PROPERTY(float progress READ getProgress WRITE setProgress NOTIFY progressChanged)

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
    void setHighlights(int highlightsIn);

    //After demosaic, during prefilmulation
    void setExposureComp(float exposureIn);

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

    void setProgress(float progressIn);

    //Getter methods
    int getHighlights(){return highlights;}

    float getExposureComp(){return exposureComp;}

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

    float getProgress(){return progress;}
/*    int getHistFinal(){return histFinal;}
    int getHistPostFilm(){return histPostFilm;}
    int getHistPreFilm(){return histPreFilm;}*/

    bool checkAbort(Valid currStep);
    bool checkAbort(){return checkAbort(filmulation);}
    unsigned short lookup(unsigned short in);

    void updateProgress(float);
    Q_INVOKABLE void invalidateImage();
    Q_INVOKABLE float getHistFinalPoint(int index, int i){return getHistogramPoint(finalHist,index,i,LogY::no);}
    Q_INVOKABLE float getHistPostFilmPoint(int index, int i){return getHistogramPoint(postFilmHist,index,i,LogY::yes);}
    Q_INVOKABLE float getHistPreFilmPoint(int index, int i){return getHistogramPoint(preFilmHist,index,i,LogY::yes);}

    bool isGUI(){return true;}

protected:
    QMutex mutex;

    int highlights;

    float exposureComp;

    float develTime;
    int agitateCount;
    int develSteps;
    float filmArea;
    float layerMixConst;

    float blackpoint;
    float whitepoint;
    bool defaultToneCurveEnabled;
    float shadowsX, shadowsY, highlightsX, highlightsY;

    float wbRMultiplier, wbGMultiplier, wbBMultiplier;

    float progress;

    LUT lutR, lutG, lutB;
    LUT filmLikeLUT;

    filmulateParams filmParams;

    struct timeval request_start_time;

    Valid valid;

    matrix<float> input_image;
    matrix<float> pre_film_image;
    Exiv2::ExifData exifData;
    matrix<float> filmulated_image;
    matrix<unsigned short> contrast_image;
    matrix<unsigned short> color_curve_image;
    matrix<unsigned short> film_curve_image;

    histogram finalHist;
    int histFinal;//dummy to signal histogram updates

    histogram postFilmHist;
    int histPostFilm;//dummy to signal histogram updates

    histogram preFilmHist;
    int histPreFilm;//dummy to signal histogram updates

    float getHistogramPoint(histogram &hist, int index, int i, LogY isLog);
    QImage emptyImage();

    void updateShortHistogram(histogram &hist, const matrix<unsigned short> image, int &roll);
    void updateFloatHistogram(histogram &hist, const matrix<float> image, float maximum, int &roll);
    int histIndex(float value, float max);

signals:
    void highlightsChanged();

    void exposureCompChanged();

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

    void progressChanged();

    //Notifications for the histograms
    void histFinalChanged();
    void histPostFilmChanged();
    void histPreFilmChanged();

public slots:

};

#endif // FILMIMAGEPROVIDER_H

