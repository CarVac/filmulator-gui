#include "sqlmodel.h"
#include <QDebug>
#include <iostream>

void SqlModel::setMinCaptureTime(unsigned int captureTimeIn)
{
    minCaptureTime = captureTimeIn;
    emit minCaptureTimeChanged();
}

void SqlModel::setMaxCaptureTime(unsigned int captureTimeIn)
{
    maxCaptureTime = captureTimeIn;
    emit maxCaptureTimeChanged();
}

void SqlModel::setMinImportTime(unsigned int importTimeIn)
{
    minImportTime = importTimeIn;
    emit minImportTimeChanged();
}

void SqlModel::setMaxImportTime(unsigned int importTimeIn)
{
    maxImportTime = importTimeIn;
    emit maxImportTimeChanged();
}

void SqlModel::setMinProcessedTime(unsigned int processedTimeIn)
{
    minProcessedTime = processedTimeIn;
    emit minImportTimeChanged();
}

void SqlModel::setMaxProcessedTime(unsigned int processedTimeIn)
{
    maxProcessedTime = processedTimeIn;
    emit maxProcessedTimeChanged();
}

void SqlModel::setMinRating(int ratingIn)
{
    minRating = ratingIn;
    emit minRatingChanged();
}

void SqlModel::setMaxRating(int ratingIn)
{
    maxRating = ratingIn;
    emit maxRatingChanged();
}

void SqlModel::setCaptureSort(int sortMode)
{
    captureSort = sortMode;
    emit captureSortChanged();
}

void SqlModel::setImportSort(int sortMode)
{
    importSort = sortMode;
    emit importSortChanged();
}

void SqlModel::setProcessedSort(int sortMode)
{
    processedSort = sortMode;
    emit processedSortChanged();
}

void SqlModel::setRatingSort(int sortMode)
{
    ratingSort = sortMode;
    emit ratingSortChanged();
}
