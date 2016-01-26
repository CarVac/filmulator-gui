#include "thumbWriteWorker.h"
#include <QDir>
#include <iostream>
using namespace std;

ThumbWriteWorker::ThumbWriteWorker(QObject *parent) : QObject(parent)
{
}

void ThumbWriteWorker::setImage(const matrix<unsigned short> imageIn,
                                Exiv2::ExifData dataIn)
{
    QMutexLocker locker(&dataMutex);
    image = imageIn;
    exifData = dataIn;
}

void ThumbWriteWorker::writeThumb(QString searchID)
{
    dataMutex.lock();
    int rows = image.nr();
    int cols = image.nc();

    QSqlQuery query;
    //If there was an error in the pipeline, the picture will be 0x0.
    if ((rows == 0) || (cols == 0))
    {
        query.prepare("UPDATE SearchTable SET STthumbWritten = -1 WHERE STsearchID = ?;");
        query.bindValue(0, searchID);
        query.exec();
        emit doneWritingThumb();
        dataMutex.unlock();
        return;
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
    dir.cd(".local/share/filmulator");
    if (!dir.cd("thumbs"))
    {
        dir.mkdir("thumbs");
        dir.cd("thumbs");
    }
    QString thumbDir = searchID;
    thumbDir.truncate(4);
    if (!dir.cd(thumbDir))
    {
        dir.mkdir(thumbDir);
        dir.cd(thumbDir);
    }
    QString outputFilename = dir.absoluteFilePath(searchID);

    //Then we write.
    imwrite_jpeg(gammaCurved, outputFilename.toStdString(), exifData, 95);

    query.prepare("UPDATE SearchTable SET STthumbWritten = 1 WHERE QTsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();

    emit doneWritingThumb();
}
