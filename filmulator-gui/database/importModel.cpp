
#include "importModel.h"
#include <iostream>
#include <QStringList>

ImportModel::ImportModel( QObject *parent ) : SqlModel( parent )
{
    ImportWorker *worker = new ImportWorker;
    worker->moveToThread( &workerThread );
    connect( this, &ImportModel::workForWorker, worker, &ImportWorker::importFile );
    connect( worker, &ImportWorker::doneProcessing, this, &ImportModel::workerFinished );
    workerThread.start( QThread::LowPriority );
}

void ImportModel::importDirectory_r( const QString dir )
{
    //This function reads in a directory and puts the raws into the database.
    if ( dir.length() == 0 )
    {
        return;
    }

    //First, we call itself recursively on the folders within.
    QDir directory = QDir( dir );
    directory.setFilter( QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );
    directory.setSorting( QDir::Name );
    QFileInfoList dirList = directory.entryInfoList();
    for ( int i=0; i < dirList.size(); i++ )
    {
        importDirectory_r( dirList.at( i ).absoluteFilePath() );
    }

    //Next, we filter for files.
    directory.setFilter( QDir::Files | QDir::NoSymLinks );
    QStringList nameFilters;
    nameFilters << "*.CR2" << "*.NEF" << "*.DNG" << "*.dng" << "*.RW2" << "*.IIQ" << "*.ARW" << "*.PEF";
    directory.setNameFilters( nameFilters );
    QFileInfoList fileList = directory.entryInfoList();

    if ( fileList.size() == 0 )
    {
        return;
    }

    for ( int i = 0; i < fileList.size(); i++ )
    {
        importParams params;
        params.fileInfo = fileList.at( i );
        params.importTZ = importTZ;
        params.cameraTZ = cameraTZ;
        params.photoDir = photoDir;
        params.backupDir = backupDir;
        params.dirConfig = dirConfig;
        queue.push_back( params );
    }

    paused = false;

    startWorker();
}

void ImportModel::workerFinished()
{
    queue.pop_front();

    if ( queue.size() == 0 )
    {
        return;
    }
    else if ( !paused )
    {
        startWorker();
    }
}

void ImportModel::startWorker()
{

    const QFileInfo info = queue.front().fileInfo;
    const int iTZ = queue.front().importTZ;
    const int cTZ = queue.front().cameraTZ;
    const QString pDir = queue.front().photoDir;
    const QString bDir = queue.front().backupDir;
    const QString dConf = queue.front().dirConfig;
    emit workForWorker( info, iTZ, cTZ, pDir, bDir, dConf );
}
