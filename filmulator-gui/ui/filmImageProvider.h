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
#include <QThread>
#include "thumbWriteWorker.h"
#include "../ui/settings.h"

class FilmImageProvider : public QObject, public QQuickImageProvider, public Interface
{
    Q_OBJECT

    Q_PROPERTY(float progress READ getProgress WRITE setProgress NOTIFY progressChanged)

public:
    FilmImageProvider(ParameterManager*);
    ~FilmImageProvider();
    QImage requestImage(const QString& /*id*/, QSize* size, const QSize& /*requestedSize*/);


    //Setter methods
    void setProgress(float progressIn);
    //Getter methods
    float getProgress(){return progress;}

    void updateFilmProgress(float);

    Q_INVOKABLE float getHistFinalPoint(int index, int i){return getHistogramPoint(finalHist,index,i,LogY::no);}
    Q_INVOKABLE float getHistRawPoint(int index, int i){return getHistogramPoint(rawHist,index,i,LogY::yes);}
    Q_INVOKABLE float getHistPostFilmPoint(int index, int i){return getHistogramPoint(postFilmHist,index,i,LogY::yes);}
    Q_INVOKABLE float getHistPreFilmPoint(int index, int i){return getHistogramPoint(preFilmHist,index,i,LogY::yes);}

    void updateHistRaw(const matrix<float>& image, float maximum, unsigned cfa[2][2], unsigned xtrans[6][6], int maxXtrans, bool isRGB, bool isMonochrome);
    void updateHistPreFilm(const matrix<float>& image, float maximum);
    void updateHistPostFilm(const matrix<float>& image, float maximum);
    void updateHistFinal(const matrix<unsigned short>& image);

    Q_INVOKABLE void writeTiff();
    Q_INVOKABLE void writeJpeg();
    Q_INVOKABLE void writeThumbnail(QString searchID);
    Q_INVOKABLE void disableThumbnailWrite() {thumbnailWriteEnabled = false;}
    Q_INVOKABLE void enableThumbnailWrite() {thumbnailWriteEnabled = true;}

    //clean up threads before exiting
    Q_INVOKABLE void exitWorker()
    {
        if (workerThread.isRunning())
        {
            workerThread.exit();
        }
    }

protected:
    ImagePipeline pipeline;
    ImagePipeline quickPipe;
    //We also want to make available image pipelines for preloading the next and previous images.
    //Upon changing images, we'll want to copy all the pipeline stages into the current quickPipe
    //Validity too... that goes with the ParamManagers.
    ImagePipeline nextQuickPipe;
    ImagePipeline prevQuickPipe;

    ThumbWriteWorker *worker = new ThumbWriteWorker;
    QThread workerThread;
    bool thumbnailWriteEnabled = true;
    bool writeThisThumbnail = true;

    bool useQuickPipe;

    ParameterManager * paramManager;
    ParameterManager * cloneParam;
    //We also want to make available image pipelines for preloading the next and previous images.
    //Upon changing images, we'll want to copy all the pipeline stages into the current quickPipe
    //When we do the shuffling of the data, once done we'll have to call
    // paramManager.cloneParams(*cloneParam) so that it knows.
    ParameterManager * nextParam;
    ParameterManager * prevParam;

    QMutex processMutex;//Ensures that output files are only of the currently selected image.
    QMutex writeDataMutex;//binds together the update of outputFilename and the outputImage.
    float progress;

    struct timeval request_start_time;

    Exiv2::ExifData exifData;
    Exiv2::ExifData basicExifData;
    std::string outputFilename;
    matrix<unsigned short> last_image;

    Histogram finalHist;
    Histogram rawHist;
    Histogram postFilmHist;
    Histogram preFilmHist;

    float getHistogramPoint(Histogram &hist, int index, int i, LogY isLog);
    QImage emptyImage();

    void updateShortHistogram(Histogram &hist, const matrix<unsigned short>& image );
    void updateFloatHistogram(Histogram &hist, const matrix<float>& image, float maximum );
    int histIndex(float value, float max);
    void zeroHistogram(Histogram &hist);

signals:
    void progressChanged();

    //Notifications for the histograms
    void histFinalChanged();
    void histRawChanged();
    void histPostFilmChanged();
    void histPreFilmChanged();

    void requestThumbnail(QString outputFilename);
    void thumbnailDone();

public slots:
    void thumbDoneWriting();

};

#endif // FILMIMAGEPROVIDER_H

