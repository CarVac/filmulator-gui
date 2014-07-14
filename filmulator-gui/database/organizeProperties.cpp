#include "organizeModel.h"
#include <QDebug>
#include <iostream>

using std::cout;
using std::endl;

//This file is a bunch of methods for changing the SELECT statement.

void OrganizeModel::setMinCaptureTime(QDate captureTimeIn)
{
    minCaptureTime = captureTimeIn;
    if (maxCaptureTime < minCaptureTime)
    {
        setMaxCaptureTime(captureTimeIn);
    }
    //We want the beginning of the day.
    QDateTime tempTime = QDateTime(captureTimeIn, QTime(0,0,0,0), QTimeZone(m_timeZone*3600));
    minCaptureTime_i = tempTime.toTime_t();
    emit minCaptureTimeChanged();
    emit organizeFilterChanged();
}

void OrganizeModel::setMaxCaptureTime(QDate captureTimeIn)
{
    maxCaptureTime = captureTimeIn;
    if (minCaptureTime > maxCaptureTime)
    {
        setMinCaptureTime(captureTimeIn);
    }
    //We want the end of the day.
    QDateTime tempTime = QDateTime(captureTimeIn, QTime(23,59,59,999), QTimeZone(m_timeZone*3600));
    maxCaptureTime_i = tempTime.toTime_t();
    emit maxCaptureTimeChanged();
    emit organizeFilterChanged();
}

void OrganizeModel::setMinImportTime(QDate importTimeIn)
{
    minImportTime = importTimeIn;
    if (maxImportTime < minImportTime)
    {
        setMaxImportTime(importTimeIn);
    }
    //We want the beginning of the day.
    QDateTime tempTime = QDateTime(importTimeIn, QTime(0,0,0,0), QTimeZone(m_timeZone*3600));
    minImportTime_i = tempTime.toTime_t();
    emit minImportTimeChanged();
    emit organizeFilterChanged();
}

void OrganizeModel::setMaxImportTime(QDate importTimeIn)
{
    maxImportTime = importTimeIn;
    if (minImportTime > maxImportTime)
    {
        setMinImportTime(importTimeIn);
    }
    //We want the end of the day.
    QDateTime tempTime = QDateTime(importTimeIn, QTime(23,59,59,999), QTimeZone(m_timeZone*3600));
    maxImportTime_i = tempTime.toTime_t();
    emit maxImportTimeChanged();
    emit organizeFilterChanged();
}

void OrganizeModel::setMinProcessedTime(QDate processedTimeIn)
{
    minProcessedTime = processedTimeIn;
    if (maxProcessedTime < minProcessedTime)
    {
        setMaxProcessedTime(processedTimeIn);
    }
    //We want the beginning of the day.
    QDateTime tempTime = QDateTime(processedTimeIn, QTime(0,0,0,0), QTimeZone(m_timeZone*3600));
    minProcessedTime_i = tempTime.toTime_t();
    emit minProcessedTimeChanged();
    emit organizeFilterChanged();
}

void OrganizeModel::setMaxProcessedTime(QDate processedTimeIn)
{
    maxProcessedTime = processedTimeIn;
    if (minProcessedTime > maxProcessedTime)
    {
        setMinProcessedTime(processedTimeIn);
    }
    //We want the end of the day.
    QDateTime tempTime = QDateTime(processedTimeIn, QTime(23,59,59,999), QTimeZone(m_timeZone*3600));
    maxProcessedTime_i = tempTime.toTime_t();
    emit maxProcessedTimeChanged();
    emit organizeFilterChanged();
}

void OrganizeModel::setMinRating(int ratingIn)
{
    minRating = ratingIn;
    emit minRatingChanged();
}

void OrganizeModel::setMaxRating(int ratingIn)
{
    maxRating = ratingIn;
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
    setMinCaptureTime(minCaptureTime);
    setMaxCaptureTime(maxCaptureTime);
    setMinImportTime(minImportTime);
    setMaxImportTime(maxImportTime);
    setMinProcessedTime(minProcessedTime);
    setMaxProcessedTime(maxProcessedTime);
}

