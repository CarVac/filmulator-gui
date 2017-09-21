#include "filmulatorDB.h"
#include <QString>
#include <QVariant>
#include <iostream>

DBSuccess setupDB(QSqlDatabase *db)
{
    QDir dir = QDir::home();
    if (!dir.cd(".local/share/filmulator"))
    {
        if (!dir.mkpath(".local/share/filmulator"))
        {
            std::cout << "Could not create database directory" << std::endl;
            return DBSuccess::failure;
        }
        else
        {
            dir.cd(".local/share/filmulator");
        }
    }
    db -> setDatabaseName(dir.absoluteFilePath("filmulatorDB"));
    //this should create the database if it doesn't already exist.

    if (db -> open())
    {
//        std::cout << "Database open!" << std::endl;
        //Success
    }
    else
    {
        std::cout << "what?!?!?!?" << std::endl;
        return DBSuccess::failure;
    }

    QSqlQuery query;
    //query.exec("PRAGMA synchronous = OFF");//Use for speed, but dangerous.
    //query.exec("PRAGMA synchronous = NORMAL");//Use for less speed, slightly dangerous.

    //We need to set up 3 tables for the processing.
    //1. The master table for searching. This should be small
    //  for speed. It points at the other two.
    query.exec("create table if not exists SearchTable ("
               "STsearchID varchar primary key,"
               "STcaptureTime integer,"//unix time
               "STname varchar,"//name of this instance
               "STfilename varchar,"//name of source file
               "STsourceHash varchar(32),"//primary key in the file table
               "STrating integer,"//for easy culling
               "STlatitude real,"
               "STlongitude real,"
               "STimportTime integer,"//unix time
               "STlastProcessedTime integer,"//unix time
               "STimportStartTime integer,"//unix time
               "STthumbWritten integer,"//-1 for error, 0 for not written or invalidated, 1 for written
               "STbigThumbWritten integer"//same as above
               ");"
               );
    query.exec("create index if not exists TimeIndex"
               " on SearchTable (STcaptureTime);"
               );

    //2. The table which carries more file info, like exifs and thumbnail locations.
    query.exec("create table if not exists FileTable ("
                "FTfileID varchar(32) primary key," //md5sum of the file
                "FTfilePath varchar,"
                "FTcameraMake varchar,"
                "FTcameraModel varchar,"
                "FTsensitivity integer,"
                "FTexposureTime varchar,"
                "FTaperture real,"
                "FTfocalLength real,"
                "FTusageIncrement integer"
                ");"
               );

    //3. The table which holds processing parameters.
    //     This table should have the same number of entries as the SearchTable.
    query.exec("create table if not exists ProcessingTable ("
               "ProcTprocID varchar primary key,"           //0
               "ProcTinitialDeveloperConcentration real,"   //1
               "ProcTreservoirThickness real,"              //2
               "ProcTactiveLayerThickness real,"            //3
               "ProcTcrystalsPerPixel real,"                //4
               "ProcTinitialCrystalRadius real,"            //5
               "ProcTinitialSilverSaltDensity real,"        //6
               "ProcTdeveloperConsumptionConst real,"       //7
               "ProcTcrystalGrowthConst real,"              //8
               "ProcTsilverSaltConsumptionConst real,"      //9
               "ProcTtotalDevelopmentTime real,"            //10
               "ProcTagitateCount integer,"                 //11
               "ProcTdevelopmentSteps integer,"             //12
               "ProcTfilmArea real,"                        //13
               "ProcTsigmaConst real,"                      //14
               "ProcTlayerMixConst real,"                   //15
               "ProcTlayerTimeDivisor real,"                //16
               "ProcTrolloffBoundary integer,"              //17
               "ProcTexposureComp real,"                    //18
               "ProcTwhitepoint real,"                      //19
               "ProcTblackpoint real,"                      //20
               "ProcTshadowsX real,"                        //21
               "ProcTshadowsY real,"                        //22
               "ProcThighlightsX real,"                     //23
               "ProcThighlightsY real,"                     //24
               "ProcThighlightRecovery integer,"            //25
               "ProcTcaEnabled integer,"                    //26
               "ProcTtemperature real,"                     //27
               "ProcTtint real,"                            //28
               "ProcTvibrance real,"                        //29
               "ProcTsaturation real,"                      //30
               "ProcTrotation integer,"                     //31
               "ProcTcropHeight real,"                      //32
               "ProcTcropAspect real,"                      //33
               "ProcTcropVoffset real,"                     //34
               "ProcTcropHoffset real"                      //35
               ");"
               );

    //Next, we set up a table for default processing parameters.
    //This will be of the same structure as ProcessingTable except for orientation and crop.
    query.exec("create table if not exists ProfileTable ("
               "ProfTprofileId varchar primary key,"        //0
               "ProfTinitialDeveloperConcentration real,"   //1
               "ProfTreservoirThickness real,"              //2
               "ProfTactiveLayerThickness real,"            //3
               "ProfTcrystalsPerPixel real,"                //4
               "ProfTinitialCrystalRadius real,"            //5
               "ProfTinitialSilverSaltDensity real,"        //6
               "ProfTdeveloperConsumptionConst real,"       //7
               "ProfTcrystalGrowthConst real,"              //8
               "ProfTsilverSaltConsumptionConst real,"      //9
               "ProfTtotalDevelopmentTime real,"            //10
               "ProfTagitateCount integer,"                 //11
               "ProfTdevelopmentSteps integer,"             //12
               "ProfTfilmArea real,"                        //13
               "ProfTsigmaConst real,"                      //14
               "ProfTlayerMixConst real,"                   //15
               "ProfTlayerTimeDivisor real,"                //16
               "ProfTrolloffBoundary integer,"              //17
               "ProfTexposureComp real,"                    //18
               "ProfTwhitepoint real,"                      //19
               "ProfTblackpoint real,"                      //20
               "ProfTshadowsX real,"                        //21
               "ProfTshadowsY real,"                        //22
               "ProfThighlightsX real,"                     //23
               "ProfThighlightsY real,"                     //24
               "ProfThighlightRecovery integer,"            //25
               "ProfTcaEnabled integer,"                    //26
               "ProfTtemperature real,"                     //27
               "ProfTtint real,"                            //28
               "ProfTvibrance real,"                        //29
               "ProfTsaturation real"                       //30
               ");"
               );

    //Next, we make a table for the queue.
    //It will hold a number for the order, and a field identical to
    // STsearchID.
    query.exec("create table if not exists QueueTable ("
               "QTindex integer,"
               "QTprocessed bool,"
               "QTexported bool,"
               "QToutput bool,"
               "QTsearchID varchar unique"
               ");"
               );

    //Now we set the default Default profile.
    query.prepare("REPLACE INTO ProfileTable values "
                  "(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
                  //0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
                  //                    1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3
    //Name of profile; must be unique.
    query.bindValue(0, "Default");
    //Initial Developer Concentration
    query.bindValue(1, 1.0f); //initialDeveloperConcentration
    //Thickness of developer reservoir
    query.bindValue(2, 1000.0f); //reservoirThickness
    //Thickness of active layer (not necessarily the same units as above. I think. Maybe. -CV)
    query.bindValue(3, 0.1f); //activeLayerThickness
    //Number of crystals per pixel that might become activated
    query.bindValue(4, 500.0f); //crystalsPerPixel
    //Initial radius for just-activated silver crystals
    query.bindValue(5, 0.00001f); //initialCrystalRadius
    //Initial surface density of silver salt crystals
    query.bindValue(6, 1.0f); //initalSilverSaltDensity
    //Rate constant for consumption of developer
    query.bindValue(7, 2000000.0f); //developerConsumptionConst
    //Proportionality constant for growth of silver crystals relative to consumption of developer
    query.bindValue(8, 0.00001f); //crystalGrowthConst
    //Rate constant for consumption of silver halides in the film layers
    query.bindValue(9, 2000000.0f); //silverSaltConsumptionConst
    //Total duration of the simulated development
    query.bindValue(10, 100.0f); //totalDevelopmentTime
    //Number of times the developer solution is mixed during the development process
    query.bindValue(11, 1); //agitateCount
    //Number of simulation steps for the development process
    query.bindValue(12, 12); //developmentSteps
    //Area of the simulated film
    query.bindValue(13, 864.0f); //filmArea
    //Constant that influences the extent of the diffusion; yes this is redundant.
    query.bindValue(14, 0.2f); //sigmaConst
    //Constant that affects the amount of layer-reservoir diffusion
    query.bindValue(15, 0.2f); //layerMixConst
    //Constant that affects the ratio of intra-layer and layer-reservoir diffusion; yes this is redundant.
    query.bindValue(16, 20.0f); //layerTimeDivisor
    //Constant that affects where the rolloff starts in exposure
    query.bindValue(17, 51275); //rolloffBoundary
    //Exposure compensation
    query.bindValue(18, 0.0f); //exposureComp
    //White clipping point
    query.bindValue(19, 0.002f); //whitepoint
    //Black clipping point
    query.bindValue(20, 0.0f); //blackpoint
    //Input value defining the general shadow region control point
    query.bindValue(21, 0.25f); //shadowsX
    //Output value defining the general shadow region control point
    query.bindValue(22, 0.25f); //shadowsY
    //Input value defining the general highlight region control point
    query.bindValue(23, 0.75f); //highlightsX
    //Output value defining the general highlight region control point
    query.bindValue(24, 0.75f); //highlightsY
    //dcraw highlight recovery parameter
    query.bindValue(25, 0); //highlightRecovery
    //Automatic CA correct switch
    query.bindValue(26, 0); //caEnabled
    //Color temperature WB adjustment
    query.bindValue(27, 5200.0f); //temperature
    //Magenta/green tint WB adjustment
    query.bindValue(28, 1.0f); //tint
    //Saturation of less-saturated stuff
    query.bindValue(29, 0.0f); //vibrance
    //Saturation of whole image
    query.bindValue(30, 0.0f); //saturation

    //Well, orientation obviously doesn't get a preset.
    query.exec();


    //Check the database version.
    query.exec("PRAGMA user_version;");
    query.next();
    const int oldVersion = query.value(0).toInt();
    std::cout << "dbSetup old version: " << oldVersion << std::endl;
    QString versionString = ";";

    query.exec("BEGIN TRANSACTION;");//begin a transaction
    switch (oldVersion) {
    case 0:
        //Generate a list of 100000 integers for useful purposes
        /*
        query.exec("CREATE TABLE integers (i integer);");
        query.exec("INSERT INTO integers (i) VALUES (0);");
        query.exec("INSERT INTO integers (i) VALUES (1);");
        query.exec("INSERT INTO integers (i) VALUES (2);");
        query.exec("INSERT INTO integers (i) VALUES (3);");
        query.exec("INSERT INTO integers (i) VALUES (4);");
        query.exec("INSERT INTO integers (i) VALUES (5);");
        query.exec("INSERT INTO integers (i) VALUES (6);");
        query.exec("INSERT INTO integers (i) VALUES (7);");
        query.exec("INSERT INTO integers (i) VALUES (8);");
        query.exec("INSERT INTO integers (i) VALUES (9);");
        query.exec("CREATE VIEW integers9 as "
                   "SELECT 100000*a.i+10000*b.i+1000*c.i+100*d.i+10*e.i+f.i as ints "
                   "FROM integers a "
                   "CROSS JOIN integers b "
                   "CROSS JOIN integers c "
                   "CROSS JOIN integers d "
                   "CROSS JOIN integers e "
                   "CROSS JOIN integers f;");
                   */
        versionString = "PRAGMA user_version = 1;";
    case 1:
        /*
        query.exec("CREATE VIEW integers5 as "
                   "SELECT 10000*a.i+1000*b.i+100*c.i+10*d.i+e.i as ints "
                   "FROM integers a "
                   "CROSS JOIN integers b "
                   "CROSS JOIN integers c "
                   "CROSS JOIN integers d "
                   "CROSS JOIN integers e;");
        query.exec("CREATE VIEW integers4 as "
                   "SELECT 1000*a.i+100*b.i+10*c.i+d.i as ints "
                   "FROM integers a "
                   "CROSS JOIN integers b "
                   "CROSS JOIN integers c "
                   "CROSS JOIN integers d;");
                   */
        versionString = "PRAGMA user_version = 2;";
    case 2:
        query.exec("DROP TABLE QueueTable;");
        query.exec("CREATE TABLE QueueTable ("
                   "QTindex integer,"
                   "QTprocessed bool,"
                   "QTexported bool,"
                   "QToutput bool,"
                   "QTsearchID varchar unique);");
        versionString = "PRAGMA user_version = 3;";
    case 3:
        query.exec("DROP VIEW integers9;");
        query.exec("DROP VIEW integers4;");
        query.exec("DROP VIEW integers3;");
        versionString = "PRAGMA user_version = 4;";
    case 4:
        query.exec("ALTER TABLE SearchTable "
                   "ADD COLUMN STimportStartTime integer;");
        query.exec("UPDATE SearchTable SET STimportStartTime = STimportTime;");
        versionString = "PRAGMA user_version = 5;";
    case 5:
        query.exec("UPDATE SearchTable SET STrating = min(5,max(0,STrating));");
        versionString = "PRAGMA user_version = 6;";
    case 6:
        query.exec("ALTER TABLE SearchTable "
                   "ADD COLUMN STthumbWritten bool;");
        query.exec("ALTER TABLE SearchTable "
                   "ADD COLUMN STbigThumbWritten bool;");
        query.exec("UPDATE SearchTable SET STthumbWritten = 1;");
        query.exec("UPDATE SearchTable SET STbigThumbWritten = 0;");
        versionString = "PRAGMA user_version = 7;";
    case 7:
        query.exec("ALTER TABLE QueueTable "
                   "ADD COLUMN QTsortedIndex;");
        query.exec("UPDATE QueueTable SET QTsortedIndex = QTindex;");
        versionString = "PRAGMA user_version = 8;";
    case 8:
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTcropHeight;");
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTcropAspect;");
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTcropVoffset;");
        query.exec("ALTER TABLE ProcessingTable ADD COLUMN ProcTcropHoffset;");
        query.exec("UPDATE ProcessingTable SET ProcTcropHeight = 0;");
        query.exec("UPDATE ProcessingTable SET ProcTcropAspect = 0;");
        query.exec("UPDATE ProcessingTable SET ProcTcropVoffset = 0;");
        query.exec("UPDATE ProcessingTable SET ProcTcropHoffset = 0;");
        versionString = "PRAGMA user_version = 9;";
    }
    query.exec(versionString);
    query.exec("COMMIT TRANSACTION;");//finalize the transaction only after writing the version.

    return DBSuccess::success;
}
