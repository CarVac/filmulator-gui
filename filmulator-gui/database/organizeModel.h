#ifndef ORGANIZEMODEL_H
#define ORGANIZEMODEL_H

#include "sqlModel.h"
#include "dateHistogramModel.h"
#include <QString>
#include <QDateTime>
#include <QTimeZone>
#include <QByteArray>
#include <exiv2/exiv2.hpp>

class OrganizeModel : public SqlModel
{
    Q_OBJECT
    Q_PROPERTY(QDate minCaptureDate   READ getMinCaptureTime   WRITE setMinCaptureTime   NOTIFY minCaptureTimeChanged)
    Q_PROPERTY(QDate maxCaptureDate   READ getMaxCaptureTime   WRITE setMaxCaptureTime   NOTIFY maxCaptureTimeChanged)
    Q_PROPERTY(QDate minImportDate    READ getMinImportTime    WRITE setMinImportTime    NOTIFY minImportTimeChanged)
    Q_PROPERTY(QDate maxImportDate    READ getMaxImportTime    WRITE setMaxImportTime    NOTIFY maxImportTimeChanged)
    Q_PROPERTY(QDate minProcessedDate READ getMinProcessedTime WRITE setMinProcessedTime NOTIFY minProcessedTimeChanged)
    Q_PROPERTY(QDate maxProcessedDate READ getMaxProcessedTime WRITE setMaxProcessedTime NOTIFY maxProcessedTimeChanged)
    Q_PROPERTY(int minRating     READ getMinRating     WRITE setMinRating     NOTIFY minRatingChanged)
    Q_PROPERTY(int maxRating     READ getMaxRating     WRITE setMaxRating     NOTIFY maxRatingChanged)
    Q_PROPERTY(int captureSort   READ getCaptureSort   WRITE setCaptureSort   NOTIFY captureSortChanged)
    Q_PROPERTY(int importSort    READ getImportSort    WRITE setImportSort    NOTIFY importSortChanged)
    Q_PROPERTY(int processedSort READ getProcessedSort WRITE setProcessedSort NOTIFY processedSortChanged)
    Q_PROPERTY(int ratingSort    READ getRatingSort    WRITE setRatingSort    NOTIFY ratingSortChanged)
    Q_PROPERTY(int timeZone MEMBER m_timeZone WRITE setTimeZone NOTIFY timeZoneChanged)

    Q_PROPERTY(int imageCount READ getImageCount NOTIFY imageCountChanged)

public:
    explicit OrganizeModel(QObject *parent = 0);
    Q_INVOKABLE void setOrganizeQuery();
    Q_INVOKABLE void setDateHistoQuery();
    Q_INVOKABLE static QString thumbDir();
    Q_INVOKABLE void setRating(const QString searchID, const int rating);
    Q_INVOKABLE void incrementRating(const QString searchID, const int ratingChange);
    Q_INVOKABLE void markDeletion(QString searchID);
    Q_INVOKABLE QString getDateTimeString(qint64 unixTimeIn);
    Q_INVOKABLE QDate getSelectedDate();
    Q_INVOKABLE QString getSelectedYMDString();

    Q_INVOKABLE QString adaptableModelQuery(const bool searchIDOnly);

    DateHistogramModel *dateHistogram = new DateHistogramModel;

    Q_INVOKABLE void setMinMaxCaptureTime(QDate captureTimeIn);
    Q_INVOKABLE void setMinMaxCaptureTimeString(QString captureTimeIn);
    Q_INVOKABLE void extendMinMaxCaptureTimeString(QString captureTimeIn);
    Q_INVOKABLE void incrementCaptureTime(int days);
    Q_INVOKABLE void extendCaptureTimeRange(int days);
    Q_INVOKABLE bool isDateSelected(QString captureTimeIn);
    void setMinCaptureTime(QDate captureTimeIn);
    void setMaxCaptureTime(QDate captureTimeIn);
    void setMinImportTime(QDate importTimeIn);
    void setMaxImportTime(QDate importTimeIn);
    void setMinProcessedTime(QDate processedTimeIn);
    void setMaxProcessedTime(QDate processedTimeIn);
    void setMinRating(int ratingIn);
    void setMaxRating(int ratingIn);

    void setCaptureSort(int sortMode);
    void setImportSort(int sortMode);
    void setProcessedSort(int sortMode);
    void setRatingSort(int sortMode);
    void setTimeZone(int timeZoneIn);

    QDate getMinCaptureTime() {return minCaptureDate;}
    QDate getMaxCaptureTime() {return maxCaptureDate;}
    QDate getMinImportTime() {return minImportDate;}
    QDate getMaxImportTime() {return maxImportDate;}
    QDate getMinProcessedTime() {return minProcessedDate;}
    QDate getMaxProcessedTime() {return maxProcessedDate;}
    int getMinRating() {return minRating;}
    int getMaxRating() {return maxRating;}

    int getCaptureSort() {return captureSort;}
    int getImportSort() {return importSort;}
    int getProcessedSort() {return processedSort;}
    int getRatingSort() {return ratingSort;}

    int getImageCount(){return m_imageCount;}

signals:
    void minCaptureTimeChanged();
    void maxCaptureTimeChanged();
    void minImportTimeChanged();
    void maxImportTimeChanged();
    void captureDateChanged();
    void minProcessedTimeChanged();
    void maxProcessedTimeChanged();
    void minRatingChanged();
    void maxRatingChanged();

    void captureSortChanged();
    void importSortChanged();
    void processedSortChanged();
    void ratingSortChanged();
    void timeZoneChanged();

    void organizeFilterChanged();

    void imageCountChanged();

    void enqueueThis(const QString STsearchID);

    void updateTableOut(QString table, int operation);

public slots:

protected:
    QSqlQuery modelQuery();
    void emitChange() {emit organizeFilterChanged();}

    //Was the histogram query initialized yet?
    bool dateHistogramSet;

    QDate startCaptureDate;//start and end are for ranges; can be reversed unlike min/max
    QDate endCaptureDate;
    QDate minCaptureDate;
    QDate maxCaptureDate;
    QDate minImportDate;
    QDate maxImportDate;
    QDate minProcessedDate;
    QDate maxProcessedDate;
    qint64 minCaptureTime;
    qint64 maxCaptureTime;
    qint64 minImportTime;
    qint64 maxImportTime;
    qint64 minProcessedTime;
    qint64 maxProcessedTime;
    int minRating;
    int maxRating;
    int m_timeZone = 0;
    int m_imageCount;

    // For these sort variables, -1 means descending, +1 means ascending, 0 means inactive.
    int captureSort;
    int importSort;
    int processedSort;
    int ratingSort;

};


#endif // ORGANIZEMODEL_H
