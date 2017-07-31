#ifndef PARAMETERMANAGER_H
#define PARAMETERMANAGER_H

#include <QObject>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMutex>
#include <QMutexLocker>
#include <QDateTime>
#include <QString>
#include <QDebug>
#include <tuple>
#include <iostream>
#include "../core/filmSim.hpp"
#include "../database/exifFunctions.h"

enum Valid {none,
            load,
            demosaic,
            prefilmulation,
            filmulation,
            blackwhite,
            colorcurve,
            filmlikecurve,
            count};

enum FilmFetch {initial,
                subsequent};

enum AbortStatus {proceed,
                  restart};

enum CopyDefaults {loadToParams,
                   loadOnlyDefaults};

//We want a struct for each stage of the pipeline for validity.
struct LoadParams {
    std::string fullFilename;
    bool tiffIn;
    bool jpegIn;
};

struct DemosaicParams {
    bool caEnabled;
    int highlights;
};

struct PrefilmParams {
    float exposureComp;
    float temperature;
    float tint;
    std::string fullFilename;
};

struct FilmParams {
    float initialDeveloperConcentration;
    float reservoirThickness;
    float activeLayerThickness;
    float crystalsPerPixel;
    float initialCrystalRadius;
    float initialSilverSaltDensity;
    float developerConsumptionConst;
    float crystalGrowthConst;
    float silverSaltConsumptionConst;
    float totalDevelopmentTime;
    int agitateCount;
    int developmentSteps;
    float filmArea;
    float sigmaConst;
    float layerMixConst;
    float layerTimeDivisor;
    float rolloffBoundary;
};

struct BlackWhiteParams {
    float blackpoint;
    float whitepoint;
    float cropHeight;
    float cropAspect;
    float cropVoffset;
    float cropHoffset;
    int rotation;
};

struct FilmlikeCurvesParams {
    float shadowsX;
    float shadowsY;
    float highlightsX;
    float highlightsY;
    float vibrance;
    float saturation;
};

class ParameterManager : public QObject
{
    Q_OBJECT
    //Loading
    Q_PROPERTY(QString imageIndex   READ getImageIndex   NOTIFY imageIndexChanged)

    Q_PROPERTY(QString filename     READ getFilename     NOTIFY filenameChanged)
    Q_PROPERTY(int sensitivity      READ getSensitivity  NOTIFY sensitivityChanged)
    Q_PROPERTY(QString exposureTime READ getExposureTime NOTIFY exposureTimeChanged)
    Q_PROPERTY(QString aperture     READ getAperture     NOTIFY apertureChanged)
    Q_PROPERTY(float focalLength    READ getFocalLength  NOTIFY focalLengthChanged)

    Q_PROPERTY(bool tiffIn MEMBER m_tiffIn WRITE setTiffIn NOTIFY tiffInChanged)
    Q_PROPERTY(bool jpegIn MEMBER m_jpegIn WRITE setJpegIn NOTIFY jpegInChanged)

    //Demosaic
    Q_PROPERTY(bool caEnabled MEMBER m_caEnabled  WRITE setCaEnabled  NOTIFY caEnabledChanged)
    Q_PROPERTY(int highlights MEMBER m_highlights WRITE setHighlights NOTIFY highlightsChanged)

    Q_PROPERTY(bool defCaEnabled READ getDefCaEnabled NOTIFY defCaEnabledChanged)
    Q_PROPERTY(int defHighlights READ getDefHighlights   NOTIFY defHighlightsChanged)

    //Prefilmulation
    Q_PROPERTY(float exposureComp MEMBER m_exposureComp WRITE setExposureComp NOTIFY exposureCompChanged)
    Q_PROPERTY(float temperature  MEMBER m_temperature  WRITE setTemperature NOTIFY temperatureChanged)
    Q_PROPERTY(float tint         MEMBER m_tint         WRITE setTint NOTIFY tintChanged)

    Q_PROPERTY(float defExposureComp READ getDefExposureComp NOTIFY defExposureCompChanged)
    Q_PROPERTY(float defTemperature  READ getDefTemperature  NOTIFY defTemperatureChanged)
    Q_PROPERTY(float defTint         READ getDefTint         NOTIFY defTintChanged)

    //Filmulation
    Q_PROPERTY(float initialDeveloperConcentration MEMBER m_initialDeveloperConcentration WRITE setInitialDeveloperConcentration NOTIFY initialDeveloperConcentrationChanged)
    Q_PROPERTY(float reservoirThickness            MEMBER m_reservoirThickness            WRITE setReservoirThickness            NOTIFY reservoirThicknessChanged)
    Q_PROPERTY(float activeLayerThickness          MEMBER m_activeLayerThickness          WRITE setActiveLayerThickness          NOTIFY activeLayerThicknessChanged)
    Q_PROPERTY(float crystalsPerPixel              MEMBER m_crystalsPerPixel              WRITE setCrystalsPerPixel              NOTIFY crystalsPerPixelChanged)
    Q_PROPERTY(float initialCrystalRadius          MEMBER m_initialCrystalRadius          WRITE setInitialCrystalRadius          NOTIFY initialCrystalRadiusChanged)
    Q_PROPERTY(float initialSilverSaltDensity      MEMBER m_initialSilverSaltDensity      WRITE setInitialSilverSaltDensity      NOTIFY initialSilverSaltDensityChanged)
    Q_PROPERTY(float developerConsumptionConst     MEMBER m_developerConsumptionConst     WRITE setDeveloperConsumptionConst     NOTIFY developerConsumptionConstChanged)
    Q_PROPERTY(float crystalGrowthConst            MEMBER m_crystalGrowthConst            WRITE setCrystalGrowthConst            NOTIFY crystalGrowthConstChanged)
    Q_PROPERTY(float silverSaltConsumptionConst    MEMBER m_silverSaltConsumptionConst    WRITE setSilverSaltConsumptionConst    NOTIFY silverSaltConsumptionConstChanged)
    Q_PROPERTY(float totalDevelopmentTime          MEMBER m_totalDevelopmentTime          WRITE setTotalDevelopmentTime          NOTIFY totalDevelopmentTimeChanged)
    Q_PROPERTY(int   agitateCount                  MEMBER m_agitateCount                  WRITE setAgitateCount                  NOTIFY agitateCountChanged)
    Q_PROPERTY(int   developmentSteps              MEMBER m_developmentSteps              WRITE setDevelopmentSteps              NOTIFY developmentStepsChanged)
    Q_PROPERTY(float filmArea                      MEMBER m_filmArea                      WRITE setFilmArea                      NOTIFY filmAreaChanged)
    Q_PROPERTY(float sigmaConst                    MEMBER m_sigmaConst                    WRITE setSigmaConst                    NOTIFY sigmaConstChanged)
    Q_PROPERTY(float layerMixConst                 MEMBER m_layerMixConst                 WRITE setLayerMixConst                 NOTIFY layerMixConstChanged)
    Q_PROPERTY(float layerTimeDivisor              MEMBER m_layerTimeDivisor              WRITE setLayerTimeDivisor              NOTIFY layerTimeDivisorChanged)
    Q_PROPERTY(float rolloffBoundary               MEMBER m_rolloffBoundary               WRITE setRolloffBoundary               NOTIFY rolloffBoundaryChanged)

    Q_PROPERTY(float defInitialDeveloperConcentration READ getDefInitialDeveloperConcentration NOTIFY defInitialDeveloperConcentrationChanged)
    Q_PROPERTY(float defReservoirThickness            READ getDefReservoirThickness            NOTIFY defReservoirThicknessChanged)
    Q_PROPERTY(float defActiveLayerThickness          READ getDefActiveLayerThickness          NOTIFY defActiveLayerThicknessChanged)
    Q_PROPERTY(float defCrystalsPerPixel              READ getDefCrystalsPerPixel              NOTIFY defCrystalsPerPixelChanged)
    Q_PROPERTY(float defInitialCrystalRadius          READ getDefInitialCrystalRadius          NOTIFY defInitialCrystalRadiusChanged)
    Q_PROPERTY(float defInitialSilverSaltDensity      READ getDefInitialSilverSaltDensity      NOTIFY defInitialSilverSaltDensityChanged)
    Q_PROPERTY(float defDeveloperConsumptionConst     READ getDefDeveloperConsumptionConst     NOTIFY defDeveloperConsumptionConstChanged)
    Q_PROPERTY(float defCrystalGrowthConst            READ getDefCrystalGrowthConst            NOTIFY defCrystalGrowthConstChanged)
    Q_PROPERTY(float defSilverSaltConsumptionConst    READ getDefSilverSaltConsumptionConst    NOTIFY defSilverSaltConsumptionConstChanged)
    Q_PROPERTY(float defTotalDevelopmentTime          READ getDefTotalDevelopmentTime          NOTIFY defTotalDevelopmentTimeChanged)
    Q_PROPERTY(int   defAgitateCount                  READ getDefAgitateCount                  NOTIFY defAgitateCountChanged)
    Q_PROPERTY(int   defDevelopmentSteps              READ getDefDevelopmentSteps              NOTIFY defDevelopmentStepsChanged)
    Q_PROPERTY(float defFilmArea                      READ getDefFilmArea                      NOTIFY defFilmAreaChanged)
    Q_PROPERTY(float defSigmaConst                    READ getDefSigmaConst                    NOTIFY defSigmaConstChanged)
    Q_PROPERTY(float defLayerMixConst                 READ getDefLayerMixConst                 NOTIFY defLayerMixConstChanged)
    Q_PROPERTY(float defLayerTimeDivisor              READ getDefLayerTimeDivisor              NOTIFY defLayerTimeDivisorChanged)
    Q_PROPERTY(float defRolloffBoundary               READ getDefRolloffBoundary               NOTIFY defRolloffBoundaryChanged)

    //Whitepoint & Blackpoint
    Q_PROPERTY(float blackpoint  MEMBER m_blackpoint  WRITE setBlackpoint  NOTIFY blackpointChanged)
    Q_PROPERTY(float whitepoint  MEMBER m_whitepoint  WRITE setWhitepoint  NOTIFY whitepointChanged)
    Q_PROPERTY(float cropHeight  MEMBER m_cropHeight  WRITE setCropHeight  NOTIFY cropHeightChanged)
    Q_PROPERTY(float cropAspect  MEMBER m_cropAspect  WRITE setCropAspect  NOTIFY cropAspectChanged)
    Q_PROPERTY(float cropVoffset MEMBER m_cropVoffset WRITE setCropVoffset NOTIFY cropVoffsetChanged)
    Q_PROPERTY(float cropHoffset MEMBER m_cropHoffset WRITE setCropHoffset NOTIFY cropHoffsetChanged)
    Q_PROPERTY(int rotation      MEMBER m_rotation    WRITE setRotation    NOTIFY rotationChanged)

    Q_PROPERTY(float defBlackpoint READ getDefBlackpoint NOTIFY defBlackpointChanged)
    Q_PROPERTY(float defWhitepoint READ getDefWhitepoint NOTIFY defWhitepointChanged)
    //There are no per-image default crop parameters
    //They're all initialized to 0.
    Q_PROPERTY(int defRotation     READ getDefRotation   NOTIFY defRotationChanged)

    //Global, all-color curves.
    Q_PROPERTY(float shadowsX    MEMBER m_shadowsX     WRITE setShadowsX    NOTIFY shadowsXChanged)
    Q_PROPERTY(float shadowsY    MEMBER m_shadowsY     WRITE setShadowsY    NOTIFY shadowsYChanged)
    Q_PROPERTY(float highlightsX MEMBER m_highlightsX  WRITE setHighlightsX NOTIFY highlightsXChanged)
    Q_PROPERTY(float highlightsY MEMBER m_highlightsY  WRITE setHighlightsY NOTIFY highlightsYChanged)
    Q_PROPERTY(float vibrance    MEMBER m_vibrance     WRITE setVibrance    NOTIFY vibranceChanged)
    Q_PROPERTY(float saturation  MEMBER m_saturation   WRITE setSaturation  NOTIFY saturationChanged)

    Q_PROPERTY(float defShadowsX    READ getDefShadowsX    NOTIFY defShadowsXChanged)
    Q_PROPERTY(float defShadowsY    READ getDefShadowsY    NOTIFY defShadowsYChanged)
    Q_PROPERTY(float defHighlightsX READ getDefHighlightsX NOTIFY defHighlightsXChanged)
    Q_PROPERTY(float defHighlightsY READ getDefHighlightsY NOTIFY defHighlightsYChanged)
    Q_PROPERTY(float defVibrance    READ getDefVibrance    NOTIFY defVibranceChanged)
    Q_PROPERTY(float defSaturation  READ getDefSaturation  NOTIFY defSaturationChanged)

    Q_PROPERTY(bool pasteable READ getPasteable NOTIFY pasteableChanged)

public:
    ParameterManager();

    Q_INVOKABLE void rotateRight();
    Q_INVOKABLE void rotateLeft();

    Q_INVOKABLE void selectImage(const QString imageID);

    Q_INVOKABLE void writeback();

    Q_INVOKABLE void copyAll(QString fromImageID);
    Q_INVOKABLE void paste(QString toImageID);

    //Each stage creates its struct, checks validity, marks the validity to indicate it's begun,
    //and then returns the struct and the validity.
    //Input
    std::tuple<Valid,AbortStatus,LoadParams> claimLoadParams();

    //Demosaic
    std::tuple<Valid,AbortStatus,DemosaicParams> claimDemosaicParams();

    //Prefilmulation
    std::tuple<Valid,AbortStatus,PrefilmParams> claimPrefilmParams();

    //Filmulation
    std::tuple<Valid,AbortStatus,FilmParams> claimFilmParams(FilmFetch fetch);

    //Whitepoint & Blackpoint (and cropping and rotation and distortion)
    std::tuple<Valid,AbortStatus,BlackWhiteParams> claimBlackWhiteParams();

    //Global, all-color curves.
    std::tuple<Valid,AbortStatus,FilmlikeCurvesParams> claimFilmlikeCurvesParams();

    Valid getValid();
    std::string getFullFilename(){return m_fullFilename;}

protected:
    //This is here for the sql insertion to pull the values from.
    void loadParams(QString imageID);
    void loadDefaults(const CopyDefaults useDefaults, const std::string absFilePath);

    //The paramMutex exists to prevent race conditions between
    //changes in the parameters and changes in validity.
    QMutex paramMutex;
    QMutex signalMutex;

    QString imageIndex;
    QString copyFromImageIndex;
    bool pasteable;
    bool pasteSome;

    void writeToDB(QString imageID);
    void paramChangeWrapper(QString);
    void disableParamChange();
    void enableParamChange();

    bool paramChangeEnabled;

    //Variables for the properties.
    //Image parameters, read-only.
    QString filename;
    int sensitivity;
    QString exposureTime;
    QString aperture;
    float focalLength;

    Valid validity;

    //Input
    std::string m_fullFilename;
    bool m_tiffIn;
    bool m_jpegIn;

    //Demosaic
    bool m_caEnabled;
    int  m_highlights;

    bool d_caEnabled; //d_'s are for default values
    int  d_highlights;

    //Prefilmulation
    float m_exposureComp;
    float m_temperature;
    float m_tint;

    float d_exposureComp;
    float d_temperature;
    float d_tint;

    //Filmulation
    float m_initialDeveloperConcentration;
    float m_reservoirThickness;
    float m_activeLayerThickness;
    float m_crystalsPerPixel;
    float m_initialCrystalRadius;
    float m_initialSilverSaltDensity;
    float m_developerConsumptionConst;
    float m_crystalGrowthConst;
    float m_silverSaltConsumptionConst;
    float m_totalDevelopmentTime;
    int   m_agitateCount;
    int   m_developmentSteps;
    float m_filmArea;
    float m_sigmaConst;
    float m_layerMixConst;
    float m_layerTimeDivisor;
    float m_rolloffBoundary;

    float d_initialDeveloperConcentration;
    float d_reservoirThickness;
    float d_activeLayerThickness;
    float d_crystalsPerPixel;
    float d_initialCrystalRadius;
    float d_initialSilverSaltDensity;
    float d_developerConsumptionConst;
    float d_crystalGrowthConst;
    float d_silverSaltConsumptionConst;
    float d_totalDevelopmentTime;
    int   d_agitateCount;
    int   d_developmentSteps;
    float d_filmArea;
    float d_sigmaConst;
    float d_layerMixConst;
    float d_layerTimeDivisor;
    float d_rolloffBoundary;

    //Whitepoint & Blackpoint
    float m_blackpoint;
    float m_whitepoint;
    float m_cropHeight = 0;
    float m_cropAspect = 0;
    float m_cropVoffset = 0;
    float m_cropHoffset = 0;

    float d_blackpoint;
    float d_whitepoint;

    //Global, all-color curves.
    float m_shadowsX;
    float m_shadowsY;
    float m_highlightsX;
    float m_highlightsY;
    float m_vibrance;
    float m_saturation;

    float d_shadowsX;
    float d_shadowsY;
    float d_highlightsX;
    float d_highlightsY;
    float d_vibrance;
    float d_saturation;

    //Rotation
    int m_rotation;

    int d_rotation;

    //Getters for read-only properties.
    QString getImageIndex(){return imageIndex;}

    QString getFilename(){return filename;}
    int getSensitivity(){return sensitivity;}
    QString getExposureTime(){return exposureTime;}
    QString getAperture(){return aperture;}
    float getFocalLength(){return focalLength;}

    bool getPasteable(){return pasteable;}

    //Getters for the defaults
    //Demosaic
    bool getDefCaEnabled(){return d_caEnabled;}
    int  getDefHighlights(){return d_highlights;}

    //Prefilmulation
    float getDefExposureComp(){return d_exposureComp;}
    float getDefTemperature(){return d_temperature;}
    float getDefTint(){return d_tint;}

    //Filmulation
    float getDefInitialDeveloperConcentration(){return d_initialDeveloperConcentration;}
    float getDefReservoirThickness(){return d_reservoirThickness;}
    float getDefActiveLayerThickness(){return d_activeLayerThickness;}
    float getDefCrystalsPerPixel(){return d_crystalsPerPixel;}
    float getDefInitialCrystalRadius(){return d_initialCrystalRadius;}
    float getDefInitialSilverSaltDensity(){return d_initialSilverSaltDensity;}
    float getDefDeveloperConsumptionConst(){return d_developerConsumptionConst;}
    float getDefCrystalGrowthConst(){return d_crystalGrowthConst;}
    float getDefSilverSaltConsumptionConst(){return d_silverSaltConsumptionConst;}
    float getDefTotalDevelopmentTime(){return d_totalDevelopmentTime;}
    int   getDefAgitateCount(){return d_agitateCount;}
    int   getDefDevelopmentSteps(){return d_developmentSteps;}
    float getDefFilmArea(){return d_filmArea;}
    float getDefSigmaConst(){return d_sigmaConst;}
    float getDefLayerMixConst(){return d_layerMixConst;}
    float getDefLayerTimeDivisor(){return d_layerTimeDivisor;}
    float getDefRolloffBoundary(){return d_rolloffBoundary;}

    //Whitepoint & blackpoint
    float getDefBlackpoint(){return d_blackpoint;}
    float getDefWhitepoint(){return d_whitepoint;}

    //Global all-color curves.
    float getDefShadowsX(){return d_shadowsX;}
    float getDefShadowsY(){return d_shadowsY;}
    float getDefHighlightsX(){return d_highlightsX;}
    float getDefHighlightsY(){return d_highlightsY;}
    float getDefVibrance(){return d_vibrance;}
    float getDefSaturation(){return d_saturation;}

    //Rotation
    int getDefRotation(){return d_rotation;}

    //Setters for the properties.
    //Loading
    void setTiffIn(bool);
    void setJpegIn(bool);

    //Demosaic
    void setCaEnabled(bool);
    void setHighlights(int);

    //Prefilmulation
    void setExposureComp(float);
    void setTemperature(float);
    void setTint(float);

    //Filmulation
    void setInitialDeveloperConcentration(float);
    void setReservoirThickness(float);
    void setActiveLayerThickness(float);
    void setCrystalsPerPixel(float);
    void setInitialCrystalRadius(float);
    void setInitialSilverSaltDensity(float);
    void setDeveloperConsumptionConst(float);
    void setCrystalGrowthConst(float);
    void setSilverSaltConsumptionConst(float);
    void setTotalDevelopmentTime(float);
    void setAgitateCount(int);
    void setDevelopmentSteps(int);
    void setFilmArea(float);
    void setSigmaConst(float);
    void setLayerMixConst(float);
    void setLayerTimeDivisor(float);
    void setRolloffBoundary(float);

    //Whitepoint & Blackpoint
    void setBlackpoint(float);
    void setWhitepoint(float);
    void setCropHeight(float);
    void setCropAspect(float);
    void setCropVoffset(float);
    void setCropHoffset(float);


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
    void imageIndexChanged();

    //Read-only image properties
    void filenameChanged();
    void sensitivityChanged();
    void exposureTimeChanged();
    void apertureChanged();
    void focalLengthChanged();

    //Copy/pasteing
    void pasteableChanged();

    //Loading
    void tiffInChanged();
    void jpegInChanged();

    //Demosaic
    void caEnabledChanged();
    void highlightsChanged();

    void defCaEnabledChanged();
    void defHighlightsChanged();

    //Prefilmulation
    void exposureCompChanged();
    void temperatureChanged();
    void tintChanged();

    void defExposureCompChanged();
    void defTemperatureChanged();
    void defTintChanged();

    //Filmulation
    void initialDeveloperConcentrationChanged();
    void reservoirThicknessChanged();
    void activeLayerThicknessChanged();
    void crystalsPerPixelChanged();
    void initialCrystalRadiusChanged();
    void initialSilverSaltDensityChanged();
    void developerConsumptionConstChanged();
    void crystalGrowthConstChanged();
    void silverSaltConsumptionConstChanged();
    void totalDevelopmentTimeChanged();
    void agitateCountChanged();
    void developmentStepsChanged();
    void filmAreaChanged();
    void sigmaConstChanged();
    void layerMixConstChanged();
    void layerTimeDivisorChanged();
    void rolloffBoundaryChanged();

    void defInitialDeveloperConcentrationChanged();
    void defReservoirThicknessChanged();
    void defActiveLayerThicknessChanged();
    void defCrystalsPerPixelChanged();
    void defInitialCrystalRadiusChanged();
    void defInitialSilverSaltDensityChanged();
    void defDeveloperConsumptionConstChanged();
    void defCrystalGrowthConstChanged();
    void defSilverSaltConsumptionConstChanged();
    void defTotalDevelopmentTimeChanged();
    void defAgitateCountChanged();
    void defDevelopmentStepsChanged();
    void defFilmAreaChanged();
    void defSigmaConstChanged();
    void defLayerMixConstChanged();
    void defLayerTimeDivisorChanged();
    void defRolloffBoundaryChanged();

    //Whitepoint & Blackpoint
    void blackpointChanged();
    void whitepointChanged();
    void cropHeightChanged();
    void cropAspectChanged();
    void cropVoffsetChanged();
    void cropHoffsetChanged();

    void defBlackpointChanged();
    void defWhitepointChanged();

    //Global, all-color curves.
    void shadowsXChanged();
    void shadowsYChanged();
    void highlightsXChanged();
    void highlightsYChanged();
    void vibranceChanged();
    void saturationChanged();

    void defShadowsXChanged();
    void defShadowsYChanged();
    void defHighlightsXChanged();
    void defHighlightsYChanged();
    void defVibranceChanged();
    void defSaturationChanged();

    //Rotation
    void rotationChanged();

    void defRotationChanged();

    //General: if any param changes, emit this one as well after the param-specific signal.
    void paramChanged(QString source);
    void updateImage(bool newImage);
    void updateTableOut(QString table, int operation);
};

#endif // PARAMETERMANAGER_H
