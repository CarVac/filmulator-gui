#include "organizemodel.h"
#include <iostream>
#include <QStringList>
#include <exiv2/exiv2.hpp>
#include <QCryptographicHash>
#include <QDateTime>
#include <QString>

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
    std::string queryString = "SELECT * ";
    queryString.append( "FROM SearchTable, FileTable, ProcessingTable " );
    queryString.append( "WHERE " );
    queryString.append( "SearchTable.searchID = ProcessingTable.procID " );
    queryString.append( "AND SearchTable.sourceHash = FileTable.fileID " );

    //Here we do the filtering.
    //For unsigned ones, if the max____Time is 0, then we don't filter.
    //For signed ones, if the max____ is <0, then we don't filter.


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

    setQuery( QString::fromStdString( queryString ) );

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
//        std::cout << qPrintable( fileList.at( i ).absoluteFilePath() ) << std::endl;
        QCryptographicHash hash( QCryptographicHash::Md5 );
        QFile file( fileList.at( i ).absoluteFilePath() );
        if ( !file.open( QIODevice::ReadOnly ) )
        {
            QDebug( "File couldn't be opened." );
//            std::cout << "File couldn't be opened." << std::endl;
        }

        while ( !file.atEnd() )
        {
            hash.addData( file.read( 8192 ) );
        }
//        std::cout << qPrintable( hash.result().toHex() ) << std::endl;
        const char *cstr = fileList.at( i ).absoluteFilePath().toStdString().c_str();
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open( cstr );
        image->readMetadata();
        Exiv2::ExifData exifData = image->exifData();
        QString exifDateTime = QString::fromStdString( exifData[ "Exif.Image.DateTime" ].toString() );
        QDateTime cameraDateTime = QDateTime::fromString( exifDateTime, "yyyy:MM:dd hh:mm:ss" );

        //Now to figure out what time and date in the camera local timezone the thing was.
        //I want to correctly convert to UTC. I need to set the datetime to the camera timezone.
//        cout << cameraTZ << endl;
        cameraDateTime.setUtcOffset( cameraTZ );
//        cout << cameraDateTime.toTime_t() << endl;
        //This lets us ensure that the UTC is correct.

        //Now, I want the folders to be set according to the local timezone where/when captured.
        //They might get screwed up in the order, if you change the import timezones, but oh well.
        //The unix time representation will still be okay.
        QDateTime captureLocaLDateTime = QDateTime::fromTime_t( cameraDateTime.toTime_t() );
        captureLocaLDateTime.setUtcOffset( importTZ );
        QDate captureLocalDate = captureLocaLDateTime.date();

        //Now I'm setting up the directory to put the file into.
        QString outputPathName = photoDir;
        outputPathName.append( captureLocalDate.toString( dirConfig ) );
        QString outputPath = outputPathName;
//        cout << "Output folder: " << outputPath.toStdString() << endl;
        QDir dir( outputPath );
        dir.mkpath( outputPath );//For some reason, QDir::mkpath( outputPath) didn't work on its own.

        //This is the full file path.
        outputPathName.append( fileList.at(i).fileName());
        cout << "Output path: " << outputPathName.toStdString() << endl;
        if ( !QFile::exists( outputPathName ) )
        {
            QFile::copy( fileList.at( i ).absoluteFilePath(), outputPathName );
        }
        else
        {
            cout << "Already exists." << endl;
        }

        
        query.prepare( "REPLACE INTO FileTable values (?,?,?,?,?,?,?,?);" );
        query.bindValue( 0, hash.result().toHex() );
        query.bindValue( 1, fileList.at( i ).fileName() );
        query.bindValue( 2, QString::fromStdString( exifData[ "Exif.Image.Make" ].toString() ) );
        query.bindValue( 3, QString::fromStdString( exifData[ "Exif.Image.Model" ].toString() ) );
        query.bindValue( 4, exifData[ "Exif.Photo.ISOSpeedRatings" ].toFloat() );
        query.bindValue( 5, QString::fromStdString( exifData[ "Exif.Photo.ExposureTime" ].toString() ) );
        query.bindValue( 6, exifData[ "Exif.Photo.FNumber" ].toFloat() );
        query.bindValue( 7, exifData[ "Exif.Photo.FocalLength" ].toFloat() );
        query.exec();
        
    }
}
