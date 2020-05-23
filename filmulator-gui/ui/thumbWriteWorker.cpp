#include "thumbWriteWorker.h"
#include <QDir>
#include <iostream>
#include "../database/database.hpp"
#include <../database/organizeModel.h>
using namespace std;

ThumbWriteWorker::ThumbWriteWorker(QObject *parent) : QObject(parent)
{
}

void ThumbWriteWorker::setImage(matrix<unsigned short> imageIn,
                                Exiv2::ExifData dataIn)
{
    QMutexLocker locker(&dataMutex);
    image = std::move(imageIn);
    exifData = dataIn;
}

//writeThumb writes the thumb if the image is non-zero sized.
bool ThumbWriteWorker::writeThumb(QString searchID)
{
    dataMutex.lock();
    int rows = image.nr();
    int cols = image.nc();

    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    QSqlQuery query(db);
    //If there was an error in the pipeline, the picture will be 0x0.
    if ((rows == 0) || (cols == 0))
    {
        query.prepare("UPDATE SearchTable SET STthumbWritten = -1 WHERE STsearchID = ?;");
        query.bindValue(0, searchID);
        query.exec();
        emit doneWritingThumb();
        dataMutex.unlock();
        return true;
    }

    matrix<float> linear;
    matrix<float> small;
    matrix<unsigned short> gammaCurved;

    //We need to linearize the brightness before scaling.

    sRGB_linearize(image, linear);
    dataMutex.unlock();
    downscale_and_crop(linear, small, 0, 0, (cols/3 -1), rows-1, 600, 600);
    //Then we put it back to the sRGB curve.
    sRGB_gammacurve(small, gammaCurved);

    //Set up the thumbnail directory.
    QDir dir = QDir::home();
    dir.cd(OrganizeModel::thumbDir());
    QString thumbDir = searchID;
    thumbDir.truncate(4);
    if (!dir.cd(thumbDir))
    {
        dir.mkdir(thumbDir);
        dir.cd(thumbDir);
    }
    QString outputFilename = dir.absoluteFilePath(searchID);
    cout << "Thumbnail being written to: " << outputFilename.toStdString() << endl;

    //Then we write.
    imwrite_jpeg(gammaCurved, outputFilename.toStdString(), exifData, 95, false);

    query.prepare("UPDATE SearchTable SET STthumbWritten = 1 WHERE STsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();

    emit doneWritingThumb();
    return false;
}
