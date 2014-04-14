#ifndef ORGANIZEMODEL_H
#define ORGANIZEMODEL_H

#include "sqlmodel.h"
#include <QString>
#include <QByteArray>
#include <exiv2/exiv2.hpp>
using namespace std;

class OrganizeModel : public SqlModel
{
    Q_OBJECT
    Q_PROPERTY( unsigned int minCaptureTime   READ getMinCaptureTime   WRITE setMinCaptureTime   NOTIFY minCaptureTimeChanged )
    Q_PROPERTY( unsigned int maxCaptureTime   READ getMaxCaptureTime   WRITE setMaxCaptureTime   NOTIFY maxCaptureTimeChanged )
    Q_PROPERTY( unsigned int minImportTime    READ getMinImportTime    WRITE setMinImportTime    NOTIFY minImportTimeChanged )
    Q_PROPERTY( unsigned int maxImportTime    READ getMaxImportTime    WRITE setMaxImportTime    NOTIFY maxImportTimeChanged )
    Q_PROPERTY( unsigned int minProcessedTime READ getMinProcessedTime WRITE setMinProcessedTime NOTIFY minProcessedTimeChanged )
    Q_PROPERTY( unsigned int maxProcessedTime READ getMaxProcessedTime WRITE setMaxProcessedTime NOTIFY maxProcessedTimeChanged )
    Q_PROPERTY( int minRating     READ getMinRating     WRITE setMinRating     NOTIFY minRatingChanged )
    Q_PROPERTY( int maxRating     READ getMaxRating     WRITE setMaxRating     NOTIFY maxRatingChanged )
    Q_PROPERTY( int captureSort   READ getCaptureSort   WRITE setCaptureSort   NOTIFY captureSortChanged )
    Q_PROPERTY( int importSort    READ getImportSort    WRITE setImportSort    NOTIFY importSortChanged )
    Q_PROPERTY( int processedSort READ getProcessedSort WRITE setProcessedSort NOTIFY processedSortChanged )
    Q_PROPERTY( int ratingSort    READ getRatingSort    WRITE setRatingSort    NOTIFY ratingSortChanged )
    Q_PROPERTY( int importTZ      READ getImportTZ      WRITE setImportTZ      NOTIFY importTZChanged )
    Q_PROPERTY( int cameraTZ      READ getCameraTZ      WRITE setCameraTZ      NOTIFY cameraTZChanged )
    Q_PROPERTY( QString photoDir  READ getPhotoDir      WRITE setPhotoDir      NOTIFY photoDirChanged )
    Q_PROPERTY( QString dirConfig READ getDirConfig     WRITE setDirConfig     NOTIFY dirConfigChanged )

public:
    explicit OrganizeModel( QObject *parent = 0 );
    Q_INVOKABLE void setOrganizeQuery();
    Q_INVOKABLE void importDirectory_r( QString dir );

    void setMinCaptureTime( unsigned int captureTimeIn );
    void setMaxCaptureTime( unsigned int captureTimeIn );
    void setMinImportTime( unsigned int importTimeIn );
    void setMaxImportTime( unsigned int importTimeIn );
    void setMinProcessedTime( unsigned int processedTimeIn );
    void setMaxProcessedTime( unsigned int processedTimeIn );
    void setMinRating( int ratingIn );
    void setMaxRating( int ratingIn);

    void setCaptureSort( int sortMode );
    void setImportSort( int sortMode );
    void setProcessedSort( int sortMode );
    void setRatingSort( int sortMode );

    void setImportTZ( int offsetIn );
    void setCameraTZ( int offsetIn );

    void setPhotoDir( QString dirIn );
    void setDirConfig( QString configIn );

    unsigned int getMinCaptureTime() { return minCaptureTime; }
    unsigned int getMaxCaptureTime() { return maxCaptureTime; }
    unsigned int getMinImportTime() { return minImportTime; }
    unsigned int getMaxImportTime() { return maxImportTime; }
    unsigned int getMinProcessedTime() { return minProcessedTime; }
    unsigned int getMaxProcessedTime() { return maxProcessedTime; }
    int getMinRating() { return minRating; }
    int getMaxRating() { return maxRating; }

    int getCaptureSort() { return captureSort; }
    int getImportSort() { return importSort; }
    int getProcessedSort() { return processedSort; }
    int getRatingSort() { return ratingSort; }

    int getImportTZ() { return importTZ/3600; }
    int getCameraTZ() { return cameraTZ/3600; }

    QString getPhotoDir() { return photoDir; }
    QString getDirConfig() { return dirConfig; }

signals:
    void minCaptureTimeChanged();
    void maxCaptureTimeChanged();
    void minImportTimeChanged();
    void maxImportTimeChanged();
    void minProcessedTimeChanged();
    void maxProcessedTimeChanged();
    void minRatingChanged();
    void maxRatingChanged();

    void captureSortChanged();
    void importSortChanged();
    void processedSortChanged();
    void ratingSortChanged();

    void importTZChanged();
    void cameraTZChanged();

    void photoDirChanged();
    void dirConfigChanged();

public slots:

protected:
    unsigned int minCaptureTime;
    unsigned int maxCaptureTime;
    unsigned int minImportTime;
    unsigned int maxImportTime;
    unsigned int minProcessedTime;
    unsigned int maxProcessedTime;
    int minRating;
    int maxRating;

    // For these sort variables, -1 means descending, +1 means ascending, 0 means inactive.
    int captureSort;
    int importSort;
    int processedSort;
    int ratingSort;

    int importTZ;
    int cameraTZ;

    QString photoDir;
    QString dirConfig;

    void fileInsert( const QString hash,
                     const QString filePathName,
                     Exiv2::ExifData exifData);
    void createNewProfile( const QString fileHash,
                           const QString fileName,
                           const int captureTime,
                           Exiv2::ExifData exifData);

};


#endif // ORGANIZEMODEL_H
