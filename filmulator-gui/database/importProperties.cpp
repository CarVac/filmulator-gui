#include "importModel.h"
#include <iostream>

//offsetIn is in hours, importTZ is in seconds.
void ImportModel::setImportTZ( int offsetIn )
{
    importTZ = offsetIn*3600;
//    std::cout << "importTZ: " << importTZ << endl;
    emit importTZChanged();
}

void ImportModel::setCameraTZ( int offsetIn )
{
    cameraTZ = offsetIn*3600;
//    std::cout << "cameraTZ: " <<  cameraTZ << endl;
    emit cameraTZChanged();
}

void ImportModel::setPhotoDir( QString dirIn )
{
    photoDir = dirIn;
    emit photoDirChanged();
}

void ImportModel::setBackupDir( QString dirIn )
{
    backupDir = dirIn;
    emit backupDirChanged();
}

void ImportModel::setDirConfig( QString configIn )
{
    dirConfig = configIn;
    if ( !dirConfig.endsWith( "/" ) )
    {
        dirConfig.append( "/" );
    }
    emit dirConfigChanged();
}
