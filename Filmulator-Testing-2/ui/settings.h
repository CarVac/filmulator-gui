#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString photoStorageDir READ getPhotoStorageDir WRITE setPhotoStorageDir NOTIFY photoStorageDirChanged )
    Q_PROPERTY( QString photoBackupDir READ getPhotoBackupDir WRITE setPhotoBackupDir NOTIFY photoBackupDirChanged )
    Q_PROPERTY( QString dirConfig READ getDirConfig WRITE setDirConfig NOTIFY dirConfigChanged )
    Q_PROPERTY( int cameraTZ READ getCameraTZ WRITE setCameraTZ NOTIFY cameraTZChanged )
    Q_PROPERTY( int importTZ READ getImportTZ WRITE setImportTZ NOTIFY importTZChanged )

public:
    explicit Settings( QObject *parent = 0 );
    void setPhotoStorageDir( QString dirIn );
    void setPhotoBackupDir( QString dirIn );
    void setDirConfig( QString configIn );
    void setCameraTZ( int offsetIn );
    void setImportTZ( int offsetIn );

    Q_INVOKABLE QString getPhotoStorageDir();
    Q_INVOKABLE QString getPhotoBackupDir();
    Q_INVOKABLE QString getDirConfig();
    Q_INVOKABLE int getCameraTZ();
    Q_INVOKABLE int getImportTZ();

protected:
    QString photoStorageDir;
    QString photoBackupDir;
    QString dirConfig;
    int cameraTZ;
    int importTZ;

signals:
    void photoStorageDirChanged();
    void photoBackupDirChanged();
    void dirConfigChanged();
    void cameraTZChanged();
    void importTZChanged();
};

#endif // SETTINGS_H
