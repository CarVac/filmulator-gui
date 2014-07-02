#ifndef PARAMETERMANAGER_H
#define PARAMETERMANAGER_H

#include <QSqlQuery>
#include "../core/imagePipeline.h"

class ParameterManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString filename MEMBER m_filename WRITE setFilename NOTIFY filenameChanged)
    Q_PROPERTY(bool tiffIn MEMBER m_tiffIn WRITE setTiffIn NOTIFY tiffInChanged)
    Q_PROPERTY(bool jpegIn MEMBER m_jpegIn WRITE setJpegIn NOTIFY jpegInChanged)
    Q_PROPERTY(bool caEnabled MEMBER m_caEnabled WRITE setCaEnabled NOTIFY caEnabledChanged)
    Q_PROPERTY(int highlights MEMBER m_highlights WRITE setHighlights NOTIFY highlightsChanged)
    Q_PROPERTY(float exposureComp MEMBER m_exposureComp WRITE setExposureComp NOTIFY exposureCompChanged)

};

#endif // PARAMETERMANAGER_H
