
#include "importModel.h"
#include "sqlInsertion.h"
#include "exifFunctions.h"
#include <iostream>
#include <QStringList>
#include <QCryptographicHash>
#include <QDateTime>

using namespace std;

ImportModel::ImportModel( QObject *parent ) : SqlModel( parent )
{
}

void ImportModel::importDirectory_r( QString dir )
{
    //This function reads in a directory and puts the raws into the database.
    std::cout << "importing directory " << qPrintable( dir ) << std::endl;
    if ( dir.length() == 0 )
    {
        return;
    }
    QDir directory = QDir( dir );
    directory.setFilter( QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );
    directory.setSorting( QDir::Name );
    QFileInfoList dirList = directory.entryInfoList();
    for ( int i=0; i < dirList.size(); i++ )
    {
        importDirectory_r( dirList.at( i ).absoluteFilePath() );
    }
    directory.setFilter( QDir::Files | QDir::NoSymLinks );
    QStringList nameFilters;
    nameFilters << "*.CR2" << "*.NEF" << "*.DNG" << "*.dng" << "*.RW2" << "*.IIQ" << "*.ARW" << "*.PEF";
    directory.setNameFilters( nameFilters );
    QFileInfoList fileList = directory.entryInfoList();

    QSqlQuery query;
    for ( int i = 0; i < fileList.size(); i++ )
    {
        //Generate a hash of the raw file.
        QCryptographicHash hash( QCryptographicHash::Md5 );
        QFile file( fileList.at( i ).absoluteFilePath() );
        if ( !file.open( QIODevice::ReadOnly ) )
        {
            qDebug( "File couldn't be opened." );
        }

        while ( !file.atEnd() )
        {
            hash.addData( file.read( 8192 ) );
        }
        QString hashString = QString( hash.result().toHex() );
//        std::cout << qPrintable( hashString ) << std::endl;

        //Grab EXIF data
        const char *cstr = fileList.at( i ).absoluteFilePath().toStdString().c_str();
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open( cstr );
        image->readMetadata();
        Exiv2::ExifData exifData = image->exifData();

        //Here I'm setting up the directories to put the file into based on the capture-local date.
        QString outputPathName = photoDir;
        outputPathName.append( exifLocalDateString( exifData, cameraTZ, importTZ, dirConfig ) );
        QString outputPath = outputPathName;
//        cout << "Output folder: " << outputPathName.toStdString() << endl;
        QDir dir( outputPath );
        dir.mkpath( outputPath );//For some reason, QDir::mkpath( outputPath ) didn't work.
        //This sets the full file path including the filename.
        outputPathName.append( fileList.at( i ).fileName() );

        //Now I'm dealing with the backup directory. First we need to make sure that it's not there.
        QDir backupRoot( backupDir );
        bool backupPathExists = backupRoot.exists();
        QString backupPathName = backupDir;
        backupPathName.append( exifLocalDateString( exifData, cameraTZ, importTZ, dirConfig ) );
        QString backupPath = backupPathName;
        //Now to set the full backup path including the filename.
        backupPathName.append( fileList.at( i ).fileName() );

        //Now we make sure that the root of the backup directory is there.
        //This is so the user can leave the same backup location and not worry about failure
        // in case they have an external hdd disconnected or something like that.
        if ( backupPathExists )
        {
            QDir backupDirectory( backupPath );
            backupDirectory.mkpath( backupPath );
            if ( !QFile::exists( backupPathName ) )
            {
                QFile::copy( fileList.at( i ).absoluteFilePath(), backupPathName );
            }
        }



        //Now we test to see if it's in the database already by comparing the hashes.
        query.prepare( "SELECT FTfilePath from FileTable WHERE (FTfileID = ?);" );
        query.bindValue( 0, hashString );
        query.exec();
        query.next();
        QString dbRecordedPath = query.value ( 0 ).toString();
        if ( dbRecordedPath == "" )//It's not in the DB. Start fresh.
        {
            //Copy file into the directory structure.
            QFile::copy( fileList.at( i ).absoluteFilePath(), outputPathName );

            //Insert into the database.
            fileInsert( hashString, outputPathName, exifData );

            //Now, make a profile and a search table entry, and generate the thumbnail.
            createNewProfile( hashString,
                              fileList.at( i ).fileName(),
                              fileList.at( i ).absoluteFilePath(),
                              exifUtcTime( exifData, cameraTZ ),
                              exifData );
        }
        else //It's already in the db, whether or not in the same path.
        {
            //Check that the file's still there.
            //One situation where this would be is if they deleted their local copy of the raw file, but
            // are re-importing it from their backup.
            //We want to copy the file in so it's usable, but not overwrite the profiles.
            if ( !QFile::exists( dbRecordedPath ) )//it was in the DB but the local file is gone
            {
                //copy it in. Perhaps they removed it and are restoring it from a backup.
                QFile::copy( fileList.at( i ).absoluteFilePath(), outputPathName );
            }
            //No need to make a new profile or insert into the db.
            //TODO: There should be a "local copy available?" flag that would need to be set here.
            //Maybe.
        }
    }
}
