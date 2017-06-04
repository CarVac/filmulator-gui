#include "importWorker.h"
#include <iostream>
#include "queueModel.h"
using std::cout;
using std::endl;

ImportWorker::ImportWorker(QObject *parent) : QObject(parent)
{
}

void ImportWorker::importFile(const QFileInfo infoIn,
                              const int importTZ,
                              const int cameraTZ,
                              const QString photoDir,
                              const QString backupDir,
                              const QString dirConfig,
                              const QDateTime importStartTime,
                              const bool appendHash,
                              const bool importInPlace,
                              const bool replaceLocation)
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
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(abspath);
    image->readMetadata();
    Exiv2::ExifData exifData = image->exifData();
    Exiv2::XmpData xmpData = image->xmpData();


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
            int value = carry + ((uint8_t) hashArray.at(i));
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
    outputPath.append(exifLocalDateString(exifData, cameraTZ, importTZ, dirConfig));
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
    backupPath.append(exifLocalDateString(exifData, cameraTZ, importTZ, dirConfig));
    QString backupPathName = backupPath;
    backupPathName.append(filename);

    //Create the directory, if the root exists, and we're not importing in place.
    QDir backupRoot(backupDir);
    if (backupRoot.exists() && !importInPlace)
    {
        QDir backupDirectory(backupPath);
        backupDirectory.mkpath(backupPath);
        if (!QFile::exists(backupPathName))
        {
            QFile::copy(infoIn.absoluteFilePath(), backupPathName);
        }
    }





    //Check to see if it's already present in the database.
    QSqlQuery query;
    query.prepare("SELECT FTfilepath FROM FileTable WHERE (FTfileID = ?);");
    query.bindValue(0, hashString);
    query.exec();
    query.next();
    const QString dbRecordedPath = query.value(0).toString();
    //If it's not in the database yet,
    //And we're not updating locations
    //  (if we are updating locations, we don't want it to add new things to the db)
    bool changedST = false;
    if (dbRecordedPath == "" && !replaceLocation)
    {
        cout << "importWorker no replace, doesn't exist" << endl;
        //Record the file location in the database.
        if (!importInPlace)
        {
            //Copy the file into our main directory. We assume it's not in here yet.
            QFile::copy(infoIn.absoluteFilePath(), outputPathName);
            fileInsert(hashString, outputPathName, exifData);
        }
        else
        {
            //If it's being imported in place, then we don't copy the file.
            fileInsert(hashString, infoIn.absoluteFilePath(), exifData);
        }

        //Now create a profile and a search table entry, and a thumbnail.
        QString STsearchID;
        STsearchID = createNewProfile(hashString,
                                      filename,
                                      exifUtcTime(exifData, cameraTZ),
                                      importStartTime,
                                      exifData,
                                      xmpData);

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
    else if (dbRecordedPath != "")//it's already in the database, so just move the file.
    {
        //See if the file is in its old location, and copy if not.
        //DON'T do this if we're updating the location.
        if (!QFile::exists(dbRecordedPath) && !importInPlace && !replaceLocation)
        {
            QFile::copy(infoIn.absoluteFilePath(), outputPathName);
        }

        //If we want to update the location of the file.
        if (replaceLocation)
        {
            fileInsert(hashString, infoIn.absoluteFilePath(), exifData);
            cout << "importWorker replace location: " << infoIn.absoluteFilePath().toStdString() << endl;

            QString STsearchID = hashString.append(QString("%1").arg(1, 4, 10, QLatin1Char('0')));
            cout << "importWorker replace STsearchID: " << STsearchID.toStdString() << endl;

            if (QString("") != STsearchID)
            {
                emit enqueueThis(STsearchID);
            }
        }
    }
    //else do nothing.

    //Tell the ImportModel whether we did anything to the SearchTable
    emit doneProcessing(changedST);
}
