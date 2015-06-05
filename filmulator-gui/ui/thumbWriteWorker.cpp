#include "thumbWriteWorker.h"
#include <iostream>

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

void ThumbWriteWorker::writeThumb(QString outputFilename)
{
    std::cout << "writeThumb before mutex" << std::endl;
    dataMutex.lock();
    int rows = image.nr();
    int cols = image.nc();

    matrix<float> linear;
    matrix<float> small;
    matrix<unsigned short> gammaCurved;

    //We need to linearize the brightness before scaling.

    std::cout << "writeThumb before linearize" << std::endl;
    sRGB_linearize(image, linear);
    dataMutex.unlock();
    std::cout << "writeThumb before downscale" << std::endl;
    downscale_and_crop(linear, small, 0, 0, (cols/3 -1), rows-1, 600, 600);
    //Then we put it back to the sRGB curve.
    std::cout << "writeThumb before gamma" << std::endl;
    sRGB_gammacurve(small, gammaCurved);

    //Then we write.
    std::cout << "writeThumb before write" << std::endl;
    imwrite_jpeg(gammaCurved, outputFilename.toStdString(), exifData, 95);

    std::cout << "writeThumb done" << std::endl;
    emit doneWritingThumb();
}
