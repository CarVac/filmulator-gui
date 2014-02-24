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
    Q_PROPERTY(float exposureComp READ getExposureComp WRITE setExposureComp NOTIFY exposureCompChanged)
    Q_PROPERTY(float filmArea READ getFilmArea WRITE setFilmArea NOTIFY filmAreaChanged)
    Q_PROPERTY(float whitepoint READ getWhitepoint WRITE setWhitepoint NOTIFY whitepointChanged)
    Q_PROPERTY(float blackpoint READ getBlackpoint WRITE setBlackpoint NOTIFY blackpointChanged)
    Q_PROPERTY(float shadowsY READ getShadowsY WRITE setShadowsY NOTIFY shadowsYChanged)
    Q_PROPERTY(float highlightsY READ getHighlightsY WRITE setHighlightsY NOTIFY highlightsYChanged)
    Q_PROPERTY(bool defaultToneCurveEnabled READ getDefaultToneCurveEnabled WRITE setDefaultToneCurveEnabled NOTIFY defaultToneCurveEnabledChanged)
    Q_PROPERTY(float progress READ getProgress WRITE setProgress NOTIFY progressChanged)
    Q_PROPERTY(int highlights READ getHighlights WRITE setHighlights NOTIFY highlightsChanged)

    //Dummy properties to signal histogram updates
    Q_PROPERTY(int histFinal READ getHistFinal NOTIFY histFinalChanged)
    Q_PROPERTY(int histPostFilm READ getHistPostFilm NOTIFY histPostFilmChanged)
    Q_PROPERTY(int histPreFilm READ getHistPreFilm NOTIFY histPreFilmChanged)
public:
    FilmImageProvider(QQuickImageProvider::ImageType type);
    FilmImageProvider();
    ~FilmImageProvider();
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize);

    void setExposureComp(float exposureIn);
    void setFilmArea(float filmAreaIn);
    void setWhitepoint(float whitepointIn);
    void setBlackpoint(float blackpointIn);
    void setShadowsY(float shadowsYIn);
    void setHighlightsY(float highlightsYIn);
    void setDefaultToneCurveEnabled(bool enabledIn);
    void setProgress(float progressIn);
    void setHighlights(int highlightsIn);

    float getExposureComp(){return exposureComp;}
    float getFilmArea(){return filmArea;}
    float getWhitepoint(){return whitepoint;}
    float getBlackpoint(){return blackpoint;}
    float getShadowsY(){return shadowsY;}
    float getHighlightsY(){return highlightsY;}
    bool getDefaultToneCurveEnabled(){return defaultToneCurveEnabled;}
    float getProgress(){return progress;}
    int getHighlights(){return highlights;}
    int getHistFinal(){return histFinal;}
    int getHistPostFilm(){return histPostFilm;}
    int getHistPreFilm(){return histPreFilm;}

    bool checkAbort(Valid currStep);
    bool checkAbort(){return checkAbort(filmulation);}
    unsigned short lookup(unsigned short in);

    void updateProgress(float);
    Q_INVOKABLE void invalidateImage();
    Q_INVOKABLE float getHistFinalPoint(int index, int i){return getHistogramPoint(finalHist,index,i,LogY::no);}
    Q_INVOKABLE float getHistPostFilmPoint(int index, int i){return getHistogramPoint(postFilmHist,index,i,LogY::yes);}
    Q_INVOKABLE float getHistPreFilmPoint(int index, int i){return getHistogramPoint(preFilmHist,index,i,LogY::yes);}


protected:
    QMutex mutex;

    float exposureComp;
    float filmArea;
    float progress;
    float whitepoint;
    float blackpoint;
    bool defaultToneCurveEnabled;
    float shadowsX, shadowsY, highlightsX, highlightsY;
    int highlights;
    float wbRMultiplier, wbGMultiplier, wbBMultiplier;

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
    void exposureCompChanged();
    void filmAreaChanged();
    void whitepointChanged();
    void blackpointChanged();
    void shadowsYChanged();
    void highlightsYChanged();
    void defaultToneCurveEnabledChanged();
    void progressChanged();
    void histFinalChanged();
    void histPostFilmChanged();
    void histPreFilmChanged();
    void highlightsChanged();

public slots:

};

#endif // FILMIMAGEPROVIDER_H

