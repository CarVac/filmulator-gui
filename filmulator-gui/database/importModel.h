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

struct importParams {
    QFileInfo fileInfoParam;
    int importTZParam;
    int cameraTZParam;
    QString photoDirParam;
    QString backupDirParam;
    QString dirConfigParam;
    QDateTime importTimeParam;
};

class ImportModel : public SqlModel
{
    Q_OBJECT

    Q_PROPERTY(int importTZ      READ getImportTZ      WRITE setImportTZ      NOTIFY importTZChanged)
    Q_PROPERTY(int cameraTZ      READ getCameraTZ      WRITE setCameraTZ      NOTIFY cameraTZChanged)

    Q_PROPERTY(QString photoDir  READ getPhotoDir      WRITE setPhotoDir      NOTIFY photoDirChanged)
    Q_PROPERTY(QString backupDir READ getBackupDir     WRITE setBackupDir     NOTIFY backupDirChanged)
    Q_PROPERTY(QString dirConfig READ getDirConfig     WRITE setDirConfig     NOTIFY dirConfigChanged)

    Q_PROPERTY(float progress READ getProgress NOTIFY progressChanged)
    Q_PROPERTY(bool emptyDir READ getEmptyDir NOTIFY emptyDirChanged)

public:
    explicit ImportModel(QObject *parent = 0);
    Q_INVOKABLE void importDirectory_r(const QString dir);

    void setImportTZ(const int offsetIn);
    void setCameraTZ(const int offsetIn);

    void setPhotoDir(const QString dirIn);
    void setBackupDir(const QString dirIn);
    void setDirConfig(const QString configIn);

    int getImportTZ() {return importTZ/3600;}
    int getCameraTZ() {return cameraTZ/3600;}

    QString getPhotoDir() {return photoDir;}
    QString getBackupDir() {return backupDir;}
    QString getDirConfig() {return dirConfig;}

    float getProgress() {return progress;}
    bool getEmptyDir() {return emptyDir;}

public slots:
    void workerFinished();

signals:
    void importTZChanged();
    void cameraTZChanged();

    void photoDirChanged();
    void backupDirChanged();
    void dirConfigChanged();

    void progressChanged();
    void emptyDirChanged();

    void searchTableChanged();

    void workForWorker(const QFileInfo infoIn,
                       const int importTZ,
                       const int cameraTZ,
                       const QString photoDir,
                       const QString backupDir,
                       const QString dirConfig,
                       const QDateTime importTime);
protected:
    int importTZ;
    int cameraTZ;

    QString photoDir;
    QString backupDir;
    QString dirConfig;

    std::deque<importParams> queue;
    int maxQueue;
    float progress = 1;
    bool emptyDir = false;

    QThread workerThread;

    bool paused = true;

    QMutex mutex;

    void startWorker(importParams);
};

#endif // IMPORTMODEL_H
