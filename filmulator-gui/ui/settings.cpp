#include "settings.h"
#include <iostream>

using namespace std;

Settings::Settings( QObject *parent ) :
    QObject( parent )
{
    QSettings settings( QSettings::UserScope, QString( "Filmulator" ) );
    photoStorageDir = settings.value( "photoDB/photoStorageDir", "" ).toString() ;
    photoBackupDir = settings.value( "photoDB/photoBackupDir", "" ).toString();
    dirConfig = settings.value( "photoDB/dirConfig", "/yyyy/MM/yyyy-MM-dd/" ).toString();
    cameraTZ = settings.value( "photoDB/cameraTZ", 0 ).toInt();
    importTZ = settings.value( "photoDB/importTZ", 0 ).toInt();
}

void Settings::setPhotoStorageDir( QString dirIn )
{
    QSettings settings( QSettings::UserScope, QString( "Filmulator" ) );
    photoStorageDir = settings.value( "photoDB/photoStorageDir", "" ).toString() ;
    photoStorageDir = dirIn;
    settings.setValue( "photoDB/photoStorageDir", dirIn );
    emit photoStorageDirChanged();
}

QString Settings::getPhotoStorageDir()
{
    QSettings settings( QSettings::UserScope, QString( "Filmulator" ) );
    photoStorageDir = settings.value( "photoDB/photoStorageDir", "" ).toString() ;
    photoStorageDir = settings.value( "photoDB/photoStorageDir", "" ).toString() ;
    emit photoStorageDirChanged();
    return photoStorageDir;
}

void Settings::setPhotoBackupDir( QString dirIn )
{
    QSettings settings( QSettings::UserScope, QString( "Filmulator" ) );
    photoStorageDir = settings.value( "photoDB/photoStorageDir", "" ).toString() ;
    photoBackupDir = dirIn;
    settings.setValue( "photoDB/photoBackupDir", dirIn );
    emit photoBackupDirChanged();
}

QString Settings::getPhotoBackupDir()
{
    QSettings settings( QSettings::UserScope, QString( "Filmulator" ) );
    photoStorageDir = settings.value( "photoDB/photoStorageDir", "" ).toString() ;
    photoBackupDir = settings.value( "photoDB/photoBackupDir", "" ).toString();
    emit photoBackupDirChanged();
    return photoBackupDir;
}

void Settings::setDirConfig( QString configIn )
{
    QSettings settings( QSettings::UserScope, QString( "Filmulator" ) );
    photoStorageDir = settings.value( "photoDB/photoStorageDir", "" ).toString() ;
    dirConfig = configIn;
    settings.setValue( "photoDB/dirConfig", configIn );
    emit dirConfigChanged();
}

QString Settings::getDirConfig()
{
    QSettings settings( QSettings::UserScope, QString( "Filmulator" ) );
    photoStorageDir = settings.value( "photoDB/photoStorageDir", "" ).toString() ;
    dirConfig = settings.value( "photoDB/dirConfig", "/yyyy/MM/yyyy-MM-dd/" ).toString();
    emit dirConfigChanged();
    return dirConfig;
}

void Settings::setCameraTZ( int offsetIn )
{
    QSettings settings( QSettings::UserScope, QString( "Filmulator" ) );
    photoStorageDir = settings.value( "photoDB/photoStorageDir", "" ).toString() ;
    cameraTZ = offsetIn;
    settings.setValue( "photoDB/cameraTZ", offsetIn );
    emit importTZChanged();
    settings.sync();
}

int Settings::getCameraTZ()
{
    QSettings settings( QSettings::UserScope, QString( "Filmulator" ) );
    photoStorageDir = settings.value( "photoDB/photoStorageDir", "" ).toString() ;
    cameraTZ = settings.value( "photoDB/cameraTZ", 0 ).toInt();
    emit cameraTZChanged();
    return cameraTZ;
}

void Settings::setImportTZ( int offsetIn )
{
    QSettings settings( QSettings::UserScope, QString( "Filmulator" ) );
    photoStorageDir = settings.value( "photoDB/photoStorageDir", "" ).toString() ;
    importTZ = offsetIn;
    settings.setValue( "photoDB/importTZ", offsetIn );
    emit importTZChanged();
}

int Settings::getImportTZ()
{
    QSettings settings( QSettings::UserScope, QString( "Filmulator" ) );
    photoStorageDir = settings.value( "photoDB/photoStorageDir", "" ).toString() ;
    importTZ = settings.value( "photoDB/importTZ", 0 ).toInt();
    emit importTZChanged();
    return importTZ;
}
