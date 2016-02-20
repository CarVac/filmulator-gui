#ifndef THUMBWRITEWORKER_H
#define THUMBWRITEWORKER_H

#include <QObject>
#include "../core/imagePipeline.h"
#include <QMutex>
#include <QMutexLocker>
#include <QSqlQuery>

/*The ThumbWriteWorker is an object that lets the main pipeline write thumbnails
 * without blocking everything else.
 */

class ThumbWriteWorker : public QObject
{
    Q_OBJECT

public:
    explicit ThumbWriteWorker(QObject *parent = 0);
    void setImage(const matrix<unsigned short> imageIn,
                  Exiv2::ExifData dataIn);

public slots:
    bool writeThumb(QString searchID);

signals:
    void doneWritingThumb();

protected:
    matrix<unsigned short> image;
    Exiv2::ExifData exifData;
    QMutex dataMutex;
};

#endif // THUMBWRITEWORKER_H
