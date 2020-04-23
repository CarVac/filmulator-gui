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
    Q_PROPERTY(int organizeRating READ getOrganizeRating WRITE setOrganizeRating NOTIFY organizeRatingChanged)
    Q_PROPERTY(int maxOrganizeRating READ getMaxOrganizeRating WRITE setMaxOrganizeRating NOTIFY maxOrganizeRatingChanged)
    Q_PROPERTY(float uiScale READ getUiScale WRITE setUiScale NOTIFY uiScaleChanged)
    Q_PROPERTY(bool enqueue READ getEnqueue WRITE setEnqueue NOTIFY enqueueChanged)
    Q_PROPERTY(bool appendHash READ getAppendHash WRITE setAppendHash NOTIFY appendHashChanged)
    Q_PROPERTY(bool mipmapView READ getMipmapView WRITE setMipmapView NOTIFY mipmapViewChanged)
    Q_PROPERTY(bool lowMemMode READ getLowMemMode WRITE setLowMemMode NOTIFY lowMemModeChanged)
    Q_PROPERTY(bool quickPreview READ getQuickPreview WRITE setQuickPreview NOTIFY quickPreviewChanged)
    Q_PROPERTY(int previewResolution READ getPreviewResolution WRITE setPreviewResolution NOTIFY previewResolutionChanged)
    Q_PROPERTY(QString lensfunStatus READ getLensfunStatus NOTIFY lensfunStatusChanged)

public:
    explicit Settings(QObject *parent = 0);
    void setPhotoStorageDir(QString dirIn);
    void setPhotoBackupDir(QString dirIn);
    void setDirConfig(QString configIn);
    void setCameraTZ(int offsetIn);
    void setImportTZ(int offsetIn);
    void setOrganizeTZ(int offsetIn);
    void setOrganizeCaptureDate(QDate dateIn);
    void setOrganizeRating(int ratingIn);
    void setMaxOrganizeRating(int ratingIn);
    void setUiScale(float uiScaleIn);
    void setEnqueue(bool enqueueIn);
    void setAppendHash(bool appendHashIn);
    void setMipmapView(bool mipmapViewIn);
    void setLowMemMode(bool lowMemModeIn);
    void setQuickPreview(bool quickPreviewIn);
    void setPreviewResolution(int resolutionIn);

    Q_INVOKABLE QString getPhotoStorageDir();
    Q_INVOKABLE QString getPhotoBackupDir();
    Q_INVOKABLE QString getDirConfig();
    Q_INVOKABLE int getCameraTZ();
    Q_INVOKABLE int getImportTZ();
    Q_INVOKABLE int getOrganizeTZ();
    Q_INVOKABLE QDate getOrganizeCaptureDate();
    Q_INVOKABLE int getOrganizeRating();
    Q_INVOKABLE int getMaxOrganizeRating();
    Q_INVOKABLE float getUiScale();
    Q_INVOKABLE bool getEnqueue();
    Q_INVOKABLE bool getAppendHash();
    Q_INVOKABLE bool getMipmapView();
    Q_INVOKABLE bool getLowMemMode();
    Q_INVOKABLE bool getQuickPreview();
    Q_INVOKABLE int getPreviewResolution();
    Q_INVOKABLE QString getLensfunStatus();

protected:
    QString photoStorageDir;
    QString photoBackupDir;
    QString dirConfig;
    int cameraTZ;
    int importTZ;
    int organizeTZ;
    QDate organizeCaptureDate;
    int organizeRating;
    int maxOrganizeRating;
    float uiScale;
    bool enqueue;
    bool appendHash;
    bool mipmapView;
    bool lowMemMode;
    bool quickPreview;
    int previewResolution;
    QString lensfunStatus;

signals:
    void photoStorageDirChanged();
    void photoBackupDirChanged();
    void dirConfigChanged();
    void cameraTZChanged();
    void importTZChanged();
    void organizeTZChanged();
    void organizeCaptureDateChanged();
    void organizeRatingChanged();
    void maxOrganizeRatingChanged();
    void uiScaleChanged();
    void enqueueChanged();
    void appendHashChanged();
    void mipmapViewChanged();
    void lowMemModeChanged();
    void quickPreviewChanged();
    void previewResolutionChanged();
    void lensfunStatusChanged();
};

#endif // SETTINGS_H
