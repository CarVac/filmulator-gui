
#include "importModel.h"
#include <iostream>

using std::cout;
using std::endl;

ImportModel::ImportModel(QObject *parent) : SqlModel(parent)
{
    tableName = "SearchTable";

    //Set up the files that it accepts as raw files on directory import
    rawNameFilters << "*.CR2" << "*.cr2" << "*.NEF" << "*.nef" << "*.DNG" << "*.dng" << "*.RW2" << "*.rw2" << "*.IIQ" << "*.iiq" << "*.ARW" << "*.arw" << "*.PEF" << "*.pef" << "*.RAF" << "*.raf" << "*.ORF" << "*.orf";

    //Set up the files that it'll show in the file picker
    dirNameFilters << "Raw image files (*.CR2 *.cr2 *.NEF *.nef *.DNG *.dng *.RW2 *.rw2 *.IIQ *.iiq *.ARW *.arw *.PEF *.pef *.RAF *.raf *.ORF *.orf)";// << "All files (*)";

    //Set up the import worker thread
    ImportWorker *worker = new ImportWorker;
    worker->moveToThread(&workerThread);
    connect(this, SIGNAL(workForWorker(const QFileInfo,
                                       const int,
                                       const int,
                                       const QString,
                                       const QString,
                                       const QString,
                                       const QDateTime,
                                       const bool,
                                       const bool)),
            worker, SLOT(importFile(const QFileInfo,
                                    const int,
                                    const int,
                                    const QString,
                                    const QString,
                                    const QString,
                                    const QDateTime,
                                    const bool,
                                    const bool)));
    connect(worker, SIGNAL(doneProcessing()), this, SLOT(workerFinished()));
    connect(worker, SIGNAL(enqueueThis(QString)), this, SLOT(enqueueRequested(QString)));
    workerThread.start(QThread::LowPriority);
}

QSqlQuery ImportModel::modelQuery()
{
    return QSqlQuery(QString(""));
}

//We want to scan for DCIM to warn people not to import in place from a memory card.
//If any cameras eventually have things that are not DCIM, they should be added here.
bool ImportModel::pathContainsDCIM(const QString dir, const bool notDirectory)
{
    //First, check to see if the directory contains the substring DCIM.
    if (dir.contains("DCIM", Qt::CaseSensitive))
    {
        return true;
    }
    else if (notDirectory)
    {
        return false;
    }
    else
    {
        //Next, we call this function recursively on the folders within.
        QDir directory = QDir(dir);
        directory.setFilter(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        directory.setSorting(QDir::Name);
        QFileInfoList dirList = directory.entryInfoList();
        for (int i=0; i < dirList.size(); i++)
        {
            if (pathContainsDCIM(dirList.at(i).absoluteFilePath(), false))
            {
                return true;
            }
        }
    }
    return false;
}

void ImportModel::importDirectory_r(const QString dir, const bool importInPlace)
{
    //This function reads in a directory and puts the raws into the database.
    if (dir.length() == 0)
    {
        emptyDir = true;
        emit emptyDirChanged();
        return;
    }
    else
    {
        emptyDir = false;
        emit emptyDirChanged();
    }

    //First, we call itself recursively on the folders within.
    QDir directory = QDir(dir);
    directory.setFilter(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    directory.setSorting(QDir::Name);
    QFileInfoList dirList = directory.entryInfoList();
    for (int i=0; i < dirList.size(); i++)
    {
        importDirectory_r(dirList.at(i).absoluteFilePath(), importInPlace);
    }

    //Next, we filter for files.
    directory.setFilter(QDir::Files | QDir::NoSymLinks);
    directory.setNameFilters(rawNameFilters);
    QFileInfoList fileList = directory.entryInfoList();

    if (fileList.size() == 0)
    {
        return;
    }

    QMutexLocker locker(&mutex);
    if (0 == queue.size())
    {
        maxQueue = 0;
    }

    QDateTime now = QDateTime::currentDateTime();
    for (int i = 0; i < fileList.size(); i++)
    {
        importParams params;
        params.fileInfoParam = fileList.at(i);
        params.importTZParam = importTZ;
        params.cameraTZParam = cameraTZ;
        params.photoDirParam = photoDir;
        params.backupDirParam = backupDir;
        params.dirConfigParam = dirConfig;
        params.importStartTimeParam = now;
        params.appendHashParam = appendHash;
        params.importInPlace = importInPlace;
        queue.push_back(params);
        maxQueue++;
    }

    progress = float(maxQueue - queue.size())/float(maxQueue);
    progressFrac = "Progress: "+QString::number(maxQueue - queue.size())+"/"+QString::number(maxQueue);
    emit progressChanged();
    emit progressFracChanged();

    paused = false;

    startWorker(queue.front());
}

QStringList ImportModel::getNameFilters()
{
    return dirNameFilters;
}

//This imports a single file, taking in a file path URL as a QString.
void ImportModel::importFile(const QString name, const bool importInPlace)
{

    //First check that it's a file.
    const QFileInfo file = QFileInfo(name.mid(7));
    if (!file.isFile())
    {
        invalidFile = true;
        emit invalidFileChanged();
        return;
    }

    bool isReadableFile = false;
    //First check if it's raw.
    for (int i = 0; i < rawNameFilters.size(); i++)
    {
        if (file.fileName().endsWith(rawNameFilters.at(i).mid(1)))
        {
            isReadableFile = true;
        }
    }
    //Future type checks go here.
    //Now we tell the GUI the result:
    if (!isReadableFile)
    {
        invalidFile = true;
        emit invalidFileChanged();
        return;
    }
    else
    {
        invalidFile = false;
        emit invalidFileChanged();
    }

    QMutexLocker locker(&mutex);
    if (0 == queue.size())
    {
        maxQueue = 0;
    }

    QDateTime now = QDateTime::currentDateTime();
    importParams params;
    params.fileInfoParam = file;
    params.importTZParam = importTZ;
    params.cameraTZParam = cameraTZ;
    params.photoDirParam = photoDir;
    params.backupDirParam = backupDir;
    params.dirConfigParam = dirConfig;
    params.importStartTimeParam = now;
    params.appendHashParam = appendHash;
    params.importInPlace = importInPlace;
    queue.push_back(params);
    maxQueue++;

    progress = float(maxQueue - queue.size())/float(maxQueue);
    progressFrac = "Progress: "+QString::number(maxQueue - queue.size())+"/"+QString::number(maxQueue);
    emit progressChanged();
    emit progressFracChanged();

    paused = false;

    startWorker(queue.front());
}

//This will import multiple files, but I'm not sure exactly how it works.
void ImportModel::importFileList(const QString name, const bool importInPlace)
{
    const QStringList nameList = name.split(",");
    for (int i = 0; i < nameList.size(); i++)
    {
        importFile(nameList.at(i), importInPlace);
    }
}

void ImportModel::workerFinished()
{
    QMutexLocker locker(&mutex);
    //cout << "ImportModel queue items remaining: " << queue.size() << endl;
    emit searchTableChanged();
    if (queue.size() <= 0)
    {
        //cout << "ImportModel no more work; empty queue" << endl;
        return;
    }

    queue.pop_front();
    if (maxQueue != 0)
    {
        progress = float(maxQueue - queue.size())/float(maxQueue);
        progressFrac = "Progress: "+QString::number(maxQueue - queue.size())+"/"+QString::number(maxQueue);
        //cout << "ImportModel::workerFinished; progress = " << progress << endl;
        emit progressChanged();
        emit progressFracChanged();
    }

    if (queue.size() <= 0)
    {
        //cout << "ImportModel no more work; just emptied the queue" << endl;
        return;
    }
    else if (!paused)
    {
        startWorker(queue.front());
    }
}

void ImportModel::enqueueRequested(const QString STsearchID)
{
    if (enqueue)
    {
        emit enqueueThis(STsearchID);
    }
}

void ImportModel::startWorker(const importParams params)
{
    const QFileInfo info = params.fileInfoParam;
    const int iTZ = params.importTZParam;
    const int cTZ = params.cameraTZParam;
    const QString pDir = params.photoDirParam;
    const QString bDir = params.backupDirParam;
    const QString dConf = params.dirConfigParam;
    const QDateTime time = params.importStartTimeParam;
    const bool append = params.appendHashParam;
    const bool inPlace = params.importInPlace;
    emit workForWorker(info, iTZ, cTZ, pDir, bDir, dConf, time, append, inPlace);
}
