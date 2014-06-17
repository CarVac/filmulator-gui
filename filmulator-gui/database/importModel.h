#ifndef IMPORTMODEL_H
#define IMPORTMODEL_H

#include "sqlModel.h"
#include <QString>
#include <exiv2/exiv2.hpp>
using namespace std;

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
    Q_INVOKABLE void importDirectory_r( QString dir );

    void setImportTZ( int offsetIn );
    void setCameraTZ( int offsetIn );

    void setPhotoDir( QString dirIn );
    void setBackupDir( QString dirIn );
    void setDirConfig( QString configIn );

    int getImportTZ() { return importTZ/3600; }
    int getCameraTZ() { return cameraTZ/3600; }

    QString getPhotoDir() { return photoDir; }
    QString getBackupDir() { return backupDir; }
    QString getDirConfig() { return dirConfig; }

signals:
    void importTZChanged();
    void cameraTZChanged();

    void photoDirChanged();
    void backupDirChanged();
    void dirConfigChanged();

protected:
    int importTZ;
    int cameraTZ;

    QString photoDir;
    QString backupDir;
    QString dirConfig;
};

#endif // IMPORTMODEL_H
