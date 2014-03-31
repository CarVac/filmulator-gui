//#include <iostream>
#include "filmulatordb.h"

void setupDB(QSqlDatabase *db)
{
    QDir dir = QDir::home();
    if (!dir.cd(".filmulator"))
    {
        dir.mkdir(".filmulator");
        dir.cd(".filmulator");
    }
    db->setDatabaseName(dir.absoluteFilePath("filmulatorDB"));
//    std::cout << "database path: " << qPrintable(dir.absoluteFilePath("filmulatorDB")) << std::endl;
    //this should create the database if it doesn't already exist.

    if(db->open())
    {
//        std::cout << "Database open!" << std::endl;
        //Success
    }
    else
    {
//        std::cout << "what?!?!?!?" << std::endl;
    }

    //We need to set up 3 tables for the processing.
    //1. The master table for searching. This should be small
    //  for speed. It points at the other two.
    QSqlQuery query;
    query.exec("create table if not exists SearchTable ("
               "searchID varchar(32) primary key,"
               "captureTime integer,"//unix time
               "name varchar,"//name of this instance
               "filename varchar,"//name of source file without extension or period
               "extension varchar,"//extension of source file without period
               "sourceHash varchar(32),"//primary key in the file table
               "rating integer,"//for easy culling
               "latitude real,"
               "longitude real,"
               "importTime integer,"//unix time
               "lastProcessedTime integer"//unix time
               ");"
               );

    //2. The table which carries more file info, like exifs and thumbnail locations.
    query.exec("create table if not exists FileTable ("
               "fileID varchar(32) primary key," //md5sum of the file
               "filePath varchar,"
               "cameraMake varchar,"
               "cameraModel varchar,"
               "sensitivity integer,"
               "exposureTime varchar,"
               "aperture real,"
               "focalLength real"
               ");"
               );

    //3. The table which holds processing parameters.
    //     This table should have the same number of entries as the SearchTable.
    query.exec("create table if not exists ProcessingTable ("
               "procID varchar(32) primary key,"//should be identical to SearchTable
               "initialDeveloperConcentration real,"
               "reservoirThickness real,"
               "activeLayerThickness real,"
               "crystalsPerPixel real,"
               "initialCrystalRadius real,"
               "initialSilverSaltDensity real,"
               "developerConsumptionConst real,"
               "crystalGrowthConst real,"
               "silverSaltConsumptionConst real"
               "totalDevelopmentTime real,"
               "agitateCount integer,"
               "developmentResolution integer,"
               "filmArea real,"
               "sigmaConst real,"
               "layerMixConst real,"
               "layerTimeDivisor real,"
               "rolloffBoundary integer,"
               "exposureComp real,"
               "whitepoint real,"
               "blackpoint real,"
               "shadowsY real,"
               "highlightsY real,"
               "highlightRecovery integer,"
               "caEnabled integer,"
               "temperature real,"
               "tint real,"
               "vibrance real,"
               "saturation real,"
               "orientation integer"
               ");"
               );

    //Next, we set up a table for default processing parameters.
    //This will be of the same structure as ProcessingTable.
    query.exec("create table if not exists ProfileTable ("
               "profileId varchar primary key,"
               "initialDeveloperConcentration real,"
               "reservoirThickness real,"
               "activeLayerThickness real,"
               "crystalsPerPixel real,"
               "silverSaltConsumptionConst,"
               "totalDevelopmentTime real,"
               "agitateCount integer,"
               "developmentResolution integer,"
               "filmArea real,"
               "sigmaConst real,"
               "layerMixConst real,"
               "layerTimeDivisor real,"
               "rolloffBoundary integer,"
               "exposureComp real,"
               "whitepoint real,"
               "blackpoint real,"
               "shadowsY real,"
               "highlightsY real,"
               "highlightRecovery integer,"
               "caEnabled integer,"
               "temperature real,"
               "tint real,"
               "vibrance real,"
               "saturation real,"
               "orientation integer"
               ");"
               );
}
