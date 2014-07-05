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
#include "parameterManager.h"

class FilmImageProvider : public QObject, public QQuickImageProvider, public Interface
{
    Q_OBJECT

    Q_PROPERTY(float progress READ getProgress WRITE setProgress NOTIFY progressChanged)

    Q_PROPERTY(bool saveTiff READ getSaveTiff WRITE setSaveTiff NOTIFY saveTiffChanged)
    Q_PROPERTY(bool saveJpeg READ getSaveJpeg WRITE setSaveJpeg NOTIFY saveJpegChanged)

public:
    FilmImageProvider(ParameterManager*);
    ~FilmImageProvider();
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize);


    //Setter methods
    void setProgress(float progressIn);

    void setSaveTiff(bool saveTiffIn);
    void setSaveJpeg(bool saveJpegIn);
    //Getter methods
    float getProgress(){return progress;}
    float getSaveTiff(){return saveTiff;}
    float getSaveJpeg(){return saveJpeg;}

    void updateFilmProgress(float);
    void setValid( Valid );

    Q_INVOKABLE float getHistFinalPoint(int index, int i){return getHistogramPoint(finalHist,index,i,LogY::no);}
    Q_INVOKABLE float getHistPostFilmPoint(int index, int i){return getHistogramPoint(postFilmHist,index,i,LogY::yes);}
    Q_INVOKABLE float getHistPreFilmPoint(int index, int i){return getHistogramPoint(preFilmHist,index,i,LogY::yes);}

    void updateHistPreFilm( const matrix<float> image, float maximum );
    void updateHistPostFilm( const matrix<float> image, float maximum );
    void updateHistFinal( const matrix<unsigned short> image);

protected:
    //ImagePipeline pipeline = ImagePipeline( BothCacheAndHisto, LowQuality );
    ImagePipeline pipeline = ImagePipeline( BothCacheAndHisto, HighQuality );
    ParameterManager * paramManager;
    bool abort;
    QMutex mutex;
    float progress;

    bool saveTiff;
    bool saveJpeg;

    struct timeval request_start_time;

    matrix<float> input_image;
    matrix<float> pre_film_image;
    Exiv2::ExifData exifData;
    matrix<float> filmulated_image;
    matrix<unsigned short> contrast_image;
    matrix<unsigned short> color_curve_image;
    matrix<unsigned short> vibrance_saturation_image;

    Histogram finalHist;
    Histogram postFilmHist;
    Histogram preFilmHist;

    float getHistogramPoint(Histogram &hist, int index, int i, LogY isLog);
    QImage emptyImage();

    void updateShortHistogram(Histogram &hist, const matrix<unsigned short> image );//, int &roll);
    void updateFloatHistogram(Histogram &hist, const matrix<float> image, float maximum );//, int &roll);
    int histIndex(float value, float max);
    void zeroHistogram(Histogram &hist);

signals:
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

