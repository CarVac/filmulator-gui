#include "organizeModel.h"
#include <QDebug>
#include <iostream>

using std::cout;
using std::endl;

//This file is a bunch of methods for changing the SELECT statement.

void OrganizeModel::setMinCaptureTime(QDate captureTimeIn)
{
    //If the value didn't change, this was probably triggered by the timezone changing.
    //That changes EVERYTHING, so we don't want it updating the organize filter until the end.
    bool changed = (minCaptureTime == captureTimeIn ? false : true);

    minCaptureTime = captureTimeIn;
    if (maxCaptureTime < minCaptureTime)
    {
        setMaxCaptureTime(captureTimeIn);
    }
    //We want the beginning of the day.
    QDateTime tempTime = QDateTime(captureTimeIn, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    minCaptureTime_i = tempTime.toTime_t();
    emit minCaptureTimeChanged();

    if (changed)
    {
        emit organizeFilterChanged();
    }
}

void OrganizeModel::setMaxCaptureTime(QDate captureTimeIn)
{
    bool changed = (maxCaptureTime == captureTimeIn ? false : true);

    maxCaptureTime = captureTimeIn;
    if (minCaptureTime > maxCaptureTime)
    {
        setMinCaptureTime(captureTimeIn);
    }
    //We want the end of the day.
    QDateTime tempTime = QDateTime(captureTimeIn, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    maxCaptureTime_i = tempTime.toTime_t();
    emit maxCaptureTimeChanged();

    if (changed)
    {
        emit organizeFilterChanged();
    }
}

void OrganizeModel::setMinMaxCaptureTime(QDate captureTimeIn)
{
    minCaptureTime = captureTimeIn;
    maxCaptureTime = captureTimeIn;
    QDateTime morning = QDateTime(captureTimeIn, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    QDateTime evening = QDateTime(captureTimeIn, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    minCaptureTime_i = morning.toTime_t();
    maxCaptureTime_i = evening.toTime_t();
    emit minCaptureTimeChanged();
    emit maxCaptureTimeChanged();
    emit captureDateChanged();
    emit organizeFilterChanged();
}
void OrganizeModel::setMinMaxCaptureTimeString(QString captureTimeIn)
{
    startCaptureTime = QDate::fromString(captureTimeIn,"yyyy/MM/dd");
    minCaptureTime = startCaptureTime;//QDate::fromString(captureTimeIn,"yyyy/MM/dd");
    maxCaptureTime = startCaptureTime;//QDate::fromString(captureTimeIn,"yyyy/MM/dd");
    QDateTime morning = QDateTime(minCaptureTime, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    QDateTime evening = QDateTime(maxCaptureTime, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    minCaptureTime_i = morning.toTime_t();
    maxCaptureTime_i = evening.toTime_t();
    emit minCaptureTimeChanged();
    emit maxCaptureTimeChanged();
    emit captureDateChanged();
    emit organizeFilterChanged();
}

//This is used by the date histogram to select multiple days.
void OrganizeModel::extendMinMaxCaptureTimeString(QString captureTimeIn)
{
    QDate tempMinCaptureTime = QDate::fromString(captureTimeIn,"yyyy/MM/dd");
    QDate tempMaxCaptureTime = QDate::fromString(captureTimeIn,"yyyy/MM/dd");
    if (tempMinCaptureTime < startCaptureTime)
    {
        minCaptureTime = tempMinCaptureTime;
    }
    else
    {
        minCaptureTime = startCaptureTime;
    }
    if (tempMaxCaptureTime > startCaptureTime)
    {
        maxCaptureTime = tempMaxCaptureTime;
    }
    else
    {
        maxCaptureTime = startCaptureTime;
    }
    QDateTime morning = QDateTime(minCaptureTime, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    QDateTime evening = QDateTime(maxCaptureTime, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    minCaptureTime_i = morning.toTime_t();
    maxCaptureTime_i = evening.toTime_t();
    emit minCaptureTimeChanged();
    emit maxCaptureTimeChanged();
    emit captureDateChanged();
    emit organizeFilterChanged();
}

//This checks whether a day is selected, for use in the date histogram (and maybe calendar)
bool OrganizeModel::isDateSelected(QString captureTimeIn)
{
    QDate testCaptureTime = QDate::fromString(captureTimeIn,"yyyy/MM/dd");
    if (testCaptureTime >= minCaptureTime && testCaptureTime <= maxCaptureTime)
    {
        return true;
    }
    else
    {
        return false;
    }

}

QDate OrganizeModel::getSelectedDate()
{
    return QDateTime(minCaptureTime.addDays(1), QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600).date();
}

QString OrganizeModel::getSelectedYMDString()
{
    QDate selectedDate = getSelectedDate();
    return selectedDate.toString("yyyy/MM/dd");
}


void OrganizeModel::setMinImportTime(QDate importTimeIn)
{
    bool changed = (minImportTime == importTimeIn ? false : true);

    minImportTime = importTimeIn;
    if (maxImportTime < minImportTime)
    {
        setMaxImportTime(importTimeIn);
    }
    //We want the beginning of the day.
    QDateTime tempTime = QDateTime(importTimeIn, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    minImportTime_i = tempTime.toTime_t();
    emit minImportTimeChanged();

    if (changed)
    {
        emit organizeFilterChanged();
    }
}

void OrganizeModel::setMaxImportTime(QDate importTimeIn)
{
    bool changed = (maxImportTime == importTimeIn ? false : true);

    maxImportTime = importTimeIn;
    if (minImportTime > maxImportTime)
    {
        setMinImportTime(importTimeIn);
    }
    //We want the end of the day.
    QDateTime tempTime = QDateTime(importTimeIn, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    maxImportTime_i = tempTime.toTime_t();
    emit maxImportTimeChanged();

    if (changed)
    {
        emit organizeFilterChanged();
    }
}

void OrganizeModel::setMinProcessedTime(QDate processedTimeIn)
{
    bool changed = (minProcessedTime == processedTimeIn ? false : true);

    minProcessedTime = processedTimeIn;
    if (maxProcessedTime < minProcessedTime)
    {
        setMaxProcessedTime(processedTimeIn);
    }
    //We want the beginning of the day.
    QDateTime tempTime = QDateTime(processedTimeIn, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    minProcessedTime_i = tempTime.toTime_t();
    emit minProcessedTimeChanged();

    if (changed)
    {
        emit organizeFilterChanged();
    }
}

void OrganizeModel::setMaxProcessedTime(QDate processedTimeIn)
{
    bool changed = (maxProcessedTime == processedTimeIn ? false : true);

    maxProcessedTime = processedTimeIn;
    if (minProcessedTime > maxProcessedTime)
    {
        setMinProcessedTime(processedTimeIn);
    }
    //We want the end of the day.
    QDateTime tempTime = QDateTime(processedTimeIn, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    maxProcessedTime_i = tempTime.toTime_t();
    emit maxProcessedTimeChanged();

    if (changed)
    {
        emit organizeFilterChanged();
    }
}

void OrganizeModel::setMinRating(int ratingIn)
{
    bool changed = (minRating == ratingIn ? false : true);

    if (ratingIn < 0)
    {
        minRating = -5;
    } else {
        minRating = ratingIn;
    }
    emit minRatingChanged();

    if (changed)
    {
        emit organizeFilterChanged();

        //The date histogram must only be updated after it was initialized.
        if (dateHistogramSet)
        {
            //Tell the dateHistogram to emit its change signal
            dateHistogram->signalChange();
        }
    }
}

void OrganizeModel::setMaxRating(int ratingIn)
{
    bool changed = (maxRating == ratingIn ? false : true);

    maxRating = ratingIn;
    emit maxRatingChanged();

    if (changed)
    {
        emit organizeFilterChanged();

        //The date histogram must only be updated after it was initialized.
        if (dateHistogramSet)
        {
            //Tell the dateHistogram to emit its change signal
            dateHistogram->signalChange();
        }
    }
}

void OrganizeModel::setCaptureSort(int sortMode)
{
    bool changed = (captureSort == sortMode ? false : true);

    captureSort = sortMode;
    emit captureSortChanged();

    if (changed)
    {
        emit organizeFilterChanged();
    }
}

void OrganizeModel::setImportSort(int sortMode)
{
    bool changed = (importSort == sortMode ? false : true);

    importSort = sortMode;
    emit importSortChanged();

    if (changed)
    {
        emit organizeFilterChanged();
    }
}

void OrganizeModel::setProcessedSort(int sortMode)
{
    bool changed = (processedSort == sortMode ? false : true);

    processedSort = sortMode;
    emit processedSortChanged();

    if (changed)
    {
        emit organizeFilterChanged();
    }
}

void OrganizeModel::setRatingSort(int sortMode)
{
    bool changed = (ratingSort == sortMode ? false : true);

    ratingSort = sortMode;
    emit ratingSortChanged();

    if (changed)
    {
        emit organizeFilterChanged();
    }
}

void OrganizeModel::setTimeZone(int timeZoneIn)
{
    m_timeZone = timeZoneIn;
    //cout << "organizeModel::setTimeZone: " << timeZoneIn << endl;
    setMinCaptureTime(minCaptureTime);
    setMaxCaptureTime(maxCaptureTime);
    setMinImportTime(minImportTime);
    setMaxImportTime(maxImportTime);
    setMinProcessedTime(minProcessedTime);
    setMaxProcessedTime(maxProcessedTime);
    emit organizeFilterChanged();
    emit captureDateChanged();

    //The date histogram must only be updated after it was initialized.
    if (dateHistogramSet)
    {
        dateHistogram->signalChange();
    }
}

