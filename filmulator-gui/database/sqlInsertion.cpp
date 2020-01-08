#include "sqlInsertion.h"
#include "../core/filmSim.hpp"
#include "exifFunctions.h"
#include "../ui/parameterManager.h"
#include "../ui/thumbWriteWorker.h"
#include "../database/database.hpp"
#include <iostream>
using std::cout;
using std::endl;

/*This function inserts info on a raw file into the database.*/
void fileInsert(const QString hash,
                const QString filePathName,
                Exiv2::ExifData exifData)
{
    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    QSqlQuery query(db);

    //TODO: check if exists, if so only update filePathName
    query.prepare("SELECT FTfilePath FROM FileTable WHERE (FTfileID = ?);");
    query.bindValue(0,hash);
    query.exec();
    const bool inDatabaseAlready = query.next();

    if (inDatabaseAlready)
    {
        query.prepare("UPDATE FileTable "
                      "SET FTfilePath = ? "
                      "WHERE (FTfileID = ?);");
        query.bindValue(0, filePathName);
        query.bindValue(1, hash);
        query.exec();
    }
    else
    {
        query.prepare("INSERT INTO FileTable values (?,?,?,?,?,?,?,?,?);");
                                                   //0 1 2 3 4 5 6 7 8
        //Hash of the file:
        query.bindValue(0, hash);
        //Full path to the new location of the file:
        query.bindValue(1, filePathName);
        //Camera manufacturer
        query.bindValue(2, exifMake(exifData));
        //Camera model
        query.bindValue(3, exifModel(exifData));
        //ISO sensitivity
        query.bindValue(4, exifIso(exifData));
        //Exposure time
        query.bindValue(5, exifTv(exifData));
        //Aperture number
        query.bindValue(6, exifAv(exifData));
        //Focal length
        query.bindValue(7, exifFl(exifData));
        //Initialize a counter at 0 for number of times it has been referenced.
        // We only do this for new imports into the database.
        query.bindValue(8, 0);
        query.exec();
    }
}

/*This function creates a default profile in the profile table, and a search entry in the searchtable.*/
/*It returns a QString containing the STsearchID.*/
QString createNewProfile(const QString fileHash,
                         const QString fileName,
                         const QDateTime captureTime,
                         const QDateTime importStartTime,
                         Exiv2::ExifData exifData,
                         Exiv2::XmpData xmpData)
{
    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    QSqlQuery query(db);
    //Retrieve the usage count from the file table, and increment it by one.
    query.prepare("SELECT FTusageIncrement FROM FileTable WHERE (FTfileID = ?);");
    query.bindValue(0, fileHash);
    query.exec();
    const bool success = query.next();
    int increment = 0;
    if (success)
    {
        increment = query.value(0).toInt();
    }
    increment++;
    query.prepare("UPDATE FileTable SET FTusageIncrement = ? WHERE FTfileID = ?;");
    query.bindValue(0, increment);
    query.bindValue(1, fileHash);
    query.exec();

    //Create a new search table entry
    query.prepare("INSERT INTO SearchTable ("
                  "STsearchID, "
                  "STcaptureTime, "
                  "STname, "
                  "STfilename, "
                  "STsourceHash, "
                  "STrating, "
                  "STlatitude, "
                  "STlongitude, "
                  "STimportTime, "
                  "STlastProcessedTime, "
                  "STimportStartTime, "
                  "STthumbWritten, "
                  "STbigThumbWritten) "
                  "values (?,?,?,?,?,?,?,?,?,?,?,?,?);");
                         //0 1 2 3 4 5 6 7 8 9 101112

    //searchID (filehash with the increment appended)
    QString searchID = fileHash;
    searchID.append(QString("%1").arg(increment, 4, 10, QLatin1Char('0')));
    query.bindValue(0, searchID);
    //captureTime (unix time)
    query.bindValue(1, captureTime.toTime_t());
    //name (of instance)
    query.bindValue(2, "");
    //filename
    query.bindValue(3, fileName);
    //sourceHash
    query.bindValue(4, fileHash);
    //rating
    //TODO: write function to get rating
    query.bindValue(5, exifRating(exifData, xmpData));

    //latitude
    //TODO: figure something out here to either grab from the exif or get user input.
    query.bindValue(6, 0);
    //longitude
    query.bindValue(7, 0);
    QDateTime now = QDateTime::currentDateTime();
    //importTime (unix time)
    query.bindValue(8, now.toTime_t());
    //lastProcessedTime (unix time)
    //It's the same as above, since we're making a new one.
    query.bindValue(9, now.toTime_t());
    //importStartTime (unix time): lets us group together import batches.
    query.bindValue(10, importStartTime.toTime_t());
    //thumbWritten
    query.bindValue(11, 0);
    //bigThumbWritten (the preview)
    query.bindValue(12, 0);

    query.exec();


    //Now we make a new profile.

    //Create a parameter manager. It loads the default defaults automatically.
    //Tell it what the image ID is. It grabs the exif rotation and WB and revises the defaults.
    //Since the image ID doesn't exist in the SearchTable (but does in the FileTable, it
    // automatically makes a new profile and writes it to the database.
    //Now we generate a thumbnail.

    ParameterManager paramManager;
    paramManager.selectImage(searchID);

    //Next, we prepare a dummy exif object because we don't care about the thumbnail's exif.
    Exiv2::ExifData exif;

    //Next, we create a dummy interface.
    Interface dummyInterface;

    //Create a pipeline of the appropriate type.
    ImagePipeline pipeline(NoCache, NoHisto, LowQuality);

    //Process an image.
    matrix<unsigned short> image = pipeline.processImage(&paramManager, &dummyInterface, exif);

    //Write the thumbnail.
    ThumbWriteWorker worker;
    worker.setImage(image, exif);
    bool writeError = worker.writeThumb(searchID);
    //imwrite_jpeg(image, outputFilename.toStdString(), exif, 90);

    //Because it might take some time to prepare the thumbnail,
    // set the import time to be equal to the last processed time.
    query.prepare("update SearchTable set STimportTime = STlastProcessedTime WHERE STsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();

    //Return STsearchID, only if the thumb was written successfully.
    if(!writeError)
    {
        return searchID;
    }
    else
    {
        return QString("");
    }
}
