#include "organizemodel.h"
#include <exiv2/exiv2.hpp>
#include <QDateTime>
#include <QString>
#include <iostream>

/*This function inserts info on a raw file into the database.*/
void OrganizeModel::fileInsert( const QString hash,
                                const QString filePathName,
                                Exiv2::ExifData exifData)
{
    QSqlQuery query;
    cout << "Before replace into" << endl;
    query.prepare( "REPLACE INTO FileTable values (?,?,?,?,?,?,?,?,?);");
    //Hash of the file:
    query.bindValue( 0, hash );
    //Full path to the new location of the file:
    query.bindValue( 1, filePathName );
    cout << "After" << endl;
    //Camera manufacturer
    query.bindValue( 2, QString::fromStdString( exifData[ "Exif.Image.Make" ].toString() ) );
    //Camera model
    query.bindValue( 3, QString::fromStdString( exifData[ "Exif.Image.Model" ].toString() ) );
    //ISO sensitivity
    query.bindValue( 4, exifData[ "Exif.Photo.ISOSpeedRatings" ].toFloat() );
    //Exposure time
    query.bindValue( 5, QString::fromStdString( exifData[ "Exif.Photo.ExposureTime" ].toString() ) );
    //Aperture number
    query.bindValue( 6, exifData[ "Exif.Photo.FNumber" ].toFloat() );
    //Focal length
    query.bindValue( 7, exifData[ "Exif.Photo.FocalLength" ].toFloat() );
    //Initialize a counter at zero for number of times it has been referenced.
    // We only do this for new imports into the database.
    query.bindValue( 8, 0 );
    query.exec();
}
