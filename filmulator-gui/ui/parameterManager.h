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
#include <memory>
#include <lensfun/lensfun.h>
#include "../core/filmSim.hpp"
#include "../database/exifFunctions.h"

enum Valid {none,
            partload,
            load,
            partdemosaic,
            demosaic,
            partprefilmulation,
            prefilmulation,
            partfilmulation,
            filmulation,
            partblackwhite,
            blackwhite,
            partcolorcurve,
            colorcurve,
            partfilmlikecurve,
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
    int caEnabled;
    int highlights;
    QString cameraName;
    QString lensName;
    bool lensfunCA;
    bool lensfunVignetting;
    bool lensfunDistortion;
    float focalLength;
    float fnumber;
    float rotationAngle;
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
    float toeBoundary;
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
    bool monochrome;
    float bwRmult;
    float bwGmult;
    float bwBmult;
};

class ParameterManager : public QObject
{
    Q_OBJECT
    //Loading
    Q_PROPERTY(QString imageIndex   READ getImageIndex   NOTIFY imageIndexChanged)

    //Read-only stuff for the UI
    Q_PROPERTY(QString filename         READ getFilename         NOTIFY filenameChanged)
    Q_PROPERTY(QString fullFilenameQstr READ getFullFilenameQstr NOTIFY fullFilenameQstrChanged)
    Q_PROPERTY(int sensitivity          READ getSensitivity      NOTIFY sensitivityChanged)
    Q_PROPERTY(QString exposureTime     READ getExposureTime     NOTIFY exposureTimeChanged)
    Q_PROPERTY(QString aperture         READ getAperture         NOTIFY apertureChanged)
    Q_PROPERTY(float focalLength        READ getFocalLength      NOTIFY focalLengthChanged)
    Q_PROPERTY(QString make             READ getMake             NOTIFY makeChanged)
    Q_PROPERTY(QString model            READ getModel            NOTIFY modelChanged)
    //Read-only lensfun stuff
    Q_PROPERTY(QString exifLensName  READ getExifLensName     NOTIFY exifLensNameChanged)
    Q_PROPERTY(bool autoCaAvail      READ getAutoCaAvail      NOTIFY autoCaAvailChanged)
    Q_PROPERTY(bool lensfunCaAvail   READ getLensfunCaAvail   NOTIFY lensfunCaAvailChanged)
    Q_PROPERTY(bool lensfunVignAvail READ getLensfunVignAvail NOTIFY lensfunVignAvailChanged)
    Q_PROPERTY(bool lensfunDistAvail READ getLensfunDistAvail NOTIFY lensfunDistAvailChanged)

    Q_PROPERTY(bool tiffIn MEMBER m_tiffIn WRITE setTiffIn NOTIFY tiffInChanged)
    Q_PROPERTY(bool jpegIn MEMBER m_jpegIn WRITE setJpegIn NOTIFY jpegInChanged)

    //Demosaic
    Q_PROPERTY(int caEnabled        MEMBER s_caEnabled      WRITE setCaEnabled      NOTIFY caEnabledChanged)
    Q_PROPERTY(int highlights       MEMBER m_highlights     WRITE setHighlights     NOTIFY highlightsChanged)
    Q_PROPERTY(QString lensfunName  MEMBER s_lensfunName    WRITE setLensfunName    NOTIFY lensfunNameChanged)
    Q_PROPERTY(int lensfunCa        MEMBER s_lensfunCa      WRITE setLensfunCa      NOTIFY lensfunCaChanged)
    Q_PROPERTY(int lensfunVign      MEMBER s_lensfunVign    WRITE setLensfunVign    NOTIFY lensfunVignChanged)
    Q_PROPERTY(int lensfunDist      MEMBER s_lensfunDist    WRITE setLensfunDist    NOTIFY lensfunDistChanged)
    Q_PROPERTY(float rotationAngle  MEMBER m_rotationAngle  WRITE setRotationAngle  NOTIFY rotationAngleChanged)
    Q_PROPERTY(float rotationPointX MEMBER m_rotationPointX WRITE setRotationPointX NOTIFY rotationPointXChanged)
    Q_PROPERTY(float rotationPointY MEMBER m_rotationPointY WRITE setRotationPointY NOTIFY rotationPointYChanged)

    Q_PROPERTY(int defCaEnabled        READ getDefCaEnabled      NOTIFY defCaEnabledChanged)
    Q_PROPERTY(int defHighlights       READ getDefHighlights     NOTIFY defHighlightsChanged)
    Q_PROPERTY(QString defLensfunName  READ getDefLensfunName    NOTIFY defLensfunNameChanged)
    Q_PROPERTY(int defLensfunCa        READ getDefLensfunCa      NOTIFY defLensfunCaChanged)
    Q_PROPERTY(int defLensfunVign      READ getDefLensfunVign    NOTIFY defLensfunVignChanged)
    Q_PROPERTY(int defLensfunDist      READ getDefLensfunDist    NOTIFY defLensfunDistChanged)
    Q_PROPERTY(float defRotationAngle  READ getDefRotationAngle  NOTIFY defRotationAngleChanged)
    Q_PROPERTY(float defRotationPointX READ getDefRotationPointX NOTIFY defRotationPointXChanged)
    Q_PROPERTY(float defRotationPointY READ getDefRotationPointY NOTIFY defRotationPointYChanged)

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
    Q_PROPERTY(float toeBoundary                   MEMBER m_toeBoundary                   WRITE setToeBoundary                   NOTIFY toeBoundaryChanged)

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
    Q_PROPERTY(float defToeBoundary                   READ getDefToeBoundary                   NOTIFY defToeBoundaryChanged)

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
    Q_PROPERTY(bool  monochrome  MEMBER m_monochrome   WRITE setMonochrome  NOTIFY monochromeChanged)
    Q_PROPERTY(float bwRmult     MEMBER m_bwRmult      WRITE setBwRmult     NOTIFY bwRmultChanged)
    Q_PROPERTY(float bwGmult     MEMBER m_bwGmult      WRITE setBwGmult     NOTIFY bwGmultChanged)
    Q_PROPERTY(float bwBmult     MEMBER m_bwBmult      WRITE setBwBmult     NOTIFY bwBmultChanged)

    Q_PROPERTY(float defShadowsX    READ getDefShadowsX    NOTIFY defShadowsXChanged)
    Q_PROPERTY(float defShadowsY    READ getDefShadowsY    NOTIFY defShadowsYChanged)
    Q_PROPERTY(float defHighlightsX READ getDefHighlightsX NOTIFY defHighlightsXChanged)
    Q_PROPERTY(float defHighlightsY READ getDefHighlightsY NOTIFY defHighlightsYChanged)
    Q_PROPERTY(float defVibrance    READ getDefVibrance    NOTIFY defVibranceChanged)
    Q_PROPERTY(float defSaturation  READ getDefSaturation  NOTIFY defSaturationChanged)
    Q_PROPERTY(bool  defMonochrome  READ getDefMonochrome  NOTIFY defMonochromeChanged)
    Q_PROPERTY(float defBwRmult     READ getDefBwRmult     NOTIFY defBwRmultChanged)
    Q_PROPERTY(float defBwGmult     READ getDefBwGmult     NOTIFY defBwGmultChanged)
    Q_PROPERTY(float defBwBmult     READ getDefBwBmult     NOTIFY defBwBmultChanged)

    Q_PROPERTY(bool pasteable READ getPasteable NOTIFY pasteableChanged)

public:
    ParameterManager();
    ~ParameterManager();

    Q_INVOKABLE void rotateRight();
    Q_INVOKABLE void rotateLeft();

    Q_INVOKABLE void selectImage(const QString imageID);

    Q_INVOKABLE void writeback();

    Q_INVOKABLE void copyAll(QString fromImageID);
    Q_INVOKABLE void paste(QString toImageID);

    //Must be called when resetting lens corrections back to default
    //So that we write back to the database, the db gets the proper "autoselect" values
    Q_INVOKABLE void resetAutoCa(){m_caEnabled = -1;}
    Q_INVOKABLE void resetLensfunName(){m_lensfunName = "NoLens";}
    Q_INVOKABLE void resetLensfunCa(){m_lensfunCa = -1;}
    Q_INVOKABLE void resetLensfunVign(){m_lensfunVign = -1;}
    Q_INVOKABLE void resetLensfunDist(){m_lensfunDist = -1;}

    //When you set lens preferences, you need to set d_lensfunXXX = s_lensfunXXX
    Q_INVOKABLE void setLensPreferences();
    Q_INVOKABLE void eraseLensPreferences();

    //combined wb; this is for custom WB sampling initiated from c++
    void setWB(const float temp, const float tint);

    //The paramMutex exists to prevent race conditions between
    // changes in the parameters and changes in validity.
    //We make them public so that we can avoid race conditions when grabbing image pipeline data
    QMutex paramMutex;
    QMutex signalMutex;

    //Each stage creates its struct, checks validity, marks the validity to indicate it's begun,
    //and then returns the struct and the validity.
    //There's a second validity-check-only method for more frequent cancellation.
    //And the final marking of complete checks one more time (and doesn't mark complete if invalid)
    //Input
    std::tuple<Valid,AbortStatus,LoadParams> claimLoadParams();
    AbortStatus claimLoadAbort();
    Valid markLoadComplete();

    //Demosaic
    std::tuple<Valid,AbortStatus,LoadParams,DemosaicParams> claimDemosaicParams();
    AbortStatus claimDemosaicAbort();
    Valid markDemosaicComplete();

    //Prefilmulation
    std::tuple<Valid,AbortStatus,PrefilmParams> claimPrefilmParams();
    AbortStatus claimPrefilmAbort();
    Valid markPrefilmComplete();

    //Filmulation
    std::tuple<Valid,AbortStatus,FilmParams> claimFilmParams();
    AbortStatus claimFilmAbort();
    Valid markFilmComplete();

    //Whitepoint & Blackpoint (and cropping and rotation and distortion)
    std::tuple<Valid,AbortStatus,BlackWhiteParams> claimBlackWhiteParams();
    AbortStatus claimBlackWhiteAbort();
    Valid markBlackWhiteComplete();

    //Individual color curves: not implemented, so we just have to mark complete
    Valid markColorCurvesComplete();

    //Global, all-color curves.
    std::tuple<Valid,AbortStatus,FilmlikeCurvesParams> claimFilmlikeCurvesParams();
    AbortStatus claimFilmLikeCurvesAbort();
    Valid markFilmLikeCurvesComplete();

    Valid getValid();
    Valid getValidityWhenCanceled();
    void setValid(Valid validityIn);

    void markStartOfProcessing(){changeMadeSinceCheck = false;}

    std::string getFullFilename(){return m_fullFilename;}

    void setClone(){isClone = true;}

    //Getters for read-only properties.
    QString getImageIndex(){return imageIndex;}

    QString getFilename(){return filename;}
    QString getFullFilenameQstr(){return fullFilenameQstr;}
    int getSensitivity(){return sensitivity;}
    QString getExposureTime(){return exposureTime;}
    QString getAperture(){return aperture;}
    float getFocalLength(){return focalLength;}
    QString getMake(){return make;}
    QString getModel(){return model;}
    QString getExifLensName(){return exifLensName;}
    bool getAutoCaAvail(){return autoCaAvail;}
    bool getLensfunCaAvail(){return lensfunCaAvail;}
    bool getLensfunVignAvail(){return lensfunVignAvail;}
    bool getLensfunDistAvail(){return lensfunDistAvail;}

    bool getPasteable(){return pasteable;}

    //Getters for the defaults
    //Demosaic
    int getDefCaEnabled(){return d_caEnabled;}
    int getDefHighlights(){return d_highlights;}
    QString getDefLensfunName(){return d_lensfunName;}
    int getDefLensfunCa(){return d_lensfunCa;}
    int getDefLensfunVign(){return d_lensfunVign;}
    int getDefLensfunDist(){return d_lensfunDist;}
    float getDefRotationAngle(){return d_rotationAngle;}
    float getDefRotationPointX(){return d_rotationPointX;}
    float getDefRotationPointY(){return d_rotationPointY;}

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
    float getDefToeBoundary(){return d_toeBoundary;}

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
    bool  getDefMonochrome(){return d_monochrome;}
    float getDefBwRmult(){return d_bwRmult;}
    float getDefBwGmult(){return d_bwGmult;}
    float getDefBwBmult(){return d_bwBmult;}

    //Rotation
    int getDefRotation(){return d_rotation;}


    //Getters for the actual params
    //Loading
    bool getTiffIn(){return m_tiffIn;}
    bool getJpegIn(){return m_jpegIn;}

    //Demosaic
    int getCaEnabled(){return s_caEnabled;}
    int getHighlights(){return m_highlights;}
    QString getLensfunName(){return s_lensfunName;}
    int getLensfunCa(){return s_lensfunCa;}
    int getLensfunVign(){return s_lensfunVign;}
    int getLensfunDist(){return s_lensfunDist;}
    float getRotationAngle(){return m_rotationAngle;}
    float getRotationPointX(){return m_rotationPointX;}
    float getRotationPointY(){return m_rotationPointY;}

    //Prefilmulation
    float getExposureComp(){return m_exposureComp;}
    float getTemperature(){return m_temperature;}
    float getTint(){return m_tint;}

    //Filmulation
    float getInitialDeveloperConcentration(){return m_initialDeveloperConcentration;}
    float getReservoirThickness(){return m_reservoirThickness;}
    float getActiveLayerThickness(){return m_activeLayerThickness;}
    float getCrystalsPerPixel(){return m_crystalsPerPixel;}
    float getInitialCrystalRadius(){return m_initialCrystalRadius;}
    float getInitialSilverSaltDensity(){return m_initialSilverSaltDensity;}
    float getDeveloperConsumptionConst(){return m_developerConsumptionConst;}
    float getCrystalGrowthConst(){return m_crystalGrowthConst;}
    float getSilverSaltConsumptionConst(){return m_silverSaltConsumptionConst;}
    float getTotalDevelopmentTime(){return m_totalDevelopmentTime;}
    int   getAgitateCount(){return m_agitateCount;}
    int   getDevelopmentSteps(){return m_developmentSteps;}
    float getFilmArea(){return m_filmArea;}
    float getSigmaConst(){return m_sigmaConst;}
    float getLayerMixConst(){return m_layerMixConst;}
    float getLayerTimeDivisor(){return m_layerTimeDivisor;}
    float getRolloffBoundary(){return m_rolloffBoundary;}
    float getToeBoundary(){return m_toeBoundary;}

    //Whitepoint & blackpoint
    float getBlackpoint(){return m_blackpoint;}
    float getWhitepoint(){return m_whitepoint;}
    float getCropHeight(){return m_cropHeight;}
    float getCropAspect(){return m_cropAspect;}
    float getCropVoffset(){return m_cropVoffset;}
    float getCropHoffset(){return m_cropHoffset;}

    //Global all-color curves.
    float getShadowsX(){return m_shadowsX;}
    float getShadowsY(){return m_shadowsY;}
    float getHighlightsX(){return m_highlightsX;}
    float getHighlightsY(){return m_highlightsY;}
    float getVibrance(){return m_vibrance;}
    float getSaturation(){return m_saturation;}
    bool  getMonochrome(){return m_monochrome;}
    float getBwRmult(){return m_bwRmult;}
    float getBwGmult(){return m_bwGmult;}
    float getBwBmult(){return m_bwBmult;}

    //Rotation
    int getRotation(){return m_rotation;}


public slots:
    //When the quick pipeline gets the params changed, we'll automatically
    // have the clone pipeline update its params.
    //This will turn changeMadeSinceCheck true, but only if it's a clone
    void cloneParams(ParameterManager * sourceParams);

    //If this is a preload pipeline, we need to stop computation
    // when *another* pipeline changes, but we don't need params copied
    void cancelComputation();

protected:
    //This is here for the sql insertion to pull the values from.
    void loadParams(QString imageID);
    void loadDefaults(const CopyDefaults useDefaults, const std::string absFilePath);

    //If this is true, then this is the clone parameter manager
    //and we should always abort whenever there's a change made
    bool isClone = false;
    bool changeMadeSinceCheck = false;

    //We need a lensfun database for looking up various things
    lfDatabase *ldb;
    //Refresh lens correction availability
    void updateAvailability();

    //This is to attempt to prevent binding loops at the start of the program
    bool justInitialized;

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
    QString fullFilenameQstr;
    int sensitivity;
    QString exposureTime;
    QString aperture;
    float fnumber;
    float focalLength;
    QString make;
    QString model;
    QString exifLensName;
    bool autoCaAvail; //For non-Bayer sensors this is not available
    bool lensfunCaAvail; //These vary depending on camera and lens (and the lensfun db)
    bool lensfunVignAvail;
    bool lensfunDistAvail;

    Valid validity;
    Valid validityWhenCanceled;

    //this is for dealing with the validity when canceled
    bool processedYet = false;

    //Input
    std::string m_fullFilename;
    bool m_tiffIn;
    bool m_jpegIn;

    //Demosaic
    int s_caEnabled;//similar to the lensfun stuff
    int m_caEnabled;
    int m_highlights;
    QString s_lensfunName;//staging params filled at load and also when manually changed
    int s_lensfunCa;      //These don't get written back
    int s_lensfunVign;
    int s_lensfunDist;
    QString m_lensfunName;//main params get loaded from db. "" or -1 indicates autoselection via exif or prefs.
    int m_lensfunCa;      //When the UI sets the s_ params, these get changed so the db gets the changes too.
    int m_lensfunVign;    //On the same token, when the UI *resets*,
    int m_lensfunDist;    // these have to go back to "" or -1, not to the default.
    float m_rotationAngle;
    float m_rotationPointX;
    float m_rotationPointY;

    int d_caEnabled; //d_'s are for default values
    int d_highlights;
    QString d_lensfunName;//*not* a blank string, but actually the lens that is either exif-automatched or in prefs
    int d_lensfunCa;      //*not* -1, but actually from prefs
    int d_lensfunVign;    //They get filled a) at loading time, or b) when lens prefs are set or erased
    int d_lensfunDist;
    float d_rotationAngle;
    float d_rotationPointX;
    float d_rotationPointY;

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
    float m_toeBoundary;

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
    float d_toeBoundary;

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
    bool  m_monochrome;
    float m_bwRmult;
    float m_bwGmult;
    float m_bwBmult;

    float d_shadowsX;
    float d_shadowsY;
    float d_highlightsX;
    float d_highlightsY;
    float d_vibrance;
    float d_saturation;
    bool  d_monochrome;
    float d_bwRmult;
    float d_bwGmult;
    float d_bwBmult;

    //Rotation
    int m_rotation;

    int d_rotation;

    //Setters for the properties.
    //Loading
    void setTiffIn(bool);
    void setJpegIn(bool);

    //Demosaic
    void setCaEnabled(int);
    void setHighlights(int);
    void setLensfunName(QString);
    void setLensfunCa(int);
    void setLensfunVign(int);
    void setLensfunDist(int);
    void setRotationAngle(float);
    void setRotationPointX(float);
    void setRotationPointY(float);

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
    void setToeBoundary(float);

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
    void setMonochrome(bool);
    void setBwRmult(float);
    void setBwGmult(float);
    void setBwBmult(float);

    //Rotation
    void setRotation(int);

signals:
    void imageIndexChanged();

    //Read-only image properties
    void filenameChanged();
    void fullFilenameQstrChanged();
    void sensitivityChanged();
    void exposureTimeChanged();
    void apertureChanged();
    void focalLengthChanged();
    void makeChanged();
    void modelChanged();
    void exifLensNameChanged();
    void autoCaAvailChanged();
    void lensfunCaAvailChanged();
    void lensfunVignAvailChanged();
    void lensfunDistAvailChanged();

    //Copy/pasteing
    void pasteableChanged();

    //Loading
    void tiffInChanged();
    void jpegInChanged();

    //Demosaic
    void caEnabledChanged();
    void highlightsChanged();
    void lensfunNameChanged();
    void lensfunCaChanged();
    void lensfunVignChanged();
    void lensfunDistChanged();
    void rotationAngleChanged();
    void rotationPointXChanged();
    void rotationPointYChanged();

    void defCaEnabledChanged();
    void defHighlightsChanged();
    void defLensfunNameChanged();
    void defLensfunCaChanged();
    void defLensfunVignChanged();
    void defLensfunDistChanged();
    void defRotationAngleChanged();
    void defRotationPointXChanged();
    void defRotationPointYChanged();

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
    void toeBoundaryChanged();

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
    void defToeBoundaryChanged();

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
    void monochromeChanged();
    void bwRmultChanged();
    void bwGmultChanged();
    void bwBmultChanged();

    void defShadowsXChanged();
    void defShadowsYChanged();
    void defHighlightsXChanged();
    void defHighlightsYChanged();
    void defVibranceChanged();
    void defSaturationChanged();
    void defMonochromeChanged();
    void defBwRmultChanged();
    void defBwGmultChanged();
    void defBwBmultChanged();

    //Rotation
    void rotationChanged();

    void defRotationChanged();

    //General: if any param changes, emit this one as well after the param-specific signal.
    void paramChanged(QString source);
    void updateClone(ParameterManager * param);
    void updateImage(bool newImage);
    void updateTableOut(QString table, int operation);

    //Error
    void fileError();
};

#endif // PARAMETERMANAGER_H
