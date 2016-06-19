#ifndef IMPORTMODEL_H
#define IMPORTMODEL_H

#include "sqlModel.h"
#include <QString>
#include <exiv2/exiv2.hpp>
#include <deque>
#include "importWorker.h"
#include <QThread>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QDateTime>
#include <QStringList>

struct importParams {
    QFileInfo fileInfoParam;
    int importTZParam;
    int cameraTZParam;
    QString photoDirParam;
    QString backupDirParam;
    QString dirConfigParam;
    QDateTime importStartTimeParam;
    bool appendHashParam;
};

class ImportModel : public SqlModel
{
    Q_OBJECT

    Q_PROPERTY(int importTZ      READ getImportTZ  WRITE setImportTZ  NOTIFY importTZChanged)
    Q_PROPERTY(int cameraTZ      READ getCameraTZ  WRITE setCameraTZ  NOTIFY cameraTZChanged)

    Q_PROPERTY(QString photoDir  READ getPhotoDir  WRITE setPhotoDir  NOTIFY photoDirChanged)
    Q_PROPERTY(QString backupDir READ getBackupDir WRITE setBackupDir NOTIFY backupDirChanged)
    Q_PROPERTY(QString dirConfig READ getDirConfig WRITE setDirConfig NOTIFY dirConfigChanged)

    //Whether or not to enqueue the image in the editor queue upon loading.
    Q_PROPERTY(bool enqueue    READ getEnqueue    WRITE setEnqueue    NOTIFY enqueueChanged)
    //Whether or not to append a part of the image hash to the filename when copying
    Q_PROPERTY(bool appendHash READ getAppendHash WRITE setAppendHash NOTIFY appendHashChanged)

    Q_PROPERTY(float progress READ getProgress NOTIFY progressChanged)
    Q_PROPERTY(QString progressFrac READ getProgressFrac NOTIFY progressFracChanged)
    Q_PROPERTY(bool emptyDir READ getEmptyDir NOTIFY emptyDirChanged)

public:
    explicit ImportModel(QObject *parent = 0);
    Q_INVOKABLE bool pathContainsDCIM(const QString dir, const bool notDirectory);
    Q_INVOKABLE void importDirectory_r(const QString dir);
    Q_INVOKABLE void importFile(const QString name);
    Q_INVOKABLE void importFileList(const QString name);
    Q_INVOKABLE QStringList getNameFilters();

    void setImportTZ(const int offsetIn);
    void setCameraTZ(const int offsetIn);

    void setPhotoDir(const QString dirIn);
    void setBackupDir(const QString dirIn);
    void setDirConfig(const QString configIn);

    void setEnqueue(const bool enqueueIn);
    void setAppendHash(const bool appendHashIn);

    int getImportTZ() {return importTZ/3600;}
    int getCameraTZ() {return cameraTZ/3600;}

    QString getPhotoDir() {return photoDir;}
    QString getBackupDir() {return backupDir;}
    QString getDirConfig() {return dirConfig;}

    bool getEnqueue() {return enqueue;}
    bool getAppendHash() {return appendHash;}

    float getProgress() {return progress;}
    QString getProgressFrac() {return progressFrac;}
    bool getEmptyDir() {return emptyDir;}

public slots:
    void workerFinished();
    void enqueueRequested(QString STsearchID);

signals:
    void importTZChanged();
    void cameraTZChanged();

    void photoDirChanged();
    void backupDirChanged();
    void dirConfigChanged();

    void enqueueChanged();
    void appendHashChanged();

    void progressChanged();
    void progressFracChanged();
    void emptyDirChanged();

    void searchTableChanged();
    void enqueueThis(QString STsearchID);

    void workForWorker(const QFileInfo infoIn,
                       const int importTZ,
                       const int cameraTZ,
                       const QString photoDir,
                       const QString backupDir,
                       const QString dirConfig,
                       const QDateTime importStartTime,
                       const bool appendHash);

    void importChanged();

protected:
    QSqlQuery modelQuery();
    void emitChange() {emit importChanged();}

    int importTZ;
    int cameraTZ;

    QString photoDir;
    QString backupDir;
    QString dirConfig;

    bool enqueue;
    bool appendHash;

    //For what is accepted on directory import
    QStringList rawNameFilters;
    //For the file picker
    QStringList dirNameFilters;

    std::deque<importParams> queue;
    int maxQueue;
    float progress = 1;
    QString progressFrac = "Progress: 0/0";
    bool emptyDir = false;

    QThread workerThread;

    bool paused = true;

    QMutex mutex;

    void startWorker(importParams);
};

#endif // IMPORTMODEL_H
