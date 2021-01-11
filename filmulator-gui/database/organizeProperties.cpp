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
    bool changed = (minCaptureDate == captureTimeIn ? false : true);

    minCaptureDate = captureTimeIn;
    if (maxCaptureDate < minCaptureDate)
    {
        setMaxCaptureTime(captureTimeIn);
    }
    //We want the beginning of the day.
    QDateTime tempTime = QDateTime(captureTimeIn, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    minCaptureTime = tempTime.toSecsSinceEpoch();
    emit minCaptureTimeChanged();

    if (changed)
    {
        emit organizeFilterChanged();
    }
}

void OrganizeModel::setMaxCaptureTime(QDate captureTimeIn)
{
    bool changed = (maxCaptureDate == captureTimeIn ? false : true);

    maxCaptureDate = captureTimeIn;
    if (minCaptureDate > maxCaptureDate)
    {
        setMinCaptureTime(captureTimeIn);
    }
    //We want the end of the day.
    QDateTime tempTime = QDateTime(captureTimeIn, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    maxCaptureTime = tempTime.toSecsSinceEpoch();
    emit maxCaptureTimeChanged();

    if (changed)
    {
        emit organizeFilterChanged();
    }
}

void OrganizeModel::setMinMaxCaptureTime(QDate captureTimeIn)
{
    startCaptureDate = captureTimeIn;
    endCaptureDate = captureTimeIn;
    minCaptureDate = captureTimeIn;
    maxCaptureDate = captureTimeIn;
    QDateTime morning = QDateTime(captureTimeIn, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    QDateTime evening = QDateTime(captureTimeIn, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    minCaptureTime = morning.toSecsSinceEpoch();
    maxCaptureTime = evening.toSecsSinceEpoch();
    emit minCaptureTimeChanged();
    emit maxCaptureTimeChanged();
    emit captureDateChanged();
    emit organizeFilterChanged();
}
void OrganizeModel::setMinMaxCaptureTimeString(QString captureTimeIn)
{
    startCaptureDate = QDate::fromString(captureTimeIn,"yyyy/MM/dd");
    endCaptureDate = startCaptureDate;
    minCaptureDate = startCaptureDate;
    maxCaptureDate = startCaptureDate;
    QDateTime morning = QDateTime(minCaptureDate, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    QDateTime evening = QDateTime(maxCaptureDate, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    minCaptureTime = morning.toSecsSinceEpoch();
    maxCaptureTime = evening.toSecsSinceEpoch();
    emit minCaptureTimeChanged();
    emit maxCaptureTimeChanged();
    emit captureDateChanged();
    emit organizeFilterChanged();
}

//This is used by the date histogram to select multiple days.
void OrganizeModel::extendMinMaxCaptureTimeString(QString captureTimeIn)
{
    endCaptureDate = QDate::fromString(captureTimeIn,"yyyy/MM/dd");
    if (endCaptureDate < startCaptureDate)
    {
        minCaptureDate = endCaptureDate;
        maxCaptureDate = startCaptureDate;
    }
    else
    {
        minCaptureDate = startCaptureDate;
        maxCaptureDate = endCaptureDate;
    }
    QDateTime morning = QDateTime(minCaptureDate, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    QDateTime evening = QDateTime(maxCaptureDate, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    minCaptureTime = morning.toSecsSinceEpoch();
    maxCaptureTime = evening.toSecsSinceEpoch();
    emit minCaptureTimeChanged();
    emit maxCaptureTimeChanged();
    emit captureDateChanged();
    emit organizeFilterChanged();
}

//For arrow key shortcut
void OrganizeModel::incrementCaptureTime(int days)
{
    startCaptureDate = endCaptureDate.addDays(days);
    if (startCaptureDate > QDate::currentDate())
    {
        startCaptureDate = QDate::currentDate();
    }

    endCaptureDate = startCaptureDate;
    minCaptureDate = startCaptureDate;
    maxCaptureDate = startCaptureDate;
    QDateTime morning = QDateTime(minCaptureDate, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    QDateTime evening = QDateTime(maxCaptureDate, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    minCaptureTime = morning.toSecsSinceEpoch();
    maxCaptureTime = evening.toSecsSinceEpoch();
    emit minCaptureTimeChanged();
    emit maxCaptureTimeChanged();
    emit captureDateChanged();
    emit organizeFilterChanged();
}

//For shift-arrow key shortcut
void OrganizeModel::extendCaptureTimeRange(int days)
{
    endCaptureDate = endCaptureDate.addDays(days);
    if (endCaptureDate > QDate::currentDate())
    {
        endCaptureDate = QDate::currentDate();
    }

    if (endCaptureDate < startCaptureDate)
    {
        minCaptureDate = endCaptureDate;
        maxCaptureDate = startCaptureDate;
    }
    else
    {
        minCaptureDate = startCaptureDate;
        maxCaptureDate = endCaptureDate;
    }
    QDateTime morning = QDateTime(minCaptureDate, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    QDateTime evening = QDateTime(maxCaptureDate, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    minCaptureTime = morning.toSecsSinceEpoch();
    maxCaptureTime = evening.toSecsSinceEpoch();
    emit minCaptureTimeChanged();
    emit maxCaptureTimeChanged();
    emit captureDateChanged();
    emit organizeFilterChanged();
}

//This checks whether a day is selected, for use in the date histogram (and maybe calendar)
bool OrganizeModel::isDateSelected(QString captureTimeIn)
{
    QDate testCaptureTime = QDate::fromString(captureTimeIn,"yyyy/MM/dd");
    if (testCaptureTime >= minCaptureDate && testCaptureTime <= maxCaptureDate)
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
    return QDateTime(minCaptureDate.addDays(1), QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600).date();
}

QString OrganizeModel::getSelectedYMDString()
{
    QDate selectedDate = getSelectedDate();
    return selectedDate.toString("yyyy/MM/dd");
}


void OrganizeModel::setMinImportTime(QDate importTimeIn)
{
    bool changed = (minImportDate == importTimeIn ? false : true);

    minImportDate = importTimeIn;
    if (maxImportDate < minImportDate)
    {
        setMaxImportTime(importTimeIn);
    }
    //We want the beginning of the day.
    QDateTime tempTime = QDateTime(importTimeIn, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    minImportTime = tempTime.toSecsSinceEpoch();
    emit minImportTimeChanged();

    if (changed)
    {
        emit organizeFilterChanged();
    }
}

void OrganizeModel::setMaxImportTime(QDate importTimeIn)
{
    bool changed = (maxImportDate == importTimeIn ? false : true);

    maxImportDate = importTimeIn;
    if (minImportDate > maxImportDate)
    {
        setMinImportTime(importTimeIn);
    }
    //We want the end of the day.
    QDateTime tempTime = QDateTime(importTimeIn, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    maxImportTime = tempTime.toSecsSinceEpoch();
    emit maxImportTimeChanged();

    if (changed)
    {
        emit organizeFilterChanged();
    }
}

void OrganizeModel::setMinProcessedTime(QDate processedTimeIn)
{
    bool changed = (minProcessedDate == processedTimeIn ? false : true);

    minProcessedDate = processedTimeIn;
    if (maxProcessedDate < minProcessedDate)
    {
        setMaxProcessedTime(processedTimeIn);
    }
    //We want the beginning of the day.
    QDateTime tempTime = QDateTime(processedTimeIn, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    minProcessedTime = tempTime.toSecsSinceEpoch();
    emit minProcessedTimeChanged();

    if (changed)
    {
        emit organizeFilterChanged();
    }
}

void OrganizeModel::setMaxProcessedTime(QDate processedTimeIn)
{
    bool changed = (maxProcessedDate == processedTimeIn ? false : true);

    maxProcessedDate = processedTimeIn;
    if (minProcessedDate > maxProcessedDate)
    {
        setMinProcessedTime(processedTimeIn);
    }
    //We want the end of the day.
    QDateTime tempTime = QDateTime(processedTimeIn, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    maxProcessedTime = tempTime.toSecsSinceEpoch();
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
    setMinCaptureTime(minCaptureDate);
    setMaxCaptureTime(maxCaptureDate);
    setMinImportTime(minImportDate);
    setMaxImportTime(maxImportDate);
    setMinProcessedTime(minProcessedDate);
    setMaxProcessedTime(maxProcessedDate);
    emit organizeFilterChanged();
    emit captureDateChanged();

    //The date histogram must only be updated after it was initialized.
    if (dateHistogramSet)
    {
        dateHistogram->signalChange();
    }
}

