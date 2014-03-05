#include "organizemodel.h"
#include <QDebug>
#include <iostream>

//This file is a bunch of methods for changing the SELECT statement.

void OrganizeModel::setMinCaptureTime(unsigned int captureTimeIn)
{
    minCaptureTime = captureTimeIn;
    emit minCaptureTimeChanged();
}

void OrganizeModel::setMaxCaptureTime(unsigned int captureTimeIn)
{
    maxCaptureTime = captureTimeIn;
    emit maxCaptureTimeChanged();
}

void OrganizeModel::setMinImportTime(unsigned int importTimeIn)
{
    minImportTime = importTimeIn;
    emit minImportTimeChanged();
}

void OrganizeModel::setMaxImportTime(unsigned int importTimeIn)
{
    maxImportTime = importTimeIn;
    emit maxImportTimeChanged();
}

void OrganizeModel::setMinProcessedTime(unsigned int processedTimeIn)
{
    minProcessedTime = processedTimeIn;
    emit minImportTimeChanged();
}

void OrganizeModel::setMaxProcessedTime(unsigned int processedTimeIn)
{
    maxProcessedTime = processedTimeIn;
    emit maxProcessedTimeChanged();
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
