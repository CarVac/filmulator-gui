#include "organizeModel.h"
#include <QDebug>
#include <iostream>

using std::cout;
using std::endl;

//This file is a bunch of methods for changing the SELECT statement.

void OrganizeModel::setMinCaptureTime(QDate captureDateIn)
{
    minCaptureTime = captureDateIn;
    if (maxCaptureTime < minCaptureTime)
    {
        setMaxCaptureTime(captureDateIn);
    }
    //We want the beginning of the day.
    QDateTime tempTime = QDateTime(captureDateIn, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    minCaptureTime_i = tempTime.toTime_t();
    emit minCaptureTimeChanged();
    emit organizeFilterChanged();
}

void OrganizeModel::setMaxCaptureTime(QDate captureDateIn)
{
    maxCaptureTime = captureDateIn;
    if (minCaptureTime > maxCaptureTime)
    {
        setMinCaptureTime(captureDateIn);
    }
    //We want the end of the day.
    QDateTime tempTime = QDateTime(captureDateIn, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    maxCaptureTime_i = tempTime.toTime_t();
    emit maxCaptureTimeChanged();
    emit organizeFilterChanged();
}

void OrganizeModel::setMinImportTime(QDate importDateIn)
{
    minImportTime = importDateIn;
    if (maxImportTime < minImportTime)
    {
        setMaxImportTime(importDateIn);
    }
    //We want the beginning of the day.
    QDateTime tempTime = QDateTime(importDateIn, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    minImportTime_i = tempTime.toTime_t();
    emit minImportTimeChanged();
    emit organizeFilterChanged();
}

void OrganizeModel::setMaxImportTime(QDate importDateIn)
{
    maxImportTime = importDateIn;
    if (minImportTime > maxImportTime)
    {
        setMinImportTime(importDateIn);
    }
    //We want the end of the day.
    QDateTime tempTime = QDateTime(importDateIn, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    maxImportTime_i = tempTime.toTime_t();
    emit maxImportTimeChanged();
    emit organizeFilterChanged();
}

void OrganizeModel::setMinProcessedTime(QDate processedDateIn)
{
    minProcessedTime = processedDateIn;
    if (maxProcessedTime < minProcessedTime)
    {
        setMaxProcessedTime(processedDateIn);
    }
    //We want the beginning of the day.
    QDateTime tempTime = QDateTime(processedDateIn, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    minProcessedTime_i = tempTime.toTime_t();
    emit minProcessedTimeChanged();
    emit organizeFilterChanged();
}

void OrganizeModel::setMaxProcessedTime(QDate processedDateIn)
{
    maxProcessedTime = processedDateIn;
    if (minProcessedTime > maxProcessedTime)
    {
        setMinProcessedTime(processedDateIn);
    }
    //We want the end of the day.
    QDateTime tempTime = QDateTime(processedDateIn, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    maxProcessedTime_i = tempTime.toTime_t();
    emit maxProcessedTimeChanged();
    emit organizeFilterChanged();
}

void OrganizeModel::setMinRating(int ratingIn)
{
    minRating = ratingIn;
    dateHistogram->signalChange();
    emit minRatingChanged();
}

void OrganizeModel::setMaxRating(int ratingIn)
{
    maxRating = ratingIn;
    dateHistogram->signalChange();
    emit maxRatingChanged();
}

void OrganizeModel::setCaptureSort(int sortMode)
{
    captureSort = sortMode;
    emit captureSortChanged();
}

void OrganizeModel::setImportSort(int sortMode)
{
    importSort = sortMode;
    emit importSortChanged();
}

void OrganizeModel::setProcessedSort(int sortMode)
{
    processedSort = sortMode;
    emit processedSortChanged();
}

void OrganizeModel::setRatingSort(int sortMode)
{
    ratingSort = sortMode;
    emit ratingSortChanged();
}

void OrganizeModel::setTimeZone(int timeZoneIn)
{
    m_timeZone = timeZoneIn;
    cout << "organizeModel::setTimeZone: " << timeZoneIn << endl;
    setMinCaptureTime(minCaptureTime);
    setMaxCaptureTime(maxCaptureTime);
    setMinImportTime(minImportTime);
    setMaxImportTime(maxImportTime);
    setMinProcessedTime(minProcessedTime);
    setMaxProcessedTime(maxProcessedTime);
}

