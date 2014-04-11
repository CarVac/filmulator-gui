#include "filmulatordb.h"
#include <QString>
#include <QVariant>

void setupDB( QSqlDatabase *db )
{
    QDir dir = QDir::home();
    if ( !dir.cd( ".filmulator" ) )
    {
        dir.mkdir( ".filmulator" );
        dir.cd( ".filmulator" );
    }
    db -> setDatabaseName( dir.absoluteFilePath( "filmulatorDB" ) );
//    std::cout << "database path: " << qPrintable(dir.absoluteFilePath("filmulatorDB")) << std::endl;
    //this should create the database if it doesn't already exist.

    if ( db -> open() )
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
    query.exec( "create table if not exists SearchTable ("
                "searchID varchar primary key,"
                "captureTime integer,"//unix time
                "name varchar,"//name of this instance
                "filename varchar,"//name of source file
                "sourceHash varchar( 32 ),"//primary key in the file table
                "rating integer,"//for easy culling
                "latitude real,"
                "longitude real,"
                "importTime integer,"//unix time
                "lastProcessedTime integer"//unix time
                ");"
                );
    query.exec( "create index if not exists TimeIndex"
                " on SearchTable ( captureTime );"
                );

    //2. The table which carries more file info, like exifs and thumbnail locations.
    query.exec( "create table if not exists FileTable ("
                "fileID varchar( 32 ) primary key," //md5sum of the file
                "filePath varchar,"
                "cameraMake varchar,"
                "cameraModel varchar,"
                "sensitivity integer,"
                "exposureTime varchar,"
                "aperture real,"
                "focalLength real,"
                "usageIncrement integer"
                ");"
                );

    //3. The table which holds processing parameters.
    //     This table should have the same number of entries as the SearchTable.
    query.exec( "create table if not exists ProcessingTable ("
                "procID varchar primary key,"           //0
                "initialDeveloperConcentration real,"   //1
                "reservoirThickness real,"              //2
                "activeLayerThickness real,"            //3
                "crystalsPerPixel real,"                //4
                "initialCrystalRadius real,"            //5
                "initialSilverSaltDensity real,"        //6
                "developerConsumptionConst real,"       //7
                "crystalGrowthConst real,"              //8
                "silverSaltConsumptionConst real,"      //9
                "totalDevelopmentTime real,"            //10
                "agitateCount integer,"                 //11
                "developmentResolution integer,"        //12
                "filmArea real,"                        //13
                "sigmaConst real,"                      //14
                "layerMixConst real,"                   //15
                "layerTimeDivisor real,"                //16
                "rolloffBoundary integer,"              //17
                "exposureComp real,"                    //18
                "whitepoint real,"                      //19
                "blackpoint real,"                      //20
                "shadowsX real,"                        //21
                "shadowsY real,"                        //22
                "highlightsX real,"                     //23
                "highlightsY real,"                     //24
                "highlightRecovery integer,"            //25
                "caEnabled integer,"                    //26
                "temperature real,"                     //27
                "tint real,"                            //28
                "vibrance real,"                        //29
                "saturation real,"                      //30
                "orientation integer"                   //31
                ");"
                );

    //Next, we set up a table for default processing parameters.
    //This will be of the same structure as ProcessingTable except for orientation.
    query.exec( "create table if not exists ProfileTable ("
                "profileId varchar primary key,"        //0
                "initialDeveloperConcentration real,"   //1
                "reservoirThickness real,"              //2
                "activeLayerThickness real,"            //3
                "crystalsPerPixel real,"                //4
                "initialCrystalRadius real,"            //5
                "initialSilverSaltDensity real,"        //6
                "developerConsumptionConst real,"       //7
                "crystalGrowthConst real,"              //8
                "silverSaltConsumptionConst real,"      //9
                "totalDevelopmentTime real,"            //10
                "agitateCount integer,"                 //11
                "developmentResolution integer,"        //12
                "filmArea real,"                        //13
                "sigmaConst real,"                      //14
                "layerMixConst real,"                   //15
                "layerTimeDivisor real,"                //16
                "rolloffBoundary integer,"              //17
                "exposureComp real,"                    //18
                "whitepoint real,"                      //19
                "blackpoint real,"                      //20
                "shadowsX real,"                        //21
                "shadowsY real,"                        //22
                "highlightsX real,"                     //23
                "highlightsY real,"                     //24
                "highlightRecovery integer,"            //25
                "caEnabled integer,"                    //26
                "temperature real,"                     //27
                "tint real,"                            //28
                "vibrance real,"                        //29
                "saturation real"                       //30
                ");"
                );

    //Now we set the default Default profile.
    query.prepare( "REPLACE INTO ProfileTable values "
                   "(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);" );
                   //0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
                   //                    1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3
    //Name of profile; must be unique.
    query.bindValue( 0, "Default" );
    //Initial Developer Concentration
    query.bindValue( 1, 1.0f ); //initialDeveloperConcentration
    //Thickness of developer reservoir
    query.bindValue( 2, 1000.0f ); //reservoirThickness
    //Thickness of active layer (not necessarily the same units as above. I think. Maybe. -CV)
    query.bindValue( 3, 0.1f ); //activeLayerThickness
    //Number of crystals per pixel that might become activated
    query.bindValue( 4, 500.0f ); //crystalsPerPixel
    //Initial radius for just-activated silver crystals
    query.bindValue( 5, 0.00001f ); //initialCrystalRadius
    //Initial surface density of silver salt crystals
    query.bindValue( 6, 1.0f ); //initalSilverSaltDensity
    //Rate constant for consumption of developer
    query.bindValue( 7, 2000000.0f ); //developerConsumptionConst
    //Proportionality constant for growth of silver crystals relative to consumption of developer
    query.bindValue( 8, 0.00001f ); //crystalGrowthConst
    //Rate constant for consumption of silver halides in the film layers
    query.bindValue( 9, 2000000.0f ); //silverSaltConsumptionConst
    //Total duration of the simulated development
    query.bindValue( 10, 100.0f ); //totalDevelopmentTime
    //Number of times the developer solution is mixed during the development process
    query.bindValue( 11, 1 ); //agitateCount
    //Number of simulation steps for the development process
    query.bindValue( 12, 12 ); //developmentResolution
    //Area of the simulated film
    query.bindValue( 13, 864.0f ); //filmArea
    //Constant that influences the extent of the diffusion; yes this is redundant.
    query.bindValue( 14, 0.2f ); //sigmaConst
    //Constant that affects the amount of layer-reservoir diffusion
    query.bindValue( 15, 0.2f ); //layerMixConst
    //Constant that affects the ratio of intra-layer and layer-reservoir diffusion; yes this is redundant.
    query.bindValue( 16, 20.0f ); //layerTimeDivisor
    //Constant that affects where the rolloff starts in exposure
    query.bindValue( 17, 51275 ); //rolloffBoundary
    //Exposure compensation
    query.bindValue( 18, 0.0f ); //exposureComp
    //White clipping point
    query.bindValue( 19, 0.0015f ); //whitepoint
    //Black clipping point
    query.bindValue( 20, 0.0f ); //blackpoint
    //Input value defining the general shadow region control point
    query.bindValue( 21, 0.25f ); //shadowsX
    //Output value defining the general shadow region control point
    query.bindValue( 22, 0.25f ); //shadowsY
    //Input value defining the general highlight region control point
    query.bindValue( 23, 0.75f ); //highlightsX
    //Output value defining the general highlight region control point
    query.bindValue( 24, 0.75f ); //highlightsY
    //dcraw highlight recovery parameter
    query.bindValue( 25, 0 ); //highlightRecovery
    //Automatic CA correct switch
    query.bindValue( 26, 0 ); //caEnabled
    //Color temperature WB adjustment
    query.bindValue( 27, 5700.0f ); //temperature
    //Magenta/green tint WB adjustment
    query.bindValue( 28, 0.0f ); //tint
    //Saturation of less-saturated stuff
    query.bindValue( 29, 0.0f ); //vibrance
    //Saturation of whole image
    query.bindValue( 30, 0.0f ); //saturation

    //Well, orientation obviously doesn't get a preset.
    query.exec();

}
