#ifndef FILMIMAGEPROVIDER_H
#define FILMIMAGEPROVIDER_H

#include <QColor>
#include <QImage>
#include <QQuickImageProvider>
#include "filmsim.hpp"

class FilmImageProvider : public QQuickImageProvider
{
public:
    FilmImageProvider(QQuickImageProvider::ImageType type);
    FilmImageProvider();
    ~FilmImageProvider();
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize);
};


#endif // FILMIMAGEPROVIDER_H
