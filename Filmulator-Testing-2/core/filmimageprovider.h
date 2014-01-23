#ifndef FILMIMAGEPROVIDER_H
#define FILMIMAGEPROVIDER_H

#include <QObject>
#include <QColor>
#include <QImage>
#include <QQuickImageProvider>
#include "filmsim.hpp"

class FilmImageProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    FilmImageProvider(QQuickImageProvider::ImageType type);
    FilmImageProvider();
    ~FilmImageProvider();
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize);
    Q_INVOKABLE void setExposureComp(float exposure);
protected:
    float exposurecomp;

public slots:
};


#endif // FILMIMAGEPROVIDER_H
