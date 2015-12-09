#include "importModel.h"
#include <iostream>

//offsetIn is in hours, importTZ is in seconds.
void ImportModel::setImportTZ(const int offsetIn)
{
    importTZ = offsetIn*3600;
    emit importTZChanged();
}

void ImportModel::setCameraTZ(const int offsetIn)
{
    cameraTZ = offsetIn*3600;
    emit cameraTZChanged();
}

void ImportModel::setPhotoDir(const QString dirIn)
{
    photoDir = dirIn;
    emit photoDirChanged();
}

void ImportModel::setBackupDir(const QString dirIn)
{
    backupDir = dirIn;
    emit backupDirChanged();
}

void ImportModel::setDirConfig(const QString configIn)
{
    dirConfig = configIn;
    if (!dirConfig.endsWith("/"))
    {
        dirConfig.append("/");
    }
    emit dirConfigChanged();
}

void ImportModel::setEnqueue(const bool enqueueIn)
{
    enqueue = enqueueIn;
    emit enqueueChanged();
}

void ImportModel::setAppendHash(const bool appendHashIn)
{
    appendHash = appendHashIn;
    emit appendHashChanged();
}
