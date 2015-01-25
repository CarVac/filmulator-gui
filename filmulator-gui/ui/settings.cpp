#include "settings.h"
#include <iostream>

using namespace std;

Settings::Settings(QObject *parent) :
    QObject(parent)
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    photoStorageDir = settings.value("photoDB/photoStorageDir", "").toString() ;
    photoBackupDir = settings.value("photoDB/photoBackupDir", "").toString();
    dirConfig = settings.value("photoDB/dirConfig", "/yyyy/MM/yyyy-MM-dd/").toString();
    cameraTZ = settings.value("photoDB/cameraTZ", 0).toInt();
    importTZ = settings.value("photoDB/importTZ", 0).toInt();
}

void Settings::setPhotoStorageDir(QString dirIn)
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    photoStorageDir = dirIn;
    settings.setValue("photoDB/photoStorageDir", dirIn);
    emit photoStorageDirChanged();
}

QString Settings::getPhotoStorageDir()
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    photoStorageDir = settings.value("photoDB/photoStorageDir", "").toString();
    emit photoStorageDirChanged();
    return photoStorageDir;
}

void Settings::setPhotoBackupDir(QString dirIn)
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    photoBackupDir = dirIn;
    settings.setValue("photoDB/photoBackupDir", dirIn);
    emit photoBackupDirChanged();
}

QString Settings::getPhotoBackupDir()
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    photoBackupDir = settings.value("photoDB/photoBackupDir", "").toString();
    emit photoBackupDirChanged();
    return photoBackupDir;
}

void Settings::setDirConfig(QString configIn)
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    dirConfig = configIn;
    settings.setValue("photoDB/dirConfig", configIn);
    emit dirConfigChanged();
}

QString Settings::getDirConfig()
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    dirConfig = settings.value("photoDB/dirConfig", "/yyyy/MM/yyyy-MM-dd/").toString();
    emit dirConfigChanged();
    return dirConfig;
}

void Settings::setCameraTZ(int offsetIn)
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    cameraTZ = offsetIn;
    settings.setValue("photoDB/cameraTZ", offsetIn);
    emit importTZChanged();
}

int Settings::getCameraTZ()
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    cameraTZ = settings.value("photoDB/cameraTZ", 0).toInt();
    emit cameraTZChanged();
    return cameraTZ;
}

void Settings::setImportTZ(int offsetIn)
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    importTZ = offsetIn;
    settings.setValue("photoDB/importTZ", offsetIn);
    emit importTZChanged();
}

int Settings::getImportTZ()
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    importTZ = settings.value("photoDB/importTZ", 0).toInt();
    emit importTZChanged();
    return importTZ;
}

void Settings::setOrganizeTZ(int offsetIn)
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    organizeTZ = offsetIn;
    settings.setValue("photoDB/organizeTZ", offsetIn);
    emit organizeTZChanged();
}

int Settings::getOrganizeTZ()
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    organizeTZ = settings.value("photoDB/organizeTZ", 0).toInt();
    emit organizeTZChanged();
    return organizeTZ;
}

void Settings::setOrganizeCaptureDate(QDate dateIn)
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    organizeCaptureDate = dateIn;
    settings.setValue("photoDB/organizeCaptureDate", dateIn);
    emit organizeCaptureDateChanged();
}

QDate Settings::getOrganizeCaptureDate()
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    organizeCaptureDate = settings.value("photoDB/organizeCaptureDate", QVariant(QDate::currentDate())).toDate();
    emit organizeCaptureDateChanged();
    return organizeCaptureDate;
}

void Settings::setUiScale(float uiScaleIn)
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    uiScale = uiScaleIn;
    settings.setValue("ui/uiScale", uiScaleIn);
    emit uiScaleChanged();
}

float Settings::getUiScale()
{
    QSettings settings(QSettings::UserScope, QString("Filmulator"));
    uiScale = settings.value("ui/uiScale", 1).toFloat();
    //emit uiScaleChanged();
    return uiScale;
}
