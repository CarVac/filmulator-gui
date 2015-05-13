#ifndef ORGANIZEMODEL_H
#define ORGANIZEMODEL_H

#include "sqlModel.h"
#include "basicSqlModel.h"
#include <QString>
#include <QDateTime>
#include <QTimeZone>
#include <QByteArray>
#include <exiv2/exiv2.hpp>

class OrganizeModel : public SqlModel
{
    Q_OBJECT
    Q_PROPERTY(QDate minCaptureTime   READ getMinCaptureTime   WRITE setMinCaptureTime   NOTIFY minCaptureTimeChanged)
    Q_PROPERTY(QDate maxCaptureTime   READ getMaxCaptureTime   WRITE setMaxCaptureTime   NOTIFY maxCaptureTimeChanged)
    Q_PROPERTY(QDate minImportTime    READ getMinImportTime    WRITE setMinImportTime    NOTIFY minImportTimeChanged)
    Q_PROPERTY(QDate maxImportTime    READ getMaxImportTime    WRITE setMaxImportTime    NOTIFY maxImportTimeChanged)
    Q_PROPERTY(QDate minProcessedTime READ getMinProcessedTime WRITE setMinProcessedTime NOTIFY minProcessedTimeChanged)
    Q_PROPERTY(QDate maxProcessedTime READ getMaxProcessedTime WRITE setMaxProcessedTime NOTIFY maxProcessedTimeChanged)
    Q_PROPERTY(int minRating     READ getMinRating     WRITE setMinRating     NOTIFY minRatingChanged)
    Q_PROPERTY(int maxRating     READ getMaxRating     WRITE setMaxRating     NOTIFY maxRatingChanged)
    Q_PROPERTY(int captureSort   READ getCaptureSort   WRITE setCaptureSort   NOTIFY captureSortChanged)
    Q_PROPERTY(int importSort    READ getImportSort    WRITE setImportSort    NOTIFY importSortChanged)
    Q_PROPERTY(int processedSort READ getProcessedSort WRITE setProcessedSort NOTIFY processedSortChanged)
    Q_PROPERTY(int ratingSort    READ getRatingSort    WRITE setRatingSort    NOTIFY ratingSortChanged)
    Q_PROPERTY(int timeZone MEMBER m_timeZone WRITE setTimeZone NOTIFY timeZoneChanged)

public:
    explicit OrganizeModel(QObject *parent = 0);
    Q_INVOKABLE void setOrganizeQuery();
    Q_INVOKABLE void setDateHistoQuery();
    Q_INVOKABLE QString thumbDir();
    Q_INVOKABLE void setRating(QString searchID, int rating);
    Q_INVOKABLE QString getDateTimeString(int unixTimeIn);

    BasicSqlModel *dateHistogram = new BasicSqlModel;

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

    QDate getMinCaptureTime() {return minCaptureTime;}
    QDate getMaxCaptureTime() {return maxCaptureTime;}
    QDate getMinImportTime() {return minImportTime;}
    QDate getMaxImportTime() {return maxImportTime;}
    QDate getMinProcessedTime() {return minProcessedTime;}
    QDate getMaxProcessedTime() {return maxProcessedTime;}
    int getMinRating() {return minRating;}
    int getMaxRating() {return maxRating;}

    int getCaptureSort() {return captureSort;}
    int getImportSort() {return importSort;}
    int getProcessedSort() {return processedSort;}
    int getRatingSort() {return ratingSort;}

signals:
    void minCaptureTimeChanged();
    void maxCaptureTimeChanged();
    void minImportTimeChanged();
    void maxImportTimeChanged();
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

public slots:

protected:
    QSqlQuery modelQuery();
    QSqlQuery dateHistoQuery();
    void emitChange() {emit organizeFilterChanged();}

    //Was the histogram query initialized yet?
    bool dateHistogramSet;

    QDate minCaptureTime;
    QDate maxCaptureTime;
    QDate minImportTime;
    QDate maxImportTime;
    QDate minProcessedTime;
    QDate maxProcessedTime;
    unsigned int minCaptureTime_i;
    unsigned int maxCaptureTime_i;
    unsigned int minImportTime_i;
    unsigned int maxImportTime_i;
    unsigned int minProcessedTime_i;
    unsigned int maxProcessedTime_i;
    int minRating;
    int maxRating;
    int m_timeZone = 0;

    // For these sort variables, -1 means descending, +1 means ascending, 0 means inactive.
    int captureSort;
    int importSort;
    int processedSort;
    int ratingSort;

};


#endif // ORGANIZEMODEL_H
