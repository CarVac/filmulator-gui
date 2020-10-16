#include "importWorker.h"
#include "../database/database.hpp"
#include <iostream>
#include "queueModel.h"
#include "QThread"
using std::cout;
using std::endl;

ImportWorker::ImportWorker(QObject *parent) : QObject(parent)
{
}

QString ImportWorker::importFile(const QFileInfo infoIn,
                              const int importTZ,
                              const int cameraTZ,
                              const QString photoDir,
                              const QString backupDir,
                              const QString dirConfig,
                              const QDateTime importStartTime,
                              const bool appendHash,
                              const bool importInPlace,
                              const bool replaceLocation,
                              const bool noThumbnail)
{
    //Generate a hash of the raw file.
    QCryptographicHash hash(QCryptographicHash::Md5);
    QFile file(infoIn.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug("File couldn't be opened.");
    }

    //Grab EXIF data from the file.
    const std::string abspath = infoIn.absoluteFilePath().toStdString();
    //We don't do this anymore because exiv2 doesn't support cr3 currently

    //Load data into the hash function.
    while (!file.atEnd())
    {
        hash.addData(file.read(8192));
    }
    QString hashString = QString(hash.result().toHex());

    QString filename = infoIn.fileName();
    //Optionally append 7 alphanumeric characters derived from the hash to the filename
    //We don't want it to do this if we're importing in place.
    if (!importInPlace && appendHash)
    {
        QString subFilename = filename.left(filename.length()-4);
        QString extension = filename.right(4);
        subFilename.append("_");
        int carry = 0;
        const char a[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        QByteArray hashArray = hash.result();

        for (int i = 0; i < 7; i++)
        {
            //Convert the byte to an integer.
            int value = carry + uint8_t(hashArray.at(i));
            //Carry it so that it affects the next one.
            carry = value / 62;
            int val = value % 62;
            subFilename.append(a[val]);
        }
        subFilename.append(extension);
        filename = subFilename;
    }

    //Set up the main directory to insert the file, and the full file path.
    //This is based on what time it was in the timezone of photo capture.
    QString outputPath = photoDir;
    outputPath.append(exifLocalDateString(abspath, cameraTZ, importTZ, dirConfig));
    QString outputPathName = outputPath;
    outputPathName.append(filename);
    //Create the directory.
    if (!importInPlace)
    {
        QDir dir(outputPath);
        dir.mkpath(outputPath);
    }

    //Sets up the backup directory.
    QString backupPath = backupDir;
    backupPath.append(exifLocalDateString(abspath, cameraTZ, importTZ, dirConfig));
    QString backupPathName = backupPath;
    backupPathName.append(filename);

    //Create the directory, if the root exists, and we're not importing in place.
    QDir backupRoot(backupDir);
    if (backupRoot.exists() && !importInPlace)
    {
        QDir backupDirectory(backupPath);
        backupDirectory.mkpath(backupPath);

        //We need to verify that this copy happens successfully
        //And that the file integrity was maintained.
        //I will have it retry up to five times upon failure;
        //In my own experience, this copy has failed
        // while the main copy that occurs later copies successfully...
        //Is this a caching thing? I dunno.
        bool success = false;
        int attempts = 0;
        if (QFile::exists(backupPathName)) //check the integrity of any file that already exists
        {
            QFile backupFile(backupPathName);
            QCryptographicHash backupHash(QCryptographicHash::Md5);
            if (!backupFile.open(QIODevice::ReadOnly))
            {
                qDebug("backup file existed but could not be opened.");
            } else {
                while (!backupFile.atEnd())
                {
                    backupHash.addData(backupFile.read(8192));
                }
            }
            QString backupHashString = QString(hash.result().toHex());
            if (backupHashString != hashString)
            {
                cout << "Backup hash check failed" << endl;
                cout << "Original hash: " << hashString.toStdString() << endl;
                cout << "Backup hash:   " << backupHashString.toStdString() << endl;
                success = false;
                backupFile.remove(backupPathName);
            } else {
                cout << "Backup hash verified" << endl;
                success = true;
            }
        }
        while (!success)
        {
            attempts += 1;
            success = QFile::copy(infoIn.absoluteFilePath(), backupPathName);
            QFile backupFile(backupPathName);
            QCryptographicHash backupHash(QCryptographicHash::Md5);
            if (!backupFile.open(QIODevice::ReadOnly))
            {
                qDebug("backup file could not be opened.");
            } else {
                while (!backupFile.atEnd())
                {
                    backupHash.addData(backupFile.read(8192));
                }
            }
            QString backupHashString = QString(hash.result().toHex());
            if (backupHashString != hashString)
            {
                cout << "Backup attempt number " << attempts << " hash failed" << endl;
                cout << "Original hash: " << hashString.toStdString() << endl;
                cout << "Backup hash:   " << backupHashString.toStdString() << endl;
                success = false;
                backupFile.remove(backupPathName);
            }
            if (attempts > 6)
            {
                cout << "Giving up on backup." << endl;
                success = true;
            }
        }
    }

    //Check to see if it's already present in the database.
    //Open a new database connection for the thread
    QSqlDatabase db = getDB();
    QSqlQuery query(db);
    query.prepare("SELECT FTfilepath FROM FileTable WHERE (FTfileID = ?);");
    query.bindValue(0, hashString);
    query.exec();
    const bool inDatabaseAlready = query.next();
    QString dbRecordedPath;
    if (inDatabaseAlready)
    {
        dbRecordedPath = query.value(0).toString();
    }
    db.close();
    //If it's not in the database yet,
    //And we're not updating locations
    //  (if we are updating locations, we don't want it to add new things to the db)
    bool changedST = false;
    QString STsearchID;
    if (!inDatabaseAlready && !replaceLocation)
    {
        //Record the file location in the database.
        if (!importInPlace)
        {
            //Copy the file into our main directory.
            //We need to verify that this copy happens successfully
            //And that the file integrity was maintained.
            //I will have it retry up to five times upon failure;
            //In my own experience, the main copy has succeeded
            // while the earlier backup failed...
            //Is this a caching thing? I dunno.
            bool success = false;
            int attempts = 0;
            while (!success)
            {
                attempts += 1;
                success = QFile::copy(infoIn.absoluteFilePath(), outputPathName);
                QFile outputFile(outputPathName);
                QCryptographicHash outputHash(QCryptographicHash::Md5);
                if (!outputFile.open(QIODevice::ReadOnly))
                {
                    qDebug("output file could not be opened.");
                } else {
                    while (!outputFile.atEnd())
                    {
                        outputHash.addData(outputFile.read(8192));
                    }
                }
                QString outputHashString = QString(hash.result().toHex());
                if (outputHashString != hashString)
                {
                    cout << "output attempt number " << attempts << " hash failed" << endl;
                    cout << "Original hash: " << hashString.toStdString() << endl;
                    cout << "Output hash:   " << outputHashString.toStdString() << endl;
                    success = false;
                    outputFile.remove(outputPathName);
                } else {
                    //success
                    fileInsert(hashString, outputPathName);
                }
                if (attempts > 6)
                {
                    cout << "Giving up on output." << endl;
                    success = true;
                }
            }
        }
        else
        {
            //If it's being imported in place, then we don't copy the file.
            fileInsert(hashString, infoIn.absoluteFilePath());
        }

        //Now create a profile and a search table entry, and a thumbnail.
        STsearchID = createNewProfile(hashString,
                                      filename,
                                      exifUtcTime(abspath, cameraTZ),
                                      importStartTime,
                                      abspath,
                                      noThumbnail);

        //Request that we enqueue the image.
        cout << "importFile SearchID: " << STsearchID.toStdString() << endl;
        if (QString("") != STsearchID)
        {
            emit enqueueThis(STsearchID);
        }
        //It might be ignored downstream, but that's not our problem here.

        //Tell the views we need updating.
        changedST = true;
    }
    else if (inDatabaseAlready)//it's already in the database, so just move the file.
    {
        //See if the file is in its old location, and copy if not.
        //DON'T do this if we're updating the location.
        if (!QFile::exists(dbRecordedPath) && !importInPlace && !replaceLocation)
        {
            //Copy the file into our main directory.
            //We need to verify that this copy happens successfully
            //And that the file integrity was maintained.
            //I will have it retry up to five times upon failure;
            //In my own experience, the main copy has succeeded
            // while the earlier backup failed...
            //Is this a caching thing? I dunno.
            bool success = false;
            int attempts = 0;
            while (!success)
            {
                attempts += 1;
                success = QFile::copy(infoIn.absoluteFilePath(), outputPathName);
                QFile outputFile(outputPathName);
                QCryptographicHash outputHash(QCryptographicHash::Md5);
                if (!outputFile.open(QIODevice::ReadOnly))
                {
                    qDebug("output file could not be opened.");
                } else {
                    while (!outputFile.atEnd())
                    {
                        outputHash.addData(outputFile.read(8192));
                    }
                }
                QString outputHashString = QString(hash.result().toHex());
                if (outputHashString != hashString)
                {
                    cout << "output attempt number " << attempts << " hash failed" << endl;
                    cout << "Original hash: " << hashString.toStdString() << endl;
                    cout << "Output hash:   " << outputHashString.toStdString() << endl;
                    success = false;
                    outputFile.remove(outputPathName);
                } else {
                    //success
                    fileInsert(hashString, outputPathName);
                }
                if (attempts > 6)
                {
                    cout << "Giving up on output." << endl;
                    success = true;
                }
            }
        }

        //If we want to update the location of the file.
        if (replaceLocation)
        {
            fileInsert(hashString, infoIn.absoluteFilePath());
            cout << "importWorker replace location: " << infoIn.absoluteFilePath().toStdString() << endl;

            STsearchID = hashString.append(QString("%1").arg(1, 4, 10, QLatin1Char('0')));
            cout << "importWorker replace STsearchID: " << STsearchID.toStdString() << endl;

            if (QString("") != STsearchID)
            {
                emit enqueueThis(STsearchID);
            }
        }
    } else { //it's not in the database but we are hoping to replace the location.
        //We only do this for CLI-based processing.
        if (noThumbnail)
        {
            fileInsert(hashString, infoIn.absoluteFilePath());
            cout << "importWorker replace location: " << infoIn.absoluteFilePath().toStdString() << endl;

            //Now create a profile and a search table entry, and a thumbnail.
            STsearchID = createNewProfile(hashString,
                                          filename,
                                          exifUtcTime(abspath, cameraTZ),
                                          importStartTime,
                                          abspath,
                                          noThumbnail);

            //Request that we enqueue the image.
            cout << "importFile SearchID: " << STsearchID.toStdString() << endl;
            if (QString("") != STsearchID)
            {
                emit enqueueThis(STsearchID);
            }
            //It might be ignored downstream, but that's not our problem here.

            //Tell the views we need updating.
            changedST = true;
        }
    }

    //Tell the ImportModel whether we did anything to the SearchTable
    emit doneProcessing(changedST);
    return STsearchID;
}
