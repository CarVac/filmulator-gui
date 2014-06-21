#ifndef IMPORTMODEL_H
#define IMPORTMODEL_H

#include "sqlModel.h"
#include <QString>
#include <exiv2/exiv2.hpp>
#include <deque>
#include "importWorker.h"
#include <QThread>
#include <QFileInfo>


struct importParams {
    QFileInfo fileInfo;
    int importTZ;
    int cameraTZ;
    QString photoDir;
    QString backupDir;
    QString dirConfig;
};

class ImportModel : public SqlModel
{
    Q_OBJECT

    Q_PROPERTY( int importTZ      READ getImportTZ      WRITE setImportTZ      NOTIFY importTZChanged )
    Q_PROPERTY( int cameraTZ      READ getCameraTZ      WRITE setCameraTZ      NOTIFY cameraTZChanged )

    Q_PROPERTY( QString photoDir  READ getPhotoDir      WRITE setPhotoDir      NOTIFY photoDirChanged )
    Q_PROPERTY( QString backupDir READ getBackupDir     WRITE setBackupDir     NOTIFY backupDirChanged )
    Q_PROPERTY( QString dirConfig READ getDirConfig     WRITE setDirConfig     NOTIFY dirConfigChanged )

public:
    explicit ImportModel( QObject *parent = 0 );
    Q_INVOKABLE void importDirectory_r( const QString dir );

    void setImportTZ( const int offsetIn );
    void setCameraTZ( const int offsetIn );

    void setPhotoDir( const QString dirIn );
    void setBackupDir( const QString dirIn );
    void setDirConfig( const QString configIn );

    int getImportTZ() { return importTZ/3600; }
    int getCameraTZ() { return cameraTZ/3600; }

    QString getPhotoDir() { return photoDir; }
    QString getBackupDir() { return backupDir; }
    QString getDirConfig() { return dirConfig; }

public slots:
    void workerFinished();

signals:
    void importTZChanged();
    void cameraTZChanged();

    void photoDirChanged();
    void backupDirChanged();
    void dirConfigChanged();

    void workForWorker( const QFileInfo infoIn,
                        const int importTZ,
                        const int cameraTZ,
                        const QString photoDir,
                        const QString backupDir,
                        const QString dirConfig );
protected:
    int importTZ;
    int cameraTZ;

    QString photoDir;
    QString backupDir;
    QString dirConfig;

    std::deque<importParams> queue;

    QThread workerThread;

    bool paused = true;

    void startWorker();
};

#endif // IMPORTMODEL_H
