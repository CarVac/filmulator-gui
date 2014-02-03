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

enum Valid {none, demosaic, filmulation, whiteblack, colorcurve, filmlikecurve};

class FilmImageProvider : public QObject, public QQuickImageProvider, public Interface
{
    Q_OBJECT
    Q_PROPERTY(float exposureComp READ getExposureComp WRITE setExposureComp NOTIFY exposureCompChanged)
    Q_PROPERTY(float filmSize READ getFilmSize WRITE setFilmSize NOTIFY filmSizeChanged)
    Q_PROPERTY(float whitepoint READ getWhitepoint WRITE setWhitepoint NOTIFY whitepointChanged)
    Q_PROPERTY(float blackpoint READ getBlackpoint WRITE setBlackpoint NOTIFY blackpointChanged)
    Q_PROPERTY(float shadowsY READ getShadowsY WRITE setShadowsY NOTIFY shadowsYChanged)
    Q_PROPERTY(float highlightsY READ getHighlightsY WRITE setHighlightsY NOTIFY highlightsYChanged)
    Q_PROPERTY(bool defaultToneCurveEnabled READ getDefaultToneCurveEnabled WRITE setDefaultToneCurveEnabled NOTIFY defaultToneCurveEnabledChanged)
    Q_PROPERTY(float progress READ getProgress WRITE setProgress NOTIFY progressChanged)
    Q_PROPERTY(int hist READ getHist NOTIFY histogramsUpdated)//Dummy variable to cause histogram updates
public:
    FilmImageProvider(QQuickImageProvider::ImageType type);
    FilmImageProvider();
    ~FilmImageProvider();
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize);

    void setExposureComp(float exposureIn);
    void setFilmSize(float filmSizeIn);
    void setWhitepoint(float whitepointIn);
    void setBlackpoint(float blackpointIn);
    void setShadowsY(float shadowsYIn);
    void setHighlightsY(float highlightsYIn);
    void setDefaultToneCurveEnabled(bool enabledIn);
    void setProgress(float progressIn);

    float getExposureComp(){return exposureComp;}
    float getFilmSize(){return filmSize;}
    float getWhitepoint(){return whitepoint;}
    float getBlackpoint(){return blackpoint;}
    float getShadowsY(){return shadowsY;}
    float getHighlightsY(){return highlightsY;}
    bool getDefaultToneCurveEnabled(){return defaultToneCurveEnabled;}
    float getProgress(){return progress;}
    int getHist(){return hist;}

    bool checkAbort(Valid currStep);
    bool checkAbort(){return checkAbort(filmulation);}
    unsigned short lookup(unsigned short in);

    void updateProgress(float);
    Q_INVOKABLE void invalidateImage();
    Q_INVOKABLE float getLumaHistogramPoint(int i){return getHistogramPoint(lumaHistogram,maxBinLuma,i);}
    Q_INVOKABLE float getRHistogramPoint(int i){return getHistogramPoint(rHistogram,maxBinR,i);}
    Q_INVOKABLE float getGHistogramPoint(int i){return getHistogramPoint(gHistogram,maxBinG,i);}
    Q_INVOKABLE float getBHistogramPoint(int i){return getHistogramPoint(bHistogram,maxBinB,i);}


protected:
    QMutex mutex;

    float exposureComp;
    float filmSize;
    float progress;
    float whitepoint;
    float blackpoint;
    int hist;
    bool defaultToneCurveEnabled;
    float shadowsX, shadowsY, highlightsX, highlightsY;

    LUT lutR, lutG, lutB;
    LUT filmLikeLUT;

    filmulateParams filmParams;

    struct timeval request_start_time;

    Valid valid;

    matrix<float> input_image;
    Exiv2::ExifData exifData;
    matrix<float> filmulated_image;
    matrix<unsigned short> contrast_image;
    matrix<unsigned short> color_curve_image;
    matrix<unsigned short> film_curve_image;

    long long lumaHistogram[128];
    long long rHistogram[128];
    long long gHistogram[128];
    long long bHistogram[128];

    long long maxBinLuma;
    long long maxBinR;
    long long maxBinG;
    long long maxBinB;

    float getHistogramPoint(long long * hist, long long maximum, int i);
    QImage emptyImage();
    void updateHistograms();

signals:
    void exposureCompChanged();
    void filmSizeChanged();
    void whitepointChanged();
    void blackpointChanged();
    void shadowsYChanged();
    void highlightsYChanged();
    void defaultToneCurveEnabledChanged();
    void progressChanged();
    void histogramsUpdated();

public slots:

};

#endif // FILMIMAGEPROVIDER_H
