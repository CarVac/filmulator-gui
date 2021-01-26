#include "settings.h"
#include <iostream>

//for lensfun
#include "../database/rawproc_lensfun/lensfun_dbupdate.h"
#include "../database/camconst.h"
#include <QDir>
#include <QStandardPaths>

using namespace std;

Settings::Settings(QObject *parent) :
    QObject(parent)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    photoStorageDir = settings.value("photoDB/photoStorageDir", "").toString() ;
    photoBackupDir = settings.value("photoDB/photoBackupDir", "").toString();
    dirConfig = settings.value("photoDB/dirConfig", "/yyyy/MM/yyyy-MM-dd/").toString();
    cameraTZ = settings.value("photoDB/cameraTZ", 0).toInt();
    importTZ = settings.value("photoDB/importTZ", 0).toInt();
    lensfunStatus = "";
    updateStatus = "";
    camconstDlStatus = "";
}

void Settings::setPhotoStorageDir(QString dirIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    photoStorageDir = dirIn;
    settings.setValue("photoDB/photoStorageDir", dirIn);
    emit photoStorageDirChanged();
}

QString Settings::getPhotoStorageDir()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    photoStorageDir = settings.value("photoDB/photoStorageDir", "").toString();
    emit photoStorageDirChanged();
    return photoStorageDir;
}

void Settings::setPhotoBackupDir(QString dirIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    photoBackupDir = dirIn;
    settings.setValue("photoDB/photoBackupDir", dirIn);
    emit photoBackupDirChanged();
}

QString Settings::getPhotoBackupDir()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    photoBackupDir = settings.value("photoDB/photoBackupDir", "").toString();
    emit photoBackupDirChanged();
    return photoBackupDir;
}

void Settings::setDirConfig(QString configIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    dirConfig = configIn;
    settings.setValue("photoDB/dirConfig", configIn);
    emit dirConfigChanged();
}

QString Settings::getDirConfig()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    dirConfig = settings.value("photoDB/dirConfig", "/yyyy/MM/yyyy-MM-dd/").toString();
    emit dirConfigChanged();
    return dirConfig;
}

void Settings::setCameraTZ(int offsetIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    cameraTZ = offsetIn;
    settings.setValue("photoDB/cameraTZ", offsetIn);
    emit importTZChanged();
}

int Settings::getCameraTZ()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    cameraTZ = settings.value("photoDB/cameraTZ", 0).toInt();
    emit cameraTZChanged();
    return cameraTZ;
}

void Settings::setImportTZ(int offsetIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    importTZ = offsetIn;
    settings.setValue("photoDB/importTZ", offsetIn);
    emit importTZChanged();
}

int Settings::getImportTZ()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    importTZ = settings.value("photoDB/importTZ", 0).toInt();
    emit importTZChanged();
    return importTZ;
}

void Settings::setOrganizeTZ(int offsetIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    organizeTZ = offsetIn;
    settings.setValue("photoDB/organizeTZ", offsetIn);
    emit organizeTZChanged();
}

int Settings::getOrganizeTZ()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    organizeTZ = settings.value("photoDB/organizeTZ", 0).toInt();
    emit organizeTZChanged();
    return organizeTZ;
}

void Settings::setOrganizeCaptureDate(QDate dateIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    organizeCaptureDate = dateIn;
    settings.setValue("photoDB/organizeCaptureDate", dateIn);
    emit organizeCaptureDateChanged();
}

QDate Settings::getOrganizeCaptureDate()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    organizeCaptureDate = settings.value("photoDB/organizeCaptureDate", QVariant(QDate::currentDate())).toDate();

    //this was added to fix weird behavior, but is it necessary?
    //someone reported a bug where the calendar shifts the day forward.
    //And restarting the program would also shift the day forward.
    organizeCaptureDate = organizeCaptureDate.addDays(1);

    emit organizeCaptureDateChanged();
    return organizeCaptureDate;
}

void Settings::setOrganizeRating(int ratingIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    organizeRating = ratingIn;
    settings.setValue("photoDB/organizeRating", ratingIn);
    emit organizeRatingChanged();
}

int Settings::getOrganizeRating()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    organizeRating = settings.value("photoDB/organizeRating", -1).toInt();
    emit organizeRatingChanged();
    return organizeRating;
}

void Settings::setMaxOrganizeRating(int ratingIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    maxOrganizeRating = ratingIn;
    settings.setValue("photoDB/maxOrganizeRating", ratingIn);
    emit maxOrganizeRatingChanged();
}

int Settings::getMaxOrganizeRating()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    maxOrganizeRating = settings.value("photoDB/maxOrganizeRating", 5).toInt();
    emit maxOrganizeRatingChanged();
    return maxOrganizeRating;
}

void Settings::setUiScale(float uiScaleIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    uiScale = uiScaleIn;
    settings.setValue("ui/uiScale", uiScaleIn);
    emit uiScaleChanged();
}

float Settings::getUiScale()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    uiScale = settings.value("ui/uiScale", 1).toFloat();
    //emit uiScaleChanged();//This does weird things when it fails to propagate upstream.
    //Commenting it here makes it apply after a restart.
    return uiScale;
}

void Settings::setEnqueue(bool enqueueIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    enqueue = enqueueIn;
    settings.setValue("import/enqueueAsImported", enqueueIn);
    emit enqueueChanged();
}

bool Settings::getEnqueue()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    enqueue = settings.value("import/enqueueAsImported", 1).toBool();
    emit enqueueChanged();
    return enqueue;
}

void Settings::setAppendHash(bool appendHashIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    appendHash = appendHashIn;
    settings.setValue("import/appendHash", appendHashIn);
    emit enqueueChanged();
}

bool Settings::getAppendHash()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    appendHash = settings.value("import/appendHash", 1).toBool();
    emit appendHashChanged();
    return appendHash;
}

void Settings::setMipmapView(bool mipmapViewIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    mipmapView = mipmapViewIn;
    settings.setValue("edit/mipmapView", mipmapViewIn);
    emit mipmapViewChanged();
}

bool Settings::getMipmapView()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    mipmapView = settings.value("edit/mipmapView", 0).toBool();
    return mipmapView;
}

void Settings::setLowMemMode(bool lowMemModeIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    lowMemMode = lowMemModeIn;
    settings.setValue("edit/lowMemMode", lowMemModeIn);
    emit lowMemModeChanged();
}

bool Settings::getLowMemMode()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    //Default: 0
    lowMemMode = settings.value("edit/lowMemMode", 0).toBool();
    emit lowMemModeChanged();
    return lowMemMode;
}

void Settings::setQuickPreview(bool quickPreviewIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    quickPreview = quickPreviewIn;
    settings.setValue("edit/quickPreview", quickPreviewIn);
    emit quickPreviewChanged();
}

bool Settings::getQuickPreview()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    //Default: 1; it should default to being on.
    quickPreview = settings.value("edit/quickPreview", 1).toBool();
    emit quickPreviewChanged();
    return quickPreview;
}

void Settings::setPreviewResolution(int resolutionIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    previewResolution = resolutionIn;
    settings.setValue("edit/previewResolution", resolutionIn);
    emit previewResolutionChanged();
}

int Settings::getPreviewResolution()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    //Default: 1000 pixels wide
    previewResolution = settings.value("edit/previewResolution", 1500).toInt();
    emit previewResolutionChanged();
    return previewResolution;
}

void Settings::setUseSystemLanguage(bool useSystemLanguageIn)
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    useSystemLanguage = useSystemLanguageIn;
    settings.setValue("ui/useSystemLanguage", useSystemLanguageIn);
    emit useSystemLanguageChanged();
}

bool Settings::getUseSystemLanguage()
{
    QSettings settings(QSettings::UserScope, "Filmulator", "Filmulator");
    //Default: 1; it should default to being on.
    useSystemLanguage = settings.value("ui/useSystemLanguage", 1).toBool();
    emit useSystemLanguageChanged();
    return useSystemLanguage;
}

void Settings::checkLensfunStatus()
{
    QDir dir = QDir::home();
    QString dirstr = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    dirstr.append("/filmulator");

    lensfunStatus = tr("Checking database version", "lensfun database check");
    emit lensfunStatusChanged();

    lf_db_return dbStatus = lensfun_dbcheck(2, dirstr.toStdString());

    if (dbStatus == LENSFUN_DBUPDATE_NOVERSION)
    {
        lensfunStatus = tr("Database unavailable from server.", "lensfun database check");
        emit lensfunStatusChanged();
    }
    if (dbStatus == LENSFUN_DBUPDATE_NODATABASE)
    {
        lensfunStatus = tr("No local database yet.", "lensfun database check");
        emit lensfunStatusChanged();
    }
    if (dbStatus == LENSFUN_DBUPDATE_OLDVERSION)
    {
        lensfunStatus = tr("Updated database available.", "lensfun database check");
        emit lensfunStatusChanged();
    }
    if (dbStatus == LENSFUN_DBUPDATE_CURRENTVERSION)
    {
        lensfunStatus = tr("Database is up to date.", "lensfun database check");
        emit lensfunStatusChanged();
    }
}
void Settings::updateLensfun()
{
    QDir dir = QDir::home();
    QString dirstr = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    dirstr.append("/filmulator");

    updateStatus = tr("Updating database", "lensfun database update");
    emit updateStatusChanged();

    lf_db_return dbStatus = lensfun_dbupdate(2, dirstr.toStdString());

    if (dbStatus == LENSFUN_DBUPDATE_OK)
    {
        updateStatus = tr("Success. Please restart.", "lensfun database update");
        emit updateStatusChanged();
    }
    if (dbStatus == LENSFUN_DBUPDATE_CURRENTVERSION)
    {
        updateStatus = tr("Database is up to date.", "lensfun database update");
        emit updateStatusChanged();
    }
    if (dbStatus == LENSFUN_DBUPDATE_NOVERSION)
    {
        updateStatus = tr("No database available from server.", "lensfun database update");
        emit updateStatusChanged();
    }
    if (dbStatus == LENSFUN_DBUPDATE_RETRIEVE_INITFAILED)
    {
        updateStatus = tr("Database retrieve failed (init).", "lensfun database update");
        emit updateStatusChanged();
    }
    if (dbStatus == LENSFUN_DBUPDATE_RETRIEVE_FILEOPENFAILED)
    {
        updateStatus = tr("Database retrieve failed (file).", "lensfun database update");
        emit updateStatusChanged();
    }
    if (dbStatus == LENSFUN_DBUPDATE_RETRIEVE_RETRIEVEFAILED)
    {
        updateStatus = tr("Database retrieve failed (retrieve).", "lensfun database update");
        emit updateStatusChanged();
    }
}

void Settings::downloadCamConst()
{
    camconst_status status = camconst_download();
    if (status == CAMCONST_DL_OK)
    {
        camconstDlStatus = tr("camconst.json download succeeded.");
        emit camconstDlStatusChanged();
    }
    if (status == CAMCONST_DL_INITFAILED)
    {
        camconstDlStatus = tr("camconst.json init failed.");
        emit camconstDlStatusChanged();
    }
    if (status == CAMCONST_DL_FOPENFAILED)
    {
        camconstDlStatus = tr("camconst.json fopen failed.");
        emit camconstDlStatusChanged();
    }
    if (status == CAMCONST_DL_RETRIEVEFAILED)
    {
        camconstDlStatus = tr("CamConst retrieve failed.");
        emit camconstDlStatusChanged();
    }
}
