#include "thumbWriteWorker.h"
#include <iostream>

ThumbWriteWorker::ThumbWriteWorker(QObject *parent) : QObject(parent)
{
}

void ThumbWriteWorker::setImage(const matrix<unsigned short> imageIn,
                                Exiv2::ExifData dataIn)
{
    image = imageIn;
    exifData = dataIn;
}

void ThumbWriteWorker::writeThumb(QString outputFilename)
{
    std::cout << "ThumbWriteWorker::writeThumb" << std::endl;
    int rows = image.nr();
    int cols = image.nc();

    matrix<float> linear;
    matrix<float> small;
    matrix<unsigned short> gammaCurved;

    //We need to linearize the brightness before scaling.
    sRGB_linearize(image, linear);
    downscale_and_crop(linear, small, 0, 0, (cols/3 -1), rows-1, 600, 600);
    //Then we put it back to the sRGB curve.
    sRGB_gammacurve(small, gammaCurved);

    //Then we write.
    imwrite_jpeg(gammaCurved, outputFilename.toStdString(), exifData, 95);

    emit doneWritingThumb();
}
