#include "importWorker.h"
#include <iostream>

ImportWorker::ImportWorker( QObject *parent ) : QObject( parent )
{
}

void ImportWorker::importFile( const QFileInfo infoIn,
                               const int importTZ,
                               const int cameraTZ,
                               const QString photoDir,
                               const QString backupDir,
                               const QString dirConfig )
{
    //Generate a hash of the raw file.
    QCryptographicHash hash( QCryptographicHash::Md5 );
    QFile file( infoIn.absoluteFilePath() );
    if ( !file.open( QIODevice::ReadOnly ) )
    {
        qDebug( "File couldn't be opened." );
    }

    //Load data into the hash function.
    while ( !file.atEnd() )
    {
        hash.addData( file.read( 8192 ) );
    }
    QString hashString = QString( hash.result().toHex() );

    //Grab EXIF data from the file.
    const char *cstr = infoIn.absoluteFilePath().toStdString().c_str();
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open( cstr );
    image->readMetadata();
    Exiv2::ExifData exifData = image->exifData();

    //Set up the main directory to insert the file, and the full file path.
    //This is based on what time it was in the timezone of photo capture.
    QString outputPath = photoDir;
    outputPath.append( exifLocalDateString( exifData, cameraTZ, importTZ, dirConfig ) );
    QString outputPathName = outputPath;
    outputPathName.append( infoIn.fileName() );
    //Create the directory.
    QDir dir( outputPath );
    dir.mkpath( outputPath );

    //Sets up the backup directory.
    QString backupPath = backupDir;
    backupPath.append( exifLocalDateString( exifData, cameraTZ, importTZ, dirConfig ) );
    QString backupPathName = backupPath;
    backupPathName.append( infoIn.fileName() );

    //Create the directory, if the root exists.
    QDir backupRoot( backupDir );
    if ( backupRoot.exists() )
    {
        QDir backupDirectory( backupPath );
        backupDirectory.mkpath( backupPath );
        if ( !QFile::exists( backupPathName ) )
        {
            QFile::copy( infoIn.absoluteFilePath(), backupPathName );
        }
    }

    //Check to see if it's already present in the database.
    QSqlQuery query;
    query.prepare( "SELECT FTfilepath FROM FileTable WHERE (FTfileID = ?);" );
    query.bindValue( 0, hashString );
    query.exec();
    query.next();
    const QString dbRecordedPath = query.value( 0 ).toString();
    if ( dbRecordedPath == "" )//It's not in the database yet.
    {
        //Copy the file into our main directory. We assume it's not in here yet.
        QFile::copy( infoIn.absoluteFilePath(), outputPathName );

        //Insert it into the database.
        fileInsert( hashString, outputPathName, exifData );

        //Now create a profile and a search table entry, and a thumbnail.
        createNewProfile( hashString,
                          infoIn.fileName(),
                          infoIn.absoluteFilePath(),
                          exifUtcTime( exifData, cameraTZ ),
                          exifData );
    }
    else //it's already in the database, so just move the file.
    {
        //See if the file is in its old location.
        //If the user deleted the local copy of the raw file, but are re-importing
        // from the backup, this situation might occur.
        if ( !QFile::exists( dbRecordedPath ) )
        {
            QFile::copy( infoIn.absoluteFilePath(), outputPathName );
        }
        //TODO: Perhaps there should be a "local copy available" flag set here.
    }
    emit doneProcessing();
}
