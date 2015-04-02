#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QDate>

class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString photoStorageDir READ getPhotoStorageDir WRITE setPhotoStorageDir NOTIFY photoStorageDirChanged)
    Q_PROPERTY(QString photoBackupDir READ getPhotoBackupDir WRITE setPhotoBackupDir NOTIFY photoBackupDirChanged)
    Q_PROPERTY(QString dirConfig READ getDirConfig WRITE setDirConfig NOTIFY dirConfigChanged)
    Q_PROPERTY(int cameraTZ READ getCameraTZ WRITE setCameraTZ NOTIFY cameraTZChanged)
    Q_PROPERTY(int importTZ READ getImportTZ WRITE setImportTZ NOTIFY importTZChanged)
    Q_PROPERTY(int organizeTZ READ getOrganizeTZ WRITE setOrganizeTZ NOTIFY organizeTZChanged)
    Q_PROPERTY(QDate organizeCaptureDate READ getOrganizeCaptureDate WRITE setOrganizeCaptureDate NOTIFY organizeCaptureDateChanged)
    Q_PROPERTY(float uiScale READ getUiScale WRITE setUiScale NOTIFY uiScaleChanged)
    Q_PROPERTY(bool enqueue READ getEnqueue WRITE setEnqueue NOTIFY enqueueChanged)

public:
    explicit Settings(QObject *parent = 0);
    void setPhotoStorageDir(QString dirIn);
    void setPhotoBackupDir(QString dirIn);
    void setDirConfig(QString configIn);
    void setCameraTZ(int offsetIn);
    void setImportTZ(int offsetIn);
    void setOrganizeTZ(int offsetIn);
    void setOrganizeCaptureDate(QDate dateIn);
    void setUiScale(float uiScaleIn);
    void setEnqueue(bool enqueueIn);

    Q_INVOKABLE QString getPhotoStorageDir();
    Q_INVOKABLE QString getPhotoBackupDir();
    Q_INVOKABLE QString getDirConfig();
    Q_INVOKABLE int getCameraTZ();
    Q_INVOKABLE int getImportTZ();
    Q_INVOKABLE int getOrganizeTZ();
    Q_INVOKABLE QDate getOrganizeCaptureDate();
    Q_INVOKABLE float getUiScale();
    Q_INVOKABLE bool getEnqueue();

protected:
    QString photoStorageDir;
    QString photoBackupDir;
    QString dirConfig;
    int cameraTZ;
    int importTZ;
    int organizeTZ;
    QDate organizeCaptureDate;
    float uiScale;
    bool enqueue;

signals:
    void photoStorageDirChanged();
    void photoBackupDirChanged();
    void dirConfigChanged();
    void cameraTZChanged();
    void importTZChanged();
    void organizeTZChanged();
    void organizeCaptureDateChanged();
    void uiScaleChanged();
    void enqueueChanged();
};

#endif // SETTINGS_H
