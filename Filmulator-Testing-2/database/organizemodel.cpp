#include "organizemodel.h"
#include <iostream>
#include <QStringList>
#include <exiv2/exiv2.hpp>
#include <QCryptographicHash>
#include <QDateTime>
#include <QString>
#include "exiffunctions.h"

using namespace std;

OrganizeModel::OrganizeModel( QObject *parent ) :
    SqlModel( parent )
{
}

void OrganizeModel::setOrganizeQuery()
{
    //We can't use the inbuilt relational table stuff; we have to
    // make our own writing functionality, and instead of setting the table,
    // we have to make our own query.

    //This doesn't support tags yet, unfortunately.
    //Tags will need to each have their own table pointing at SearchTable id's.
    //Then, we would have to add the tag table into the FROM statement
    //and another equality statement there.

    //First we will prepare a string to feed into the query.
    //We only really care about info in the searchtable.
    std::string queryString = "SELECT * ";
    queryString.append( "FROM SearchTable " );
//    queryString.append( "WHERE " );
//    queryString.append( "SearchTable.searchID = ProcessingTable.procID " );
//    queryString.append( "AND SearchTable.sourceHash = FileTable.fileID " );

    //Here we do the filtering.
    //For unsigned ones, if the max____Time is 0, then we don't filter.
    //For signed ones, if the max____ is <0, then we don't filter.

/*
    if ( maxCaptureTime != 0 )
    {
        queryString.append( "AND SearchTable.captureTime <= " );
        queryString.append( std::to_string( maxCaptureTime ) );
        queryString.append( " " );
        queryString.append( "AND SearchTable.captureTime >= " );
        queryString.append( std::to_string( minCaptureTime ) );
        queryString.append( " " );
    }
    if ( maxImportTime != 0 )
    {
        queryString.append( "AND SearchTable.importTime <= " );
        queryString.append( std::to_string( maxImportTime ) );
        queryString.append( " " );
        queryString.append( "AND SearchTable.importTime >= " );
        queryString.append( std::to_string( minImportTime ) );
        queryString.append( " " );
    }
    if ( maxProcessedTime != 0 )
    {
        queryString.append( "AND SearchTable.lastProcessedTime <= " );
        queryString.append( std::to_string( maxProcessedTime ) );
        queryString.append( " " );
        queryString.append( "AND SearchTable.lastProcessedTime >= " );
        queryString.append( std::to_string( minProcessedTime ) );
        queryString.append( " " );
    }
    if ( maxRating >= 0 )
    {
        queryString.append( "AND SearchTable.rating <= " );
        queryString.append( std::to_string( maxRating ) );
        queryString.append( " " );
        queryString.append( "AND SearchTable.rating >= " );
        queryString.append( std::to_string( minRating ) );
        queryString.append( " " );
    }
*/
    //Now we go to the ordering.
    //By default, we will always sort by captureTime and filename,
    //but we want them to be last in case some other sorting method is chosen.
    //It doesn't really matter what the order is other than that, except that
    // we want the rating first because it has actual categories.
    //Any other stuff will have been covered by the filtering.

    //First we need to actually write ORDER BY
    queryString.append( "ORDER BY " );

    if ( ratingSort == 1 ){ queryString.append( "SearchTable.Rating ASC, " ); }
    else if ( ratingSort == -1 ){ queryString.append( "SearchTable.Rating DESC, " ); }

    if ( processedSort == 1 ){ queryString.append( "SearchTable.lastProcessedTime ASC, " ); }
    else if ( processedSort == -1 ){ queryString.append( "SearchTable.lastProcessedTime DESC, " ); }

    if ( importSort == 1 ){ queryString.append( "SearchTable.importTime ASC, " ); }
    else if ( importSort == -1 ){ queryString.append( "searchTable.importTime DESC, " ); }

    if ( captureSort == 1 )
    {
        queryString.append( "SearchTable.captureTime ASC, " );
        queryString.append( "SearchTable.filename ASC;" );
    }
    else //if ( captureSort == -1 )
    {
        queryString.append( "SearchTable.captureTime DESC, " );
        queryString.append( "SearchTable.filename DESC;" );
    }

    cout << queryString << endl;

    setQuery( QSqlQuery( QString::fromStdString( queryString ) ) );
 }

void OrganizeModel::importDirectory_r( QString dir )
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
    nameFilters << "*.CR2" << "*.NEF" << "*.DNG" << "*.dng" << "*.RW2" << "*.IIQ" << "*.ARW";
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

        //Here I'm setting up the directory to put the file into based on the capture-local date.
        QString outputPathName = photoDir;
        outputPathName.append( exifLocalDateString( exifData, cameraTZ, importTZ, dirConfig ) );
        QString outputPath = outputPathName;
//        cout << "Output folder: " << outputPath.toStdString() << endl;
        QDir dir( outputPath );
        dir.mkpath( outputPath );//For some reason, QDir::mkpath( outputPath) didn't work on its own.

        //This sets the full file path including the filename.
        outputPathName.append( fileList.at(i).fileName());
        cout << "Output path: " << outputPathName.toStdString() << endl;


        //Now we test to see if it's in the database already by comparing the hashes.
        query.prepare( "SELECT filePath from FileTable WHERE (fileID = ?);" );
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
                              fileList.at(i).fileName(),
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
