
#include "importModel.h"
#include <iostream>
#include <QStringList>

using std::cout;
using std::endl;

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

    QMutexLocker locker(&mutex);
    if (0 == queue.size())
    {
        maxQueue = 0;
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
        maxQueue++;
    }

    paused = false;

    startWorker(queue.front());
}

void ImportModel::workerFinished()
{
    QMutexLocker locker(&mutex);
    cout << "ImportModel queue items remaining: " << queue.size() << endl;
    emit searchTableChanged();
    if ( queue.size() <= 0 )
    {
        cout << "ImportModel no more work; empty queue" << endl;
        return;
    }

    queue.pop_front();
    if (maxQueue != 0)
    {
        progress = float(maxQueue - queue.size())/float(maxQueue);
        cout << "ImportModel::workerFinished; progress = " << progress << endl;
        emit progressChanged();
    }

    if ( queue.size() <= 0 )
    {
        cout << "ImportModel no more work; just emptied the queue" << endl;
        return;
    }
    else if ( !paused )
    {
        cout << "ImportModel before startWorker" << endl;
        startWorker(queue.front());
        cout << "importModel after startWorker" << endl;
    }
}

void ImportModel::startWorker(importParams params)
{

    const QFileInfo info = params.fileInfo;
    const int iTZ = params.importTZ;
    const int cTZ = params.cameraTZ;
    const QString pDir = params.photoDir;
    const QString bDir = params.backupDir;
    const QString dConf = params.dirConfig;
    emit workForWorker( info, iTZ, cTZ, pDir, bDir, dConf );
}
