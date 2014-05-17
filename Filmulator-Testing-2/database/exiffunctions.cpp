#include "exiffunctions.h"

int exifUtcTime( Exiv2::ExifData exifData, const int cameraTZ )
{
    QString exifDateTime = QString::fromStdString( exifData[ "Exif.Image.DateTime" ].toString() );

    QDateTime cameraDateTime = QDateTime::fromString( exifDateTime, "yyyy:MM:dd hh:mm:ss" );

    cameraDateTime.setUtcOffset( cameraTZ );

    return int( cameraDateTime.toTime_t() );
}

QString exifLocalDateString( Exiv2::ExifData exifData,
                             const int cameraTZ,
                             const int importTZ,
                             const QString dirConfig )
{
    QDateTime captureLocalDateTime =
            QDateTime::fromTime_t( exifUtcTime( exifData, cameraTZ ) );

    captureLocalDateTime.setUtcOffset( importTZ );

    return captureLocalDateTime.toString( dirConfig );
}

int exifDefaultRotation( Exiv2::ExifData exifData )
{
    int exifOrientation;
    try
    {
        exifOrientation = ( int ) exifData[ "Exif.Image.Orientation" ].value().toLong();
    }
    catch ( ... )
    {
        exifOrientation = 0;
    }
    switch( exifOrientation )
    {
    case 3://upside down
    {
        return 2;
    }
    case 6://right side down
    {
        return 3;
    }
    case 8://left side down
    {
        return 1;
    }
    default:
    {
        return 0;
    }
    }
}

QString exifMake( Exiv2::ExifData exifData )
{
    return QString::fromStdString( exifData[ "Exif.Image.Make" ].toString() );
}

QString exifModel( Exiv2::ExifData exifData )
{
    return QString::fromStdString( exifData[ "Exif.Image.Model" ].toString() );
}

float exifIso( Exiv2::ExifData exifData )
{
    return exifData [ "Exif.Photo.ISOSpeedRatings" ].toFloat();
}

QString exifTv( Exiv2::ExifData exifData )
{
    return QString::fromStdString( exifData[ "Exif.Photo.ExposureTime" ].toString() );
}

float exifAv( Exiv2::ExifData exifData )
{
    return exifData[ "Exif.Photo.FNumber" ].toFloat();
}

float exifFl( Exiv2::ExifData exifData )
{
    return exifData[ "Exif.Photo.FocalLength" ].toFloat();
}
