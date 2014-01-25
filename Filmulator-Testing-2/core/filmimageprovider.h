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
    Q_INVOKABLE void setWhitepoint(float whitepointIn);
    Q_INVOKABLE void invalidateInputImage();
    Q_INVOKABLE void invalidateFilmulation();
protected:
    float exposurecomp;
    matrix<float> input_image_cache;
    matrix<float> filmulated_image_cache;
    bool input_image_valid;
    bool filmulated_image_valid;
    filmulateParams filmParams;
    float whitepoint;

public slots:
};

#endif // FILMIMAGEPROVIDER_H
