#include "organizemodel.h"
#include "../core/filmsim.hpp"
#include <exiv2/exiv2.hpp>
#include <QDateTime>
#include <QString>
#include <iostream>
#include "exiffunctions.h"
#include <QDebug>
#include <QSqlError>

/*This function inserts info on a raw file into the database.*/
void OrganizeModel::fileInsert( const QString hash,
                                const QString filePathName,
                                Exiv2::ExifData exifData)
{
    QSqlQuery query;
    cout << "Before replace into" << endl;
    query.prepare( "REPLACE INTO FileTable values (?,?,?,?,?,?,?,?,?);");
                                                 //0 1 2 3 4 5 6 7 8
    //Hash of the file:
    query.bindValue( 0, hash );
    //Full path to the new location of the file:
    query.bindValue( 1, filePathName );
    cout << "After" << endl;
    //Camera manufacturer
    query.bindValue( 2, exifMake( exifData ) );
    //Camera model
    query.bindValue( 3, exifModel( exifData ) );
    //ISO sensitivity
    query.bindValue( 4, exifIso( exifData ) );
    //Exposure time
    query.bindValue( 5, exifTv( exifData ) );
    //Aperture number
    query.bindValue( 6, exifAv( exifData ) );
    //Focal length
    query.bindValue( 7, exifFl( exifData ) );
    //Initialize a counter at 0 for number of times it has been referenced.
    // We only do this for new imports into the database.
    query.bindValue( 8, 0 );
    query.exec();
}

/*This function creates a default profile in the profile table, and a search entry in the searchtable.*/
void OrganizeModel::createNewProfile( const QString fileHash,
                                      const QString fileName,
                                      const QString absoluteFilePath,
                                      const int captureTime,
                                      Exiv2::ExifData exifData)
{
    QSqlQuery query;
    //Retrieve the usage count from the file table, and increment it by one.
    query.prepare( "SELECT usageIncrement FROM FileTable WHERE ( fileID = ? );" );
    query.bindValue( 0, fileHash );
    query.exec();
    query.next();
    int increment = query.value( 0 ).toInt();
    increment++;
    query.prepare( "UPDATE FileTable SET usageIncrement = ? WHERE fileID = ?;" );
    query.bindValue( 0, increment );
    query.bindValue( 1, fileHash );
    query.exec();

    //Create a new search table entry
    query.prepare( "INSERT INTO SearchTable values (?,?,?,?,?,?,?,?,?,?);" );
                                                  //0 1 2 3 4 5 6 7 8 9

    //searchID (filehash with the increment appended)
    QString searchID = fileHash;
    cout << "increment: " << increment << endl;
    searchID.append( QString( "%1" ).arg( increment, 4, 10, QLatin1Char( '0' ) ) );
    query.bindValue(  0, searchID );
    //captureTime (unix time)
    query.bindValue(  1, captureTime );
    //name (of instance)
    query.bindValue(  2, "" );
    //filename
    query.bindValue(  3, fileName );
    //sourceHash
    query.bindValue(  4, fileHash );
    //rating
    //TODO: write function to get rating
    query.bindValue(  5, 0 );
    //latitude
    //TODO: figure something out here to either grab from the exif or get user input.
    query.bindValue(  6, 0 );
    //longitude
    query.bindValue(  7, 0 );

    //importTime (unix time)
    QDateTime currentTime = QDateTime::currentDateTime();
    query.bindValue(  8, currentTime.toTime_t() );
    //lastProcessedTime (unix time)
    //It's the same as above, since we're making a new one.
    query.bindValue(  9, currentTime.toTime_t() );

    query.exec();


    //Now we make a new profile.
    query.prepare( "INSERT INTO ProcessingTable values "
                   "(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
                   //0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
                   //                    1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3

    //First we need to retrieve the defaults.
    QSqlQuery defaultQuery;
    defaultQuery.exec( "SELECT * FROM ProfileTable WHERE ( ProfileID = \"Default\" )");
    defaultQuery.next();
    //procID (same as searchID in SearchTable)
    query.bindValue(  0, searchID );
    //Here, we pull the values from the default profile.
    query.bindValue(  1, defaultQuery.value(  1 ).toFloat() );//initialDeveloperConcentration
    query.bindValue(  2, defaultQuery.value(  2 ).toFloat() );//reservoirThickness
    query.bindValue(  3, defaultQuery.value(  3 ).toFloat() );//activeLayerThickness
    query.bindValue(  4, defaultQuery.value(  4 ).toFloat() );//crystalsPerPixel
    query.bindValue(  5, defaultQuery.value(  5 ).toFloat() );//initialCrystalRadius
    query.bindValue(  6, defaultQuery.value(  6 ).toFloat() );//initialSilverSaltDensity
    query.bindValue(  7, defaultQuery.value(  7 ).toFloat() );//developerConsumptionConst
    query.bindValue(  8, defaultQuery.value(  8 ).toFloat() );//crystalGrowthConst
    query.bindValue(  9, defaultQuery.value(  9 ).toFloat() );//silverSaltConsumptionConst
    query.bindValue( 10, defaultQuery.value( 10 ).toFloat() );//totalDevelopmentTime
    query.bindValue( 11, defaultQuery.value( 11 ).toInt()   );//agitateCount
    query.bindValue( 12, defaultQuery.value( 12 ).toInt()   );//developmentResolution
    query.bindValue( 13, defaultQuery.value( 13 ).toFloat() );//filmArea
    query.bindValue( 14, defaultQuery.value( 14 ).toFloat() );//sigmaConst
    query.bindValue( 15, defaultQuery.value( 15 ).toFloat() );//layerMixConst
    query.bindValue( 16, defaultQuery.value( 16 ).toFloat() );//layerTimeDivisor
    query.bindValue( 17, defaultQuery.value( 17 ).toInt()   );//rolloffBoundary
    query.bindValue( 18, defaultQuery.value( 18 ).toFloat() );//exposureComp
    query.bindValue( 19, defaultQuery.value( 19 ).toFloat() );//whitepoint
    query.bindValue( 20, defaultQuery.value( 20 ).toFloat() );//blackpoint
    query.bindValue( 21, defaultQuery.value( 21 ).toFloat() );//shadowsX
    query.bindValue( 22, defaultQuery.value( 22 ).toFloat() );//shadowsY
    query.bindValue( 23, defaultQuery.value( 23 ).toFloat() );//highlightsX
    query.bindValue( 24, defaultQuery.value( 24 ).toFloat() );//highlightsY
    query.bindValue( 25, defaultQuery.value( 25 ).toInt()   );//highlightRecovery
    query.bindValue( 26, defaultQuery.value( 26 ).toInt()   );//caEnabled
    double temp, tint;
    optimizeWBMults( absoluteFilePath.toStdString(), temp, tint );
    cout << "organizeinsertion: temp: " << temp << endl;
    cout << "organizeinsertion: tint: " << tint << endl;
    query.bindValue( 27, defaultQuery.value( 27 ).toFloat() );//temperature
    query.bindValue( 28, defaultQuery.value( 28 ).toFloat() );//tint
    query.bindValue( 29, defaultQuery.value( 29 ).toFloat() );//vibrance
    query.bindValue( 30, defaultQuery.value( 30 ).toFloat() );//saturation
    //Orientation
    query.bindValue( 31, exifDefaultRotation( exifData ) );

    bool success = query.exec();
    if ( !success )
    {
        qDebug() << query.lastError();
    }


    //Now we generate a thumbnail.
    //TODO
}
