#ifndef PARAMETERMANAGER_H
#define PARAMETERMANAGER_H

#include <QSqlQuery>
#include "../core/imagePipeline.h"
#include <QMutex>
#include <QMutexLocker>

class ParameterManager : public QObject
{
    Q_OBJECT
    //Loading
    Q_PROPERTY(QString filename MEMBER m_filename WRITE setFilename NOTIFY filenameChanged)
    Q_PROPERTY(bool tiffIn MEMBER m_tiffIn WRITE setTiffIn NOTIFY tiffInChanged)
    Q_PROPERTY(bool jpegIn MEMBER m_jpegIn WRITE setJpegIn NOTIFY jpegInChanged)

    //Demosaic
    Q_PROPERTY(bool caEnabled MEMBER m_caEnabled WRITE setCaEnabled NOTIFY caEnabledChanged)
    Q_PROPERTY(int highlights MEMBER m_highlights WRITE setHighlights NOTIFY highlightsChanged)

    //Prefilmulation
    Q_PROPERTY(float exposureComp MEMBER m_exposureComp WRITE setExposureComp NOTIFY exposureCompChanged)
    Q_PROPERTY(double temperature MEMBER m_temperature WRITE setTemperature NOTIFY temperatureChanged)
    Q_PROPERTY(double tint MEMBER m_tint WRITE setTint NOTIFY tintChanged)

    //Filmulation
    Q_PROPERTY(float initialDeveloperConcentration MEMBER m_initialDeveloperConcentration WRITE setInitialDeveloperConcentration NOTIFY initialDeveloperConcentrationChanged)
    Q_PROPERTY(float reservoirThickness MEMBER m_reservoirThickness WRITE setReservoirThickness NOTIFY reservoirThicknessChanged)
    Q_PROPERTY(float activeLayerThickness MEMBER m_activeLayerThickness WRITE setActiveLayerThickness NOTIFY activeLayerThicknessChanged)
    Q_PROPERTY(float crystalsPerPixel MEMBER m_crystalsPerPixel WRITE setCrystalsPerPixel NOTIFY crystalsPerPixelChanged)
    Q_PROPERTY(float initialCrystalRadius MEMBER m_initialCrystalRadius WRITE setInitialCrystalRadius NOTIFY initialCrystalRadiusChanged)
    Q_PROPERTY(float developerConsumptionConst MEMBER m_developerConsumptionConst WRITE setDeveloperConsumptionConst NOTIFY developerConsumptionConstChanged)
    Q_PROPERTY(float crystalGrowthConst MEMBER m_crystalGrowthConst WRITE setCrystalGrowthConst NOTIFY crystalGrowthConstChanged)
    Q_PROPERTY(float silverSaltConsumptionConst MEMBER m_silverSaltConsumptionConst WRITE setSilverSaltConsumptionConst NOTIFY silverSaltConsumptionConstChanged)
    Q_PROPERTY(float totalDevelTime MEMBER m_totalDevelTime WRITE setTotalDevelTime NOTIFY totalDevelTimeChanged)
    Q_PROPERTY(int agitateCount MEMBER m_agitateCount WRITE setAgitateCount NOTIFY agitateCountChanged)
    Q_PROPERTY(int developmentSteps MEMBER m_developmentSteps WRITE setDevelopmentSteps NOTIFY developmentStepsChanged)
    Q_PROPERTY(float filmArea MEMBER m_filmArea WRITE setFilmArea NOTIFY filmAreaChanged)
    Q_PROPERTY(float sigmaConst MEMBER m_sigmaConst WRITE setSigmaConst NOTIFY sigmaConstChanged)
    Q_PROPERTY(float layerMixConst MEMBER m_layerMixConst WRITE setLayerMixConst NOTIFY layerMixConstChanged)
    Q_PROPERTY(float layerTimeDivisor MEMBER m_layerTimeDivisor WRITE setLayerTimeDivisor NOTIFY layerTimeDivisorChanged)
    Q_PROPERTY(int rolloffBoundary MEMBER m_rolloffBoundary WRITE setRolloffBoundary NOTIFY rolloffBoundaryChanged)

    //Whitepoint & Blackpoint
    Q_PROPERTY(float blackpoint MEMBER m_blackpoint WRITE setBlackpoint NOTIFY blackpointChanged)
    Q_PROPERTY(float whitepoint MEMBER m_whitepoint WRITE setWhitepoint NOTIFY whitepointChanged)

    //Global, all-color curves.
    Q_PROPERTY(float shadowsX MEMBER m_shadowsX WRITE setShadowsX NOTIFY shadowsXChanged)
    Q_PROPERTY(float shadowsY MEMBER m_shadowsY WRITE setShadowsY NOTIFY shadowsYChanged)
    Q_PROPERTY(float highlightsX MEMBER m_highlightsX WRITE setHighlightsX NOTIFY highlightsXChanged)
    Q_PROPERTY(float highlightsY MEMBER m_highlightsY WRITE setHighlightsY NOTIFY highlightsYChanged)
    Q_PROPERTY(float vibrance MEMBER m_vibrance WRITE setVibrance NOTIFY vibranceChanged)
    Q_PROPERTY(float saturation MEMBER m_saturation WRITE setSaturation NOTIFY saturationChanged)

    //Rotation
    Q_PROPERTY(int rotation MEMBER m_rotation WRITE setRotation NOTIFY rotationChanged)

public:
    ParameterManager();
    ProcessingParameters getParams();

    Q_INVOKABLE void rotateRight();
    Q_INVOKABLE void rotateLeft();

    Q_INVOKABLE void selectImage(QString);

protected:
    ProcessingParameters param;
    QMutex mutex;

    QString imageIndex;

    //Helper for setting up SQL queries.
    QString queryHelper(QString imageID, QString columnName);

    //Variables for the properties.
    //Loading
    QString m_filename;
    bool m_tiffIn;
    bool m_jpegIn;

    //Demosaic
    bool m_caEnabled;
    int m_highlights;

    //Prefilmulation
    float m_exposureComp;
    double m_temperature;
    double m_tint;

    //Filmulation
    float m_initialDeveloperConcentration;
    float m_reservoirThickness;
    float m_activeLayerThickness;
    float m_crystalsPerPixel;
    float m_initialCrystalRadius;
    float m_developerConsumptionConst;
    float m_crystalGrowthConst;
    float m_silverSaltConsumptionConst;
    float m_totalDevelTime;
    int m_agitateCount;
    int m_developmentSteps;
    float m_filmArea;
    float m_sigmaConst;
    float m_layerMixConst;
    float m_layerTimeDivisor;
    int m_rolloffBoundary;

    //Whitepoint & Blackpoint
    float m_blackpoint;
    float m_whitepoint;

    //Global, all-color curves.
    float m_shadowsX;
    float m_shadowsY;
    float m_highlightsX;
    float m_highlightsY;
    float m_vibrance;
    float m_saturation;

    //Rotation
    int m_rotation;

    //Setters for the properties.
    //Loading
    void setFilename(QString);
    void setTiffIn(bool);
    void setJpegIn(bool);

    //Demosaic
    void setCaEnabled(bool);
    void setHighlights(int);

    //Prefilmulation
    void setExposureComp(float);
    void setTemperature(double);
    void setTint(double);

    //Filmulation
    void setInitialDeveloperConcentration(float);
    void setReservoirThickness(float);
    void setActiveLayerThickness(float);
    void setCrystalsPerPixel(float);
    void setInitialCrystalRadius(float);
    void setDeveloperConsumptionConst(float);
    void setCrystalGrowthConst(float);
    void setSilverSaltConsumptionConst(float);
    void setTotalDevelTime(float);
    void setAgitateCount(int);
    void setDevelopmentSteps(int);
    void setFilmArea(float);
    void setSigmaConst(float);
    void setLayerMixConst(float);
    void setLayerTimeDivisor(float);
    void setRolloffBoundary(int);

    //Whitepoint & Blackpoint
    void setBlackpoint(float);
    void setWhitepoint(float);

    //Global, all-color curves.
    void setShadowsX(float);
    void setShadowsY(float);
    void setHighlightsX(float);
    void setHighlightsY(float);
    void setVibrance(float);
    void setSaturation(float);

    //Rotation
    void setRotation(int);

signals:
    //Loading
    void filenameChanged();
    void tiffInChanged();
    void jpegInChanged();

    //Demosaic
    void caEnabledChanged();
    void highlightsChanged();

    //Prefilmulation
    void exposureCompChanged();
    void temperatureChanged();
    void tintChanged();

    //Filmulation
    void initialDeveloperConcentrationChanged();
    void reservoirThicknessChanged();
    void activeLayerThicknessChanged();
    void crystalsPerPixelChanged();
    void initialCrystalRadiusChanged();
    void developerConsumptionConstChanged();
    void crystalGrowthConstChanged();
    void silverSaltConsumptionConstChanged();
    void totalDevelTimeChanged();
    void agitateCountChanged();
    void developmentStepsChanged();
    void filmAreaChanged();
    void sigmaConstChanged();
    void layerMixConstChanged();
    void layerTimeDivisorChanged();
    void rolloffBoundaryChanged();

    //Whitepoint & Blackpoint
    void blackpointChanged();
    void whitepointChanged();

    //Global, all-color curves.
    void shadowsXChanged();
    void shadowsYChanged();
    void highlightsXChanged();
    void highlightsYChanged();
    void vibranceChanged();
    void saturationChanged();

    //Rotation
    void rotationChanged();

    //General: if any param changes, emit this one as well after the param-specific signal.
    void paramChanged();
};

#endif // PARAMETERMANAGER_H
