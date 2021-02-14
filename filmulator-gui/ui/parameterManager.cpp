#include "parameterManager.h"
#include "../database/database.hpp"
#include "../database/exifFunctions.h"
#include <QFile>
#include <QDir>
#include <QStandardPaths>

using std::min;
using std::cout;
using std::endl;

ParameterManager::ParameterManager() : QObject(0)
{
    justInitialized = true;
    paramChangeEnabled = true;

    imageIndex = "";

    cout << "ParamManager load defaults to params" << endl;

    //Load the defaults, copy to the parameters, there's no filename yet.
    loadDefaults(CopyDefaults::loadToParams, "");

    //these aren't initialized by loadDefaults
    s_caEnabled = 0;
    s_lensfunName = "";
    s_lensfunCa = 0;
    s_lensfunVign = 0;
    s_lensfunDist = 0;

    aperture = "0.0";
    exposureTime = "0.0";
    filename = "";
    fullFilenameQstr = "";
    sensitivity = 0;
    focalLength = 0;
    make = "";
    model = "";
    exifLensName = "";
    autoCaAvail = true;
    lensfunCaAvail = true;
    lensfunVignAvail = true;
    lensfunDistAvail = true;
    m_lensfunName = "";

    customWbAvail = false;

    //initialize lensfun db
    QDir dir = QDir::home();
    QString dirstr = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    dirstr.append("/filmulator/version_2");
    std::string stdstring = dirstr.toStdString();
    cout << "ParamManager directory string: " << stdstring << endl;

    cout << "ParamManager initializing lensfun db" << endl;
    ldb = lf_db_create();
    if (!ldb)
    {
        cout << "Failed to create database!" << endl;
    }

    ldb->Load(stdstring.c_str());

    cout << "ParamManager done initializing lensfun" << endl;

    validity = Valid::none;

    pasteable = false;
    pasteSome = false;
}

ParameterManager::~ParameterManager()
{
    if (ldb != NULL)
    {
        lf_db_destroy(ldb);
    }
}

std::tuple<Valid,AbortStatus,LoadParams> ParameterManager::claimLoadParams()
{
    QMutexLocker paramLocker(&paramMutex);
    AbortStatus abort;
    if (validity < Valid::none)//If something earlier than this has changed
    {
        abort = AbortStatus::restart;//not actually possible
        cout << "claimLoadParams validity abort" << endl;
    }
    else if (changeMadeSinceCheck)
    {
        abort = AbortStatus::restart;
    }
    else
    {
        abort = AbortStatus::proceed;
        validity = Valid::partload;//mark as being in progress
    }
    //changeMadeSinceCheck = false;
    LoadParams params;
    params.fullFilename = m_fullFilename;
    params.tiffIn = m_tiffIn;
    params.jpegIn = m_jpegIn;
    std::tuple<Valid,AbortStatus,LoadParams> tup(validity, abort, params);
    return tup;
}

AbortStatus ParameterManager::claimLoadAbort()
{
    QMutexLocker paramLocker(&paramMutex);
    if (validity < Valid::partload)//make sure that progress on this step isn't invalid
    {
        changeMadeSinceCheck = false;
        return AbortStatus::restart;
    }
    else if (changeMadeSinceCheck)
    {
        changeMadeSinceCheck = false;
        return AbortStatus::restart;
    }
    else
    {
        changeMadeSinceCheck = false;
        return AbortStatus::proceed;
    }
}

Valid ParameterManager::markLoadComplete()
{
    QMutexLocker paramLocker(&paramMutex);
    processedYet = true;
    if (Valid::partload == validity)
    {
        validity = Valid::load;//mark step complete (duh)
    }
    return validity;
}

void ParameterManager::setTiffIn(bool tiffIn)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_tiffIn = tiffIn;
        validity = min(validity, Valid::none);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setTiff"));
    }
}

void ParameterManager::setJpegIn(bool jpegIn)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_jpegIn = jpegIn;
        validity = min(validity, Valid::none);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setJpeg"));
    }
}

std::tuple<Valid,AbortStatus,LoadParams,DemosaicParams> ParameterManager::claimDemosaicParams()
{
    QMutexLocker paramLocker(&paramMutex);
    AbortStatus abort;
    if (validity < Valid::load)
    {
        abort = AbortStatus::restart;
    }
    else if (changeMadeSinceCheck)
    {
        abort = AbortStatus::restart;
    }
    else
    {
        abort = AbortStatus::proceed;
        validity = Valid::partdemosaic;
    }
    changeMadeSinceCheck = false;
    LoadParams loadParams;
    loadParams.fullFilename = m_fullFilename;
    loadParams.jpegIn = m_jpegIn;
    loadParams.tiffIn = m_tiffIn;

    DemosaicParams demParams;
    demParams.caEnabled = s_caEnabled;
    demParams.highlights = m_highlights;
    demParams.cameraName = model;
    demParams.lensName = s_lensfunName;//we use the staging ones because they're always populated
    demParams.lensfunCA = s_lensfunCa >= 1;
    demParams.lensfunVignetting = s_lensfunVign >= 1;
    demParams.lensfunDistortion = s_lensfunDist >= 1;
    demParams.focalLength = focalLength;
    demParams.fnumber = fnumber;
    demParams.rotationAngle = m_rotationAngle;
    std::tuple<Valid,AbortStatus,LoadParams,DemosaicParams> tup(validity, abort, loadParams, demParams);
    return tup;
}

AbortStatus ParameterManager::claimDemosaicAbort()
{
    QMutexLocker paramLocker(&paramMutex);
    if (validity < Valid::partdemosaic)
    {
        changeMadeSinceCheck = false;
        return AbortStatus::restart;
    }
    else if (changeMadeSinceCheck)
    {
        changeMadeSinceCheck = false;
        return AbortStatus::restart;
    }
    else
    {
        changeMadeSinceCheck = false;
        return AbortStatus::proceed;
    }
}

Valid ParameterManager::markDemosaicComplete()
{
    QMutexLocker paramLocker(&paramMutex);
    processedYet = true;
    if (Valid::partdemosaic == validity)
    {
        validity = Valid::demosaic;
    }
    return validity;
}

void ParameterManager::setCaEnabled(int caEnabled)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        s_caEnabled = caEnabled;
        m_caEnabled = caEnabled;
        validity = min(validity, Valid::load);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setCaEnabled"));
    }
}

void ParameterManager::setHighlights(int highlights)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_highlights = highlights;
        validity = min(validity, Valid::load);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setHighlights"));
    }
}

//The lensfun parameters need staging params because when they're loaded from the
// preferences or automatched, you *don't* want them to be written back to the database

void ParameterManager::setLensfunName(QString lensName)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        s_lensfunName = lensName;
        m_lensfunName = lensName;
        validity = min(validity, Valid::load);
        paramLocker.unlock();
        //We need to check what lens corrections are available based on the camera and lens
        updateLensfunAvailability();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setLensfunName"));
    }
}

void ParameterManager::setLensfunCa(int caEnabled)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        s_lensfunCa = caEnabled;
        m_lensfunCa = caEnabled;
        validity = min(validity, Valid::load);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setLensfunCa"));
    }
}

void ParameterManager::setLensfunVign(int vignEnabled)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        s_lensfunVign = vignEnabled;
        m_lensfunVign = vignEnabled;
        validity = min(validity, Valid::load);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setLensfunVign"));
    }
}

void ParameterManager::setLensfunDist(int distEnabled)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        s_lensfunDist = distEnabled;
        m_lensfunDist = distEnabled;
        validity = min(validity, Valid::load);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setLensfunDist"));
    }
}

void ParameterManager::setRotationAngle(float angleIn)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_rotationAngle = angleIn;
        validity = min(validity, Valid::load);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setRotationAngle"));
    }
}

void ParameterManager::setRotationPointX(float rowIn)
{
    if (!justInitialized)
    {
        //no need to lock the paramMutex, since this doesn't affect the image at all
        m_rotationPointX = rowIn;
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setRotationPointX"));
    }
}

void ParameterManager::setRotationPointY(float colIn)
{
    if (!justInitialized)
    {
        //no need to lock the paramMutex, since this doesn't affect the image at all
        m_rotationPointY = colIn;
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setRotationPointY"));
    }
}

std::tuple<Valid,AbortStatus,PrefilmParams> ParameterManager::claimPrefilmParams()
{
    QMutexLocker paramLocker(&paramMutex);
    AbortStatus abort;
    if (validity < Valid::demosaic)
    {
        abort = AbortStatus::restart;
    }
    else if (changeMadeSinceCheck)
    {
        abort = AbortStatus::restart;
    }
    else
    {
        abort = AbortStatus::proceed;
        validity = Valid::partprefilmulation;
    }
    changeMadeSinceCheck = false;
    PrefilmParams params;
    params.exposureComp = m_exposureComp;
    params.temperature = m_temperature;
    params.tint = m_tint;
    params.fullFilename = m_fullFilename;//it's okay to include previous things in later params if necessary
    std::tuple<Valid,AbortStatus,PrefilmParams> tup(validity, abort, params);
    return tup;
}

AbortStatus ParameterManager::claimPrefilmAbort()
{
    QMutexLocker paramLocker(&paramMutex);
    if (validity < Valid::partprefilmulation)
    {
        changeMadeSinceCheck = false;
        return AbortStatus::restart;
    }
    else if (changeMadeSinceCheck)
    {
        changeMadeSinceCheck = false;
        return AbortStatus::restart;
    }
    else
    {
        changeMadeSinceCheck = false;
        return AbortStatus::proceed;
    }
}

Valid ParameterManager::markPrefilmComplete()
{
    QMutexLocker paramLocker(&paramMutex);
    processedYet = true;
    if (Valid::partprefilmulation == validity)
    {
        validity = Valid::prefilmulation;
    }
    return validity;
}

void ParameterManager::setExposureComp(float exposureComp)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_exposureComp = exposureComp;
        validity = min(validity, Valid::demosaic);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setExposureComp"));
    }
}

void ParameterManager::setTemperature(float temperature)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_temperature = temperature;
        validity = min(validity, Valid::demosaic);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setTemperature"));
    }
}

void ParameterManager::setTint(float tint)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_tint = tint;
        validity = min(validity, Valid::demosaic);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setTint"));
    }
}

void ParameterManager::setWB(const float temp, const float tint)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_temperature = temp;
        m_tint = tint;
        validity = min(validity, Valid::demosaic);
        paramLocker.unlock();

        //This is called from C++ only, so we have it emit the parameter change signals
        emit temperatureChanged();
        emit tintChanged();

        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setWB"));
        writeback();//Normally the slider has to call this when released, but this isn't a slider.
    }
}

std::tuple<Valid,AbortStatus,FilmParams> ParameterManager::claimFilmParams()
{
    QMutexLocker paramLocker(&paramMutex);
    AbortStatus abort;

    //If it's the first time, the source data is from prefilmulation.
    if (validity < Valid::prefilmulation)
    {
        abort = AbortStatus::restart;
    }
    else if (changeMadeSinceCheck)
    {
        abort = AbortStatus::restart;
    }
    else
    {
        abort = AbortStatus::proceed;
        validity = Valid::partfilmulation;
    }
    changeMadeSinceCheck = false;
    FilmParams params;
    params.initialDeveloperConcentration = m_initialDeveloperConcentration;
    params.reservoirThickness = m_reservoirThickness;
    params.activeLayerThickness = m_activeLayerThickness;
    params.crystalsPerPixel = m_crystalsPerPixel;
    params.initialCrystalRadius = m_initialCrystalRadius;
    params.initialSilverSaltDensity = m_initialSilverSaltDensity;
    params.developerConsumptionConst = m_developerConsumptionConst;
    params.crystalGrowthConst = m_crystalGrowthConst;
    params.silverSaltConsumptionConst = m_silverSaltConsumptionConst;
    params.totalDevelopmentTime = m_totalDevelopmentTime;
    params.agitateCount = m_agitateCount;
    params.developmentSteps = m_developmentSteps;
    params.filmArea = m_filmArea;
    params.sigmaConst = m_sigmaConst;
    params.layerMixConst = m_layerMixConst;
    params.layerTimeDivisor = m_layerTimeDivisor;
    params.rolloffBoundary = m_rolloffBoundary;
    params.toeBoundary = m_toeBoundary;
    std::tuple<Valid,AbortStatus,FilmParams> tup(validity,abort, params);
    return tup;
}

AbortStatus ParameterManager::claimFilmAbort()
{
    QMutexLocker paramLocker(&paramMutex);
    if (validity < Valid::partfilmulation)
    {
        changeMadeSinceCheck = false;
        return AbortStatus::restart;
    }
    else if (changeMadeSinceCheck)
    {
        changeMadeSinceCheck = false;
        return AbortStatus::restart;
    }
    else
    {
        changeMadeSinceCheck = false;
        return AbortStatus::proceed;
    }
}

Valid ParameterManager::markFilmComplete()
{
    QMutexLocker paramLocker(&paramMutex);
    processedYet = true;
    if (Valid::partfilmulation == validity)
    {
        validity = Valid::filmulation;
    }
    return validity;
}

void ParameterManager::setInitialDeveloperConcentration(float initialDeveloperConcentration)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_initialDeveloperConcentration = initialDeveloperConcentration;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setInitialDeveloperConcentration"));
    }
}

void ParameterManager::setReservoirThickness(float reservoirThickness)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_reservoirThickness = reservoirThickness;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setReservoirThickness"));
    }
}

void ParameterManager::setActiveLayerThickness(float activeLayerThickness)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_activeLayerThickness = activeLayerThickness;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setActiveLayerThickness"));
    }
}

void ParameterManager::setCrystalsPerPixel(float crystalsPerPixel)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_crystalsPerPixel = crystalsPerPixel;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setCrystalsPerPixel"));
    }
}

void ParameterManager::setInitialCrystalRadius(float initialCrystalRadius)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_initialCrystalRadius = initialCrystalRadius;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setInitialCrystalRadius"));
    }
}

void ParameterManager::setInitialSilverSaltDensity(float initialSilverSaltDensity)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_initialSilverSaltDensity = initialSilverSaltDensity;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setInitialSilverSaltDensity"));
    }
}

void ParameterManager::setDeveloperConsumptionConst(float developerConsumptionConst)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_developerConsumptionConst = developerConsumptionConst;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setDeveloperConsumptionConst"));
    }
}

void ParameterManager::setCrystalGrowthConst(float crystalGrowthConst)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_crystalGrowthConst = crystalGrowthConst;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setCrystalGrowthConst"));
    }
}

void ParameterManager::setSilverSaltConsumptionConst(float silverSaltConsumptionConst)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_silverSaltConsumptionConst = silverSaltConsumptionConst;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setSilverSaltConsumptionConst"));
    }
}

void ParameterManager::setTotalDevelopmentTime(float totalDevelopmentTime)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_totalDevelopmentTime = totalDevelopmentTime;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setTotalDevelopmentTime"));
    }
}

void ParameterManager::setAgitateCount(int agitateCount)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_agitateCount = agitateCount;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setAgitateCount"));
    }
}

void ParameterManager::setDevelopmentSteps(int developmentSteps)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_developmentSteps = developmentSteps;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setDevelopmentSteps"));
    }
}

void ParameterManager::setFilmArea(float filmArea)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_filmArea = filmArea;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setFilmArea"));
    }
}

void ParameterManager::setSigmaConst(float sigmaConst)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_sigmaConst = sigmaConst;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setSigmaConst"));
    }
}

void ParameterManager::setLayerMixConst(float layerMixConst)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_layerMixConst = layerMixConst;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setLayerMixConst"));
    }
}

void ParameterManager::setLayerTimeDivisor(float layerTimeDivisor)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_layerTimeDivisor = layerTimeDivisor;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setLayerTimeDivisor"));
    }
}

void ParameterManager::setRolloffBoundary(float rolloffBoundary)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_rolloffBoundary = rolloffBoundary;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setRolloffBoundary"));
    }
}

void ParameterManager::setToeBoundary(float toeBoundary)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_toeBoundary = toeBoundary;
        validity = min(validity, Valid::prefilmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setToeBoundary"));
    }
}

std::tuple<Valid,AbortStatus,BlackWhiteParams> ParameterManager::claimBlackWhiteParams()
{
    QMutexLocker paramLocker(&paramMutex);
    AbortStatus abort;
    if (validity < Valid::filmulation)
    {
        abort = AbortStatus::restart;
    }
    else if (changeMadeSinceCheck)
    {
        abort = AbortStatus::restart;
    }
    else
    {
        abort = AbortStatus::proceed;
        validity = Valid::partblackwhite;
    }
    changeMadeSinceCheck = false;
    BlackWhiteParams params;
    params.blackpoint  = m_blackpoint;
    params.whitepoint  = m_whitepoint;
    params.cropHeight  = m_cropHeight;
    params.cropAspect  = m_cropAspect;
    params.cropVoffset = m_cropVoffset;
    params.cropHoffset = m_cropHoffset;
    params.rotation = m_rotation;
    std::tuple<Valid,AbortStatus,BlackWhiteParams> tup(validity, abort, params);
    return tup;
}

//No aborting here; the pipeline will look back and recompute the histograms
// later on if the parameters change.
CropParams ParameterManager::claimCropParams()
{
    QMutexLocker paramLocker(&paramMutex);

    CropParams params;
    params.cropHeight  = m_cropHeight;
    params.cropAspect  = m_cropAspect;
    params.cropVoffset = m_cropVoffset;
    params.cropHoffset = m_cropHoffset;
    params.rotation = m_rotation;
    return params;
}

AbortStatus ParameterManager::claimBlackWhiteAbort()
{
    QMutexLocker paramLocker(&paramMutex);
    if (validity < Valid::partblackwhite)
    {
        changeMadeSinceCheck = false;
        return AbortStatus::restart;
    }
    else if (changeMadeSinceCheck)
    {
        changeMadeSinceCheck = false;
        return AbortStatus::restart;
    }
    else
    {
        changeMadeSinceCheck = false;
        return AbortStatus::proceed;
    }
}

Valid ParameterManager::markBlackWhiteComplete()
{
    QMutexLocker paramLocker(&paramMutex);
    processedYet = true;
    if (Valid::partblackwhite == validity)
    {
        validity = Valid::blackwhite;
    }
    return validity;
}

void ParameterManager::setBlackpoint(float blackpoint)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_blackpoint = blackpoint;
        validity = min(validity, Valid::filmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setBlackpoint"));
    }
}

void ParameterManager::setWhitepoint(float whitepoint)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_whitepoint = whitepoint;
        validity = min(validity, Valid::filmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setWhitepoint"));
    }
}

void ParameterManager::setCropHeight(float cropHeight)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_cropHeight = cropHeight;
        validity = min(validity, Valid::filmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setCropHeight"));
    }
}

void ParameterManager::setCropAspect(float cropAspect)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_cropAspect = cropAspect;
        validity = min(validity, Valid::filmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setCropAspect"));
    }
}

void ParameterManager::setCropVoffset(float cropVoffset)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_cropVoffset = cropVoffset;
        validity = min(validity, Valid::filmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setCropVoffset"));
    }
}

void ParameterManager::setCropHoffset(float cropHoffset)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_cropHoffset = cropHoffset;
        validity = min(validity, Valid::filmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setCropHoffset"));
    }
}

void ParameterManager::setRotation(int rotation)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_rotation = rotation;
        validity = min(validity, Valid::filmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setRotation"));
    }
}

void ParameterManager::rotateRight()
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        int rotation = m_rotation - 1;
        if (rotation < 0)
        {
            rotation += 4;
        }
        m_rotation = rotation;
        validity = min(validity, Valid::filmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("rotateRight"));
        writeback();//Normally the slider has to call this when released, but this isn't a slider.
    }
}

void ParameterManager::rotateLeft()
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        int rotation = m_rotation + 1;
        if (rotation > 3)
        {
            rotation -= 4;
        }
        m_rotation = rotation;
        validity = min(validity, Valid::filmulation);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("rotateLeft"));
        writeback();//Normally the slider has to call this when released, but this isn't a slider.
    }
}

Valid ParameterManager::markColorCurvesComplete()
{
    QMutexLocker paramLocker(&paramMutex);
    processedYet = true;
    if (Valid::blackwhite == validity)
    {
        validity = Valid::colorcurve;
    }
    return validity;
}

//We don't have any color curves, so this one short-circuits those
// and checks back to blackwhite validity.
//If we add color curves in that place, we do need to replace the following
// uses of 'Valid::blackwhite' with 'Valid::colorcurve'
std::tuple<Valid,AbortStatus,FilmlikeCurvesParams> ParameterManager::claimFilmlikeCurvesParams()
{
    QMutexLocker paramLocker(&paramMutex);
    AbortStatus abort;
    if (validity < Valid::colorcurve)
    {
        abort = AbortStatus::restart;
    }
    else if (changeMadeSinceCheck)
    {
        abort = AbortStatus::restart;
    }
    else
    {
        abort = AbortStatus::proceed;
        validity = Valid::partfilmlikecurve;
    }
    changeMadeSinceCheck = false;
    FilmlikeCurvesParams params;
    params.shadowsX = m_shadowsX;
    params.shadowsY = m_shadowsY;
    params.highlightsX = m_highlightsX;
    params.highlightsY = m_highlightsY;
    params.vibrance = m_vibrance;
    params.saturation = m_saturation;
    params.monochrome = m_monochrome;
    params.bwRmult = m_bwRmult;
    params.bwGmult = m_bwGmult;
    params.bwBmult = m_bwBmult;
    std::tuple<Valid,AbortStatus,FilmlikeCurvesParams> tup(validity, abort, params);
    return tup;
}

AbortStatus ParameterManager::claimFilmLikeCurvesAbort()
{
    QMutexLocker paramLocker(&paramMutex);
    if (validity < Valid::partfilmlikecurve)
    {
        changeMadeSinceCheck = false;
        return AbortStatus::restart;
    }
    else if (changeMadeSinceCheck)
    {
        changeMadeSinceCheck = false;
        return AbortStatus::restart;
    }
    else
    {
        changeMadeSinceCheck = false;
        return AbortStatus::proceed;
    }
}

Valid ParameterManager::markFilmLikeCurvesComplete()
{
    QMutexLocker paramLocker(&paramMutex);
    processedYet = true;
    if (Valid::partfilmlikecurve == validity)
    {
        validity = Valid::filmlikecurve;
    }
    return validity;
}

void ParameterManager::setShadowsX(float shadowsX)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_shadowsX = shadowsX;
        validity = min(validity, Valid::blackwhite);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setShadowsX"));
    }
}

void ParameterManager::setShadowsY(float shadowsY)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_shadowsY = shadowsY;
        validity = min(validity, Valid::blackwhite);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setShadowsY"));
    }
}

void ParameterManager::setHighlightsX(float highlightsX)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_highlightsX = highlightsX;
        validity = min(validity, Valid::blackwhite);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setHighlightsX"));
    }
}

void ParameterManager::setHighlightsY(float highlightsY)
{
    if (!justInitialized)
    {
        cout << "highlights Y changed" << endl;
        QMutexLocker paramLocker(&paramMutex);
        m_highlightsY = highlightsY;
        validity = min(validity, Valid::blackwhite);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setHighlightsY"));
    }
}

void ParameterManager::setVibrance(float vibrance)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_vibrance = vibrance;
        validity = min(validity, Valid::blackwhite);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setVibrance"));
    }
}

void ParameterManager::setSaturation(float saturation)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_saturation = saturation;
        validity = min(validity, Valid::blackwhite);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setSaturation"));
    }
}

void ParameterManager::setMonochrome(bool monochrome)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_monochrome = monochrome;
        validity = min(validity, Valid::blackwhite);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setMonochrome"));
    }
}

void ParameterManager::setBwRmult(float Rmult)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_bwRmult = Rmult;
        validity = min(validity, Valid::blackwhite);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setBwRmult"));
    }
}

void ParameterManager::setBwGmult(float Gmult)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_bwGmult = Gmult;
        validity = min(validity, Valid::blackwhite);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setBwGmult"));
    }
}

void ParameterManager::setBwBmult(float Bmult)
{
    if (!justInitialized)
    {
        QMutexLocker paramLocker(&paramMutex);
        m_bwBmult = Bmult;
        validity = min(validity, Valid::blackwhite);
        paramLocker.unlock();
        QMutexLocker signalLocker(&signalMutex);
        paramChangeWrapper(QString("setBwBmult"));
    }
}

Valid ParameterManager::getValid()
{
    QMutexLocker paramLocker(&paramMutex);
    if (processedYet)
    {
        return validity;
    } else {
        return Valid::none;
    }
}

void ParameterManager::setValid(Valid validityIn)
{
    QMutexLocker paramLocker(&paramMutex);
    validity = validityIn;
    if (validity != Valid::none)
    {
        //we know it had been processed
        processedYet = true;
    }
}

Valid ParameterManager::getValidityWhenCanceled()
{
    QMutexLocker paramLocker(&paramMutex);
    return validityWhenCanceled;
}

//This gets called by a slider (from qml) when it is released.
//It syncs the main (slider-interfaced) settings with the database.
void ParameterManager::writeback()
{
    //Writeback gets called by sliders once you let go.
    //Non-range-type params like orientation have the setters call this.
    if (paramChangeEnabled)
    {
        writeToDB(imageIndex);
    }
}

//This syncs the passed-in parameters to the database.
//Now that it's a SQL REPLACE, you MUST INCLUDE ALL PARAMETERS
//or else it won't populate the field; it deletes and then re-inserts.
void ParameterManager::writeToDB(QString imageID)
{
    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    //Write back the slider to the database.
    QSqlQuery query(db);
    query.exec("BEGIN;");//Stick these all into one db action for speed.
    query.prepare("REPLACE INTO ProcessingTable ("
                  "ProcTprocID, "                         // 0
                  "ProcTinitialDeveloperConcentration, "  // 1
                  "ProcTreservoirThickness, "             // 2
                  "ProcTactiveLayerThickness, "           // 3
                  "ProcTcrystalsPerPixel, "               // 4
                  "ProcTinitialCrystalRadius, "           // 5
                  "ProcTinitialSilverSaltDensity, "       // 6
                  "ProcTdeveloperConsumptionConst, "      // 7
                  "ProcTcrystalGrowthConst, "             // 8
                  "ProcTsilverSaltConsumptionConst, "     // 9
                  "ProcTtotalDevelopmentTime, "           //10
                  "ProcTagitateCount, "                   //11
                  "ProcTdevelopmentSteps, "               //12
                  "ProcTfilmArea, "                       //13
                  "ProcTsigmaConst, "                     //14
                  "ProcTlayerMixConst, "                  //15
                  "ProcTlayerTimeDivisor, "               //16
                  "ProcTrolloffBoundary, "                //17
                  "ProcTexposureComp, "                   //18
                  "ProcTwhitepoint, "                     //19
                  "ProcTblackpoint, "                     //20
                  "ProcTshadowsX, "                       //21
                  "ProcTshadowsY, "                       //22
                  "ProcThighlightsX, "                    //23
                  "ProcThighlightsY, "                    //24
                  "ProcThighlightRecovery, "              //25
                  "ProcTcaEnabled, "                      //26
                  "ProcTtemperature, "                    //27
                  "ProcTtint, "                           //28
                  "ProcTvibrance, "                       //29
                  "ProcTsaturation, "                     //30
                  "ProcTrotation, "                       //31
                  "ProcTcropHeight, "                     //32
                  "ProcTcropAspect, "                     //33
                  "ProcTcropVoffset, "                    //34
                  "ProcTcropHoffset, "                    //35
                  "ProcTmonochrome, "                     //36
                  "ProcTbwRmult, "                        //37
                  "ProcTbwGmult, "                        //38
                  "ProcTbwBmult, "                        //39
                  "ProcTtoeBoundary, "                    //40
                  "ProcTlensfunName, "                    //41
                  "ProcTlensfunCa, "                      //42
                  "ProcTlensfunVign, "                    //43
                  "ProcTlensfunDist, "                    //44
                  "ProcTrotationAngle, "                  //45
                  "ProcTrotationPointX, "                 //46
                  "ProcTrotationPointY) "                 //47
                  " values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
                  //                            1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3 3 3 3 3 3 4 4 4 4 4 4 4 4
                  //        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7
    query.bindValue( 0, imageID);
    query.bindValue( 1, m_initialDeveloperConcentration);
    query.bindValue( 2, m_reservoirThickness);
    query.bindValue( 3, m_activeLayerThickness);
    query.bindValue( 4, m_crystalsPerPixel);
    query.bindValue( 5, m_initialCrystalRadius);
    query.bindValue( 6, m_initialSilverSaltDensity);
    query.bindValue( 7, m_developerConsumptionConst);
    query.bindValue( 8, m_crystalGrowthConst);
    query.bindValue( 9, m_silverSaltConsumptionConst);
    query.bindValue(10, m_totalDevelopmentTime);
    query.bindValue(11, m_agitateCount);
    query.bindValue(12, m_developmentSteps);
    query.bindValue(13, m_filmArea);
    query.bindValue(14, m_sigmaConst);
    query.bindValue(15, m_layerMixConst);
    query.bindValue(16, m_layerTimeDivisor);
    query.bindValue(17, m_rolloffBoundary);
    query.bindValue(18, m_exposureComp);
    query.bindValue(19, m_whitepoint);
    query.bindValue(20, m_blackpoint);
    query.bindValue(21, m_shadowsX);
    query.bindValue(22, m_shadowsY);
    query.bindValue(23, m_highlightsX);
    query.bindValue(24, m_highlightsY);
    query.bindValue(25, m_highlights);
    query.bindValue(26, m_caEnabled);
    query.bindValue(27, m_temperature);
    query.bindValue(28, m_tint);
    query.bindValue(29, m_vibrance);
    query.bindValue(30, m_saturation);
    query.bindValue(31, m_rotation);
    query.bindValue(32, m_cropHeight);
    query.bindValue(33, m_cropAspect);
    query.bindValue(34, m_cropVoffset);
    query.bindValue(35, m_cropHoffset);
    query.bindValue(36, m_monochrome);
    query.bindValue(37, m_bwRmult);
    query.bindValue(38, m_bwGmult);
    query.bindValue(39, m_bwBmult);
    query.bindValue(40, m_toeBoundary);
    query.bindValue(41, m_lensfunName);
    query.bindValue(42, m_lensfunCa);
    query.bindValue(43, m_lensfunVign);
    query.bindValue(44, m_lensfunDist);
    query.bindValue(45, m_rotationAngle);
    query.bindValue(46, m_rotationPointX);
    query.bindValue(47, m_rotationPointY);
    query.exec();
    //Write that it's been edited to the SearchTable (actually writing the edit time)
    QDateTime now = QDateTime::currentDateTime();
    query.prepare("UPDATE SearchTable SET STlastProcessedTime = ?, "
                  "STthumbWritten = 0, "
                  "STbigThumbWritten = 0 "
                  "WHERE STsearchID = ?;");
    query.bindValue(0, QVariant(now.toSecsSinceEpoch()));
    query.bindValue(1, imageID);
    query.exec();
    //Write that it's been edited to the QueueTable
    //If it's not in the queue yet then this won't do anything.
    query.prepare("UPDATE QueueTable SET QTprocessed = 1, "
                  "QTexported = 0 WHERE QTsearchID = ?;");
    query.bindValue(0, imageID);
    query.exec();
    query.exec("COMMIT;");//Apply all the changes together.

    //Notify other database models of the changes.
    //If you change this, you have to change the same in paste() as well.
    emit updateTableOut("ProcessingTable", 0);//0 means edit
    emit updateTableOut("SearchTable", 0);//0 means edit
    emit updateTableOut("QueueTable", 0);//0 means edit
}

//selectImage deals with selection from qml.
//It accepts the searchID (the md5 with the instance number appended).
//It loads the default parameters, and commands the parameters to be loaded.
//If there is no ProcTable stuff for the image, it will load from defaults and write to the db.
void ParameterManager::selectImage(const QString imageID)
{
    QMutexLocker paramLocker(&paramMutex);//Make all the param changes happen together.
    disableParamChange();//Prevent aborting of computation.

    if (imageIndex != imageID)
    {
        imageIndex = imageID;
        if (processedYet)
        {
            validityWhenCanceled = validity;
            processedYet = false;
        } else {
            validityWhenCanceled = Valid::none;
        }
        validity = Valid::none;
        emit imageIndexChanged();
    }

    QString tempString = imageID;
    tempString.truncate(32);//length of md5


    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    QSqlQuery query(db);
    query.prepare("SELECT \
                  FTfilePath,FTsensitivity,FTexposureTime,FTaperture,FTfocalLength,FTcameraMake,FTcameraModel \
                  FROM FileTable WHERE FTfileID = ?;");
    query.bindValue(0, QVariant(tempString));
    query.exec();
    query.first();

    //This will help us get the column index of the desired column name.
    int nameCol;
    QSqlRecord rec = query.record();

    //Filename. First is the full path for the image pipeline.
    nameCol = rec.indexOf("FTfilePath");
    if (-1 == nameCol) { std::cout << "paramManager FTfilePath" << endl; }
    QString name = query.value(nameCol).toString();
    fullFilenameQstr = name;
    m_fullFilename = name.toStdString();
    filename = name.right(name.size() - name.lastIndexOf(QString("/")) - 1);

    //We need to check that the file is actually accessible before continuing.
    QFile file(name);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug("File could not be opened.");
        emit fileError();
        return;
    } else {
        emit filenameChanged();
        emit fullFilenameQstrChanged();
    }

    nameCol = rec.indexOf("FTsensitivity");
    if (-1 == nameCol) { std::cout << "paramManager FTsensitivity" << endl; }
    sensitivity = query.value(nameCol).toInt();
    emit sensitivityChanged();

    nameCol = rec.indexOf("FTexposureTime");
    if (-1 == nameCol) { std::cout << "paramManager FTexposureTime" << endl; }
    QString expTimeTemp = query.value(nameCol).toString();
    bool okNum;
    unsigned int numerator = expTimeTemp.left(expTimeTemp.lastIndexOf("/")).toInt(&okNum, 10);
    bool okDen;
    unsigned int denominator = expTimeTemp.right(expTimeTemp.size() - expTimeTemp.lastIndexOf("/") - 1).toInt(&okDen, 10);
    if (okNum && okDen)
    {
        unsigned int divisor = gcd(numerator, denominator);
        numerator /= divisor;
        denominator /= divisor;
        double expTime = double(numerator)/double(denominator);
        if (expTime > 0.5)
        {
            exposureTime = QString::number(expTime, 'f', 1);
        } else if (expTime < 0.1 && expTime > 0) { //some phones have weird fractional shutter speeds
            exposureTime = QString("1/%1").arg(round(1.0/expTime));
        } else {
            exposureTime = QString("%1/%2").arg(numerator).arg(denominator);
        }
    }
    else
    {
        exposureTime = expTimeTemp;
    }
    emit exposureTimeChanged();

    nameCol = rec.indexOf("FTaperture");
    if (-1 == nameCol) { std::cout << "paramManager FTaperture" << endl; }
    fnumber = query.value(nameCol).toFloat();
    if (fnumber >= 8)
    {
        aperture = QString::number(fnumber,'f',0);
    }
    else //fnumber < 8
    {
        aperture = QString::number(fnumber,'f',1);
    }
    emit apertureChanged();

    nameCol = rec.indexOf("FTfocalLength");
    if (-1 == nameCol) { std::cout << "paramManager FTfocalLength" << endl; }
    focalLength = query.value(nameCol).toFloat();
    emit focalLengthChanged();

    nameCol = rec.indexOf("FTcameraMake");
    if (-1 == nameCol) { std::cout << "paramManager FTcameraMake" << endl; }
    make = query.value(nameCol).toString();
    emit makeChanged();

    nameCol = rec.indexOf("FTcameraModel");
    if (-1 == nameCol) { std::cout << "paramManager FTcameraModel" << endl; }
    model = query.value(nameCol).toString();
    emit modelChanged();

    exifLensName = exifLens(m_fullFilename);
    //cout << "parammanager exifLensName: " << exifLensName.toStdString() << endl;
    emit exifLensNameChanged();

    //Copy all of the processing parameters from the db into this param manager.
    //First we check and see if it's new or not.
    query.prepare("SELECT COUNT(*) FROM ProcessingTable WHERE ProcTprocID = ?;");
    query.bindValue(0, QVariant(imageID));
    query.exec();
    query.first();
    const bool newImage = (query.value(0).toInt() == 0);
    if (newImage == true)
    {
        //Load the defaults for this image (including rotation and WB)
        loadDefaults(CopyDefaults::loadToParams, m_fullFilename);
        //Write them to the database
        writeToDB(imageID);
    }
    else
    {
        //Load parameters from the database
        loadParams(imageID);
        //Load the defaults, but don't change the main parameters.
        loadDefaults(CopyDefaults::loadOnlyDefaults, m_fullFilename);
    }

    //Now that we have the database params m_, we need to set the d_ and s_ params accordingly

    //We need to fill the d_ parameters with what they'd be as default:
    // whatever's preferred, or if there's no preference, automatched.
    //The s_ params will be set here, but overwritten if the m_ params are set.

    //First check and see if it's in the table
    query.prepare("SELECT COUNT(*) FROM LensPrefs  WHERE ExifCamera = ? AND ExifLens = ?;");
    query.bindValue(0, model);
    query.bindValue(1, exifLensName);
    query.exec();
    query.first();
    const bool hasPreferences = (query.value(0).toInt() > 0);
    if (hasPreferences)
    {
        //cout << "Has lens preferences" << endl;
        query.prepare("SELECT LensfunLens, LensfunCa, LensfunVign, LensfunDist, AutoCa FROM LensPrefs  WHERE ExifCamera = ? AND ExifLens = ?;");
        query.bindValue(0, model);
        query.bindValue(1, exifLensName);
        query.exec();
        query.first();

        rec = query.record();

        nameCol = rec.indexOf("LensfunLens");
        if (-1 == nameCol) { std::cout << "paramManager LensfunLens" << endl; }
        d_lensfunName = query.value(nameCol).toString();
        s_lensfunName = d_lensfunName;

        nameCol = rec.indexOf("LensfunCa");
        if (-1 == nameCol) { std::cout << "paramManager LensfunCa" << endl; }
        d_lensfunCa = query.value(nameCol).toInt();
        s_lensfunCa = d_lensfunCa;

        nameCol = rec.indexOf("LensfunVign");
        if (-1 == nameCol) { std::cout << "paramManager LensfunVign" << endl; }
        d_lensfunVign = query.value(nameCol).toInt();
        s_lensfunVign = d_lensfunVign;

        nameCol = rec.indexOf("LensfunDist");
        if (-1 == nameCol) { std::cout << "paramManager LensfunDist" << endl; }
        d_lensfunDist = query.value(nameCol).toInt();
        s_lensfunDist = d_lensfunDist;

        nameCol = rec.indexOf("AutoCa");
        if (-1 == nameCol) { std::cout << "paramManager AutoCa" << endl; }
        d_caEnabled = query.value(nameCol).toInt();
        s_caEnabled = d_caEnabled;
    } else {
        //No preferences
        //cout << "Has no lens preferences" << endl;
        //If there's a match for the exif lens, use that
        d_lensfunName = identifyLens(m_fullFilename);
        s_lensfunName = d_lensfunName;
        //cout << "Found lens: " << d_lensfunName.toStdString() << endl;
        //There are no global preferences, so we turn off all the corrections
        d_caEnabled = 0;
        s_caEnabled = 0;
        d_lensfunCa = 0;
        s_lensfunCa = 0;
        d_lensfunVign = 0;
        s_lensfunVign = 0;
        d_lensfunDist = 0;
        s_lensfunDist = 0;
    }

    //Now, if the m_ params are set, we overwrite the s_ params accordingly
    if (m_lensfunName != "NoLens")
    {
        s_lensfunName = m_lensfunName;
        //cout << "Lens was in database: " << s_lensfunName.toStdString() << endl;
        if (m_caEnabled > -1)
        {
            s_caEnabled = m_caEnabled;
        }
        if (m_lensfunCa > -1)
        {
            s_lensfunCa = m_lensfunCa;
        }
        if (m_lensfunVign > -1)
        {
            s_lensfunVign = m_lensfunVign;
        }
        if (m_lensfunDist > -1)
        {
            s_lensfunDist = m_lensfunDist;
        }
    } else {
        //If there's no matching lens, disable all the lensfun corrections.
        //cout << "No lens found" << endl;
        //s_caEnabled needs to be changed to whatever the database said though.
        if (m_caEnabled > -1)
        {
            s_caEnabled = m_caEnabled;
        }
        s_lensfunCa = 0;
        s_lensfunVign = 0;
        s_lensfunDist = 0;
    }
    //cout << "Default lens: " << d_lensfunName.toStdString() << endl;

    //Finally, we need to change the availability for the various lens corrections
    //First is Auto CA Correct, which only works with Bayer CFAs.
    std::unique_ptr<LibRaw> libraw = std::unique_ptr<LibRaw>(new LibRaw());
    int libraw_error;
#if (defined(_WIN32) || defined(__WIN32__))
    const QString tempFilename = QString::fromStdString(m_fullFilename);
    std::wstring wstr = tempFilename.toStdWString();
    libraw_error = libraw->open_file(wstr.c_str());
#else
    const char *cstr = m_fullFilename.c_str();
    libraw_error = libraw->open_file(cstr);
#endif
    if (libraw_error)
    {
        cout << "selectImage: Could not read input file!" << endl;
        cout << "libraw error text: " << libraw_strerror(libraw_error) << endl;
        emit fileError();
        return;
    }

    bool isSraw = libraw->is_sraw();
    //cout << "Is sraw: " << isSraw << endl;
    bool isWeird = libraw->COLOR(0,0)==6;
    //cout << "Is weird: " << isWeird << endl;
    int maxXtrans = 0;
    for (int i=0; i<6; i++)
    {
        for (int j=0; j<6; j++)
        {
            maxXtrans = max(maxXtrans,int(libraw->imgdata.idata.xtrans[i][j]));
        }
    }
    bool isXtrans = maxXtrans > 0;
    //cout << "Is xtrans: " << isXtrans << endl;
    autoCaAvail = !isSraw && !isWeird && !isXtrans;
    //cout << "Auto CA is available: " << autoCaAvail << endl;
    emit autoCaAvailChanged();

    //Then is lensfun, which depends on the camera and lens.
    updateLensfunAvailability();

    //Then is the white balance, which depends on the camera model
    updateCustomWbAvailability();

    paramLocker.unlock();

    //Emit that the things have changed.
    emit caEnabledChanged();
    emit highlightsChanged();
    emit lensfunNameChanged();
    emit lensfunCaChanged();
    emit lensfunVignChanged();
    emit lensfunDistChanged();
    emit rotationAngleChanged();
    emit rotationPointXChanged();
    emit rotationPointYChanged();
    emit exposureCompChanged();
    emit temperatureChanged();
    emit tintChanged();
    emit initialDeveloperConcentrationChanged();
    emit reservoirThicknessChanged();
    emit activeLayerThicknessChanged();
    emit crystalsPerPixelChanged();
    emit initialCrystalRadiusChanged();
    emit initialSilverSaltDensityChanged();
    emit developerConsumptionConstChanged();
    emit crystalGrowthConstChanged();
    emit silverSaltConsumptionConstChanged();
    emit totalDevelopmentTimeChanged();
    emit agitateCountChanged();
    emit developmentStepsChanged();
    emit filmAreaChanged();
    emit sigmaConstChanged();
    emit layerMixConstChanged();
    emit layerTimeDivisorChanged();
    emit rolloffBoundaryChanged();
    emit blackpointChanged();
    emit whitepointChanged();
    emit cropHeightChanged();
    emit cropAspectChanged();
    emit cropVoffsetChanged();
    emit cropHoffsetChanged();
    emit rotationChanged();
    emit shadowsXChanged();
    emit shadowsYChanged();
    emit highlightsXChanged();
    emit highlightsYChanged();
    emit vibranceChanged();
    emit saturationChanged();
    emit monochromeChanged();
    emit bwRmultChanged();
    emit bwGmultChanged();
    emit bwBmultChanged();
    emit toeBoundaryChanged();

    emit defCaEnabledChanged();
    emit defHighlightsChanged();
    emit defLensfunNameChanged();
    emit defLensfunCaChanged();
    emit defLensfunVignChanged();
    emit defLensfunDistChanged();
    emit defRotationAngleChanged();
    emit defRotationPointXChanged();
    emit defRotationPointYChanged();
    emit defExposureCompChanged();
    emit defTemperatureChanged();
    emit defTintChanged();
    emit defInitialDeveloperConcentrationChanged();
    emit defReservoirThicknessChanged();
    emit defActiveLayerThicknessChanged();
    emit defCrystalsPerPixelChanged();
    emit defInitialCrystalRadiusChanged();
    emit defInitialSilverSaltDensityChanged();
    emit defDeveloperConsumptionConstChanged();
    emit defCrystalGrowthConstChanged();
    emit defSilverSaltConsumptionConstChanged();
    emit defTotalDevelopmentTimeChanged();
    emit defAgitateCountChanged();
    emit defDevelopmentStepsChanged();
    emit defFilmAreaChanged();
    emit defSigmaConstChanged();
    emit defLayerMixConstChanged();
    emit defLayerTimeDivisorChanged();
    emit defRolloffBoundaryChanged();
    emit defBlackpointChanged();
    emit defWhitepointChanged();
    emit defShadowsXChanged();
    emit defShadowsYChanged();
    emit defHighlightsXChanged();
    emit defHighlightsYChanged();
    emit defVibranceChanged();
    emit defSaturationChanged();
    emit defMonochromeChanged();
    emit defBwRmultChanged();
    emit defBwGmultChanged();
    emit defBwBmultChanged();
    emit defToeBoundaryChanged();

    //Mark that it's safe for sliders to move again.
    QMutexLocker signalLocker(&signalMutex);
    enableParamChange();//Re-enable updating of the image.
    paramChangeWrapper(QString("selectImage"));

}

//This loads all of the default processing params into the param manager.
//
//If told to copyDefaults, it'll load all of the defaults into everything.
//If the file path is an empty string, it doesn't perform any computations on the exif data
// and just loads from the default profile.
void ParameterManager::loadDefaults(const CopyDefaults copyDefaults, const std::string absFilePath)
{
    QSqlRecord rec;
    int nameCol;

    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    QSqlQuery query(db);

    //This query will be shared.
    query.prepare("SELECT * FROM ProfileTable WHERE ProfTprofileID = ?;");
    query.bindValue(0, "Default");
    query.exec();
    query.first();
    rec = query.record();

    //These should be changed depending on the file, once we get this loading tiffs.
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        validity = Valid::none;
        m_tiffIn = false;
        m_jpegIn = false;
    }

    //First is caEnabled. See the lensfun stuff for explanation as to why we don't write d_
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        nameCol = rec.indexOf("ProfTcaEnabled");
        if (-1 == nameCol) { std::cout << "paramManager ProfTcaEnabled" << endl; }
        m_caEnabled = query.value(nameCol).toInt();
    }

    //Highlights (highlight recovery)
    nameCol = rec.indexOf("ProfThighlightRecovery");
    if (-1 == nameCol) { std::cout << "paramManager ProfThighlightRecovery" << endl; }
    const int temp_highlights = query.value(nameCol).toInt();
    d_highlights = temp_highlights;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_highlights = temp_highlights;
    }

    //The lens correction parameters don't actually have the defaults loaded into the d_ params.
    //If it's "loadtoparams" then we copy them only into the m_ params,
    // so that they can be written back to the database.
    //Otherwise, the m_ params are filled from the database by loadParams(),
    // the d_ params are filled from the lens prefs or by automatching,
    // and s_ params are filled from either database, lens prefs, or automatching in that order.
    //So this is only needed if it's "loadtoparams", otherwise do nothing for lens corrections.
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        //Lensfun lens name
        nameCol = rec.indexOf("ProfTlensfunName");
        if (-1 == nameCol) { std::cout << "paramManager ProfTlensfunName" << endl; }
        m_lensfunName = query.value(nameCol).toString();
        //Lensfun CA correction
        nameCol = rec.indexOf("ProfTlensfunCa");
        if (-1 == nameCol) { std::cout << "paramManager ProfTlensfunCa" << endl; }
        m_lensfunCa = query.value(nameCol).toInt();
        //Lensfun vignetting correction
        nameCol = rec.indexOf("ProfTlensfunVign");
        if (-1 == nameCol) { std::cout << "paramManager ProfTlensfunVign" << endl; }
        m_lensfunVign = query.value(nameCol).toInt();
        //Lensfun distortion correction
        nameCol = rec.indexOf("ProfTlensfunDist");
        if (-1 == nameCol) { std::cout << "paramManager ProfTlensfunDist" << endl; }
        m_lensfunDist = query.value(nameCol).toInt();
    }

    //Fine rotation angle
    nameCol = rec.indexOf("ProfTrotationAngle");
    if (-1 == nameCol) { std::cout << "paramManager ProfTrotationAngle" << endl; }
    const float temp_rotationAngle = query.value(nameCol).toFloat();
    d_rotationAngle = temp_rotationAngle;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_rotationAngle = temp_rotationAngle;
    }

    //Rotation reference point coordinates
    nameCol = rec.indexOf("ProfTrotationPointX");
    if (-1 == nameCol) { std::cout << "paramManager ProfTrotationPointX" << endl; }
    const float temp_rotationPointX = query.value(nameCol).toFloat();
    d_rotationPointX = temp_rotationPointX;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_rotationPointX = temp_rotationPointX;
    }
    nameCol = rec.indexOf("ProfTrotationPointY");
    if (-1 == nameCol) { std::cout << "paramManager ProfTrotationPointY" << endl; }
    const float temp_rotationPointY = query.value(nameCol).toFloat();
    d_rotationPointY = temp_rotationPointY;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_rotationPointY = temp_rotationPointY;
    }

    //Exposure compensation
    nameCol = rec.indexOf("ProfTexposureComp");
    if (-1 == nameCol) { std::cout << "paramManager ProfTexposureComp" << endl; }
    const float temp_exposureComp = query.value(nameCol).toFloat();
    d_exposureComp = temp_exposureComp;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_exposureComp = temp_exposureComp;
    }

    if ("" == filename)
    {
        //If there is no filename supplied, then get the defaults from the db standard profile.
        //Temperature
        nameCol = rec.indexOf("ProfTtemperature");
        if (-1 == nameCol) { std::cout << "paramManager ProfTtemperature" << endl; }
        const float temp_temperature = query.value(nameCol).toFloat();
        d_temperature = temp_temperature;

        //Tint
        nameCol = rec.indexOf("ProfTtint");
        if (-1 == nameCol) { std::cout << "paramManager ProfTtint" << endl; }
        const float temp_tint = query.value(nameCol).toFloat();
        d_tint = temp_tint;
    }
    else
    {
        //If there is a file, calculate the camera WB.
        //This should only be used for raw images, in the case that tiff support is added back in.
        float temp_temperature, temp_tint;
        optimizeWBMults(absFilePath, temp_temperature, temp_tint);
        d_temperature = temp_temperature;
        d_tint = temp_tint;
    }
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_temperature = d_temperature;
        m_tint = d_tint;
    }

    //Initial developer concentration
    nameCol = rec.indexOf("ProfTinitialDeveloperConcentration");
    if (-1 == nameCol) { std::cout << "paramManager ProfTinitialDeveloperConcentration" << endl; }
    const float temp_initialDeveloperConcentration = query.value(nameCol).toFloat();
    d_initialDeveloperConcentration = temp_initialDeveloperConcentration;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_initialDeveloperConcentration = temp_initialDeveloperConcentration;
    }

    //Reservoir thickness
    nameCol = rec.indexOf("ProfTreservoirThickness");
    if (-1 == nameCol) { std::cout << "paramManager ProfTreservoirThickness" << endl; }
    const float temp_reservoirThickness = query.value(nameCol).toFloat();
    d_reservoirThickness = temp_reservoirThickness;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_reservoirThickness = temp_reservoirThickness;
    }

    //Active layer thickness
    nameCol = rec.indexOf("ProfTactiveLayerThickness");
    if (-1 == nameCol) { std::cout << "paramManager ProfTactiveLayerThickness" << endl; }
    const float temp_activeLayerThickness = query.value(nameCol).toFloat();
    d_activeLayerThickness = temp_activeLayerThickness;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_activeLayerThickness = temp_activeLayerThickness;
    }

    //Crystals per pixel
    nameCol = rec.indexOf("ProfTcrystalsPerPixel");
    if (-1 == nameCol) { std::cout << "paramManager ProfTcrystalsPerPixel" << endl; }
    const float temp_crystalsPerPixel = query.value(nameCol).toFloat();
    d_crystalsPerPixel = temp_crystalsPerPixel;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_crystalsPerPixel = temp_crystalsPerPixel;
    }

    //Initial crystal radius
    nameCol = rec.indexOf("ProfTinitialCrystalRadius");
    if (-1 == nameCol) { std::cout << "paramManager ProfTinitialCrystalRadius" << endl; }
    const float temp_initialCrystalRadius = query.value(nameCol).toFloat();
    d_initialCrystalRadius = temp_initialCrystalRadius;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_initialCrystalRadius = temp_initialCrystalRadius;
    }

    //Initial silver salt area density
    nameCol = rec.indexOf("ProfTinitialSilverSaltDensity");
    if (-1 == nameCol) { std::cout << "paramManager ProfTinitialSilverSaltDensity" << endl; }
    const float temp_initialSilverSaltDensity = query.value(nameCol).toFloat();
    d_initialSilverSaltDensity = temp_initialSilverSaltDensity;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_initialSilverSaltDensity = temp_initialSilverSaltDensity;
    }

    //Developer consumption rate constant
    nameCol = rec.indexOf("ProfTdeveloperConsumptionConst");
    if (-1 == nameCol) { std::cout << "paramManager ProfTdeveloperConsumptionConst" << endl; }
    const float temp_developerConsumptionConst = query.value(nameCol).toFloat();
    d_developerConsumptionConst = temp_developerConsumptionConst;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_developerConsumptionConst = temp_developerConsumptionConst;
    }

    //Crystal growth rate constant
    nameCol = rec.indexOf("ProfTcrystalGrowthConst");
    if (-1 == nameCol) { std::cout << "paramManager ProfTcrystalGrowthConst" << endl; }
    const float temp_crystalGrowthConst = query.value(nameCol).toFloat();
    d_crystalGrowthConst = temp_crystalGrowthConst;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_crystalGrowthConst = temp_crystalGrowthConst;
    }

    //Silver halide consumption rate constant
    nameCol = rec.indexOf("ProfTsilverSaltConsumptionConst");
    if (-1 == nameCol) { std::cout << "paramManager ProfTsilverSaltConsumptionConst" << endl; }
    const float temp_silverSaltConsumptionConst = query.value(nameCol).toFloat();
    d_silverSaltConsumptionConst = temp_silverSaltConsumptionConst;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_silverSaltConsumptionConst = temp_silverSaltConsumptionConst;
    }

    //Total development time
    nameCol = rec.indexOf("ProfTtotalDevelopmentTime");
    if (-1 == nameCol) { std::cout << "paramManager ProfTtotalDevelopmentTime" << endl; }
    const float temp_totalDevelopmentTime = query.value(nameCol).toFloat();
    d_totalDevelopmentTime = temp_totalDevelopmentTime;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_totalDevelopmentTime = temp_totalDevelopmentTime;
    }

    //Number of agitations
    nameCol = rec.indexOf("ProfTagitateCount");
    if (-1 == nameCol) { std::cout << "paramManager ProfTagitateCount" << endl; }
    const int temp_agitateCount = query.value(nameCol).toInt();
    d_agitateCount = temp_agitateCount;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_agitateCount = temp_agitateCount;
    }

    //Number of simulation steps for development
    nameCol = rec.indexOf("ProfTdevelopmentSteps");
    if (-1 == nameCol) { std::cout << "paramManager ProfTdevelopmentSteps" << endl; }
    const int temp_developmentSteps = query.value(nameCol).toInt();
    d_developmentSteps = temp_developmentSteps;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_developmentSteps = temp_developmentSteps;
    }

    //Area of film for the simulation
    nameCol = rec.indexOf("ProfTfilmArea");
    if (-1 == nameCol) { std::cout << "paramManager ProfTfilmArea" << endl; }
    const float temp_filmArea = query.value(nameCol).toFloat();
    d_filmArea = temp_filmArea;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_filmArea = temp_filmArea;
    }

    //A constant for the size of the diffusion. It...affects the same thing as film area.
    nameCol = rec.indexOf("ProfTsigmaConst");
    if (-1 == nameCol) { std::cout << "paramManager ProfTsigmaConst" << endl; }
    const float temp_sigmaConst = query.value(nameCol).toFloat();
    d_sigmaConst = temp_sigmaConst;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_sigmaConst = temp_sigmaConst;
    }

    //Layer mix constant: the amount of active developer that gets exchanged with the reservoir.
    nameCol = rec.indexOf("ProfTlayerMixConst");
    if (-1 == nameCol) { std::cout << "paramManager ProfTlayerMixConst" << endl; }
    const float temp_layerMixConst = query.value(nameCol).toFloat();
    d_layerMixConst = temp_layerMixConst;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_layerMixConst = temp_layerMixConst;
    }

    //Layer time divisor: Controls the relative intra-layer and inter-layer diffusion.
    nameCol = rec.indexOf("ProfTlayerTimeDivisor");
    if (-1 == nameCol) { std::cout << "paramManager ProfTlayerTimeDivisor" << endl; }
    const float temp_layerTimeDivisor = query.value(nameCol).toFloat();
    d_layerTimeDivisor = temp_layerTimeDivisor;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_layerTimeDivisor = temp_layerTimeDivisor;
    }

    //Rolloff boundary. This is where highlights start to roll off.
    nameCol = rec.indexOf("ProfTrolloffBoundary");
    if (-1 == nameCol) { std::cout << "paramManager ProfTrolloffBoundary" << endl; }
    const float temp_rolloffBoundary = query.value(nameCol).toFloat();
    d_rolloffBoundary = temp_rolloffBoundary;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_rolloffBoundary = temp_rolloffBoundary;
    }

    //Toe boundary. This is the offset for the values where the toe starts to roll off.
    nameCol = rec.indexOf("ProfTtoeBoundary");
    if (-1 == nameCol) { std::cout << "paramManager ProfTtoeBoundary" << endl; }
    const float temp_toeBoundary = query.value(nameCol).toFloat();
    d_toeBoundary = temp_toeBoundary;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_toeBoundary = temp_toeBoundary;
    }

    //Post-filmulator black clipping point
    nameCol = rec.indexOf("ProfTblackpoint");
    if (-1 == nameCol) { std::cout << "paramManager ProfTblackpoint" << endl; }
    const float temp_blackpoint = query.value(nameCol).toFloat();
    d_blackpoint = temp_blackpoint;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_blackpoint = temp_blackpoint;
    }

    //Post-filmulator white clipping point
    nameCol = rec.indexOf("ProfTwhitepoint");
    if (-1 == nameCol) { std::cout << "paramManager ProfTwhitepoint" << endl; }
    const float temp_whitepoint = query.value(nameCol).toFloat();
    d_whitepoint = temp_whitepoint;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_whitepoint = temp_whitepoint;
    }

    //Crop stuff; this stuff always defaults to 0 so that the UI knows to
    // pick the correct aspect ratio for the full image.
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_cropHeight = 0;
        m_cropAspect = 0;
        m_cropVoffset = 0;
        m_cropHoffset = 0;
    }

    //Shadow control point x value
    nameCol = rec.indexOf("ProfTshadowsX");
    if (-1 == nameCol) { std::cout << "paramManager ProfTshadowsX" << endl; }
    const float temp_shadowsX = query.value(nameCol).toFloat();
    d_shadowsX = temp_shadowsX;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_shadowsX = temp_shadowsX;
    }

    //Shadow control point y value
    nameCol = rec.indexOf("ProfTshadowsY");
    if (-1 == nameCol) { std::cout << "paramManager ProfTshadowsY" << endl; }
    const float temp_shadowsY = query.value(nameCol).toFloat();
    d_shadowsY = temp_shadowsY;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_shadowsY = temp_shadowsY;
    }

    //Highlight control point x value
    nameCol = rec.indexOf("ProfThighlightsX");
    if (-1 == nameCol) { std::cout << "paramManager ProfThighlightsX" << endl; }
    const float temp_highlightsX = query.value(nameCol).toFloat();
    d_highlightsX = temp_highlightsX;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_highlightsX = temp_highlightsX;
    }

    //Highlight control point y value
    nameCol = rec.indexOf("ProfThighlightsY");
    if (-1 == nameCol) { std::cout << "paramManager ProfThighlightsY" << endl; }
    const float temp_highlightsY = query.value(nameCol).toFloat();
    d_highlightsY = temp_highlightsY;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_highlightsY = temp_highlightsY;
    }

    //Vibrance (saturation of less-saturated things)
    nameCol = rec.indexOf("ProfTvibrance");
    if (-1 == nameCol) { std::cout << "paramManager ProfTvibrance" << endl; }
    const float temp_vibrance = query.value(nameCol).toFloat();
    d_vibrance = temp_vibrance;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_vibrance = temp_vibrance;
    }

    //Saturation
    nameCol = rec.indexOf("ProfTsaturation");
    if (-1 == nameCol) { std::cout << "paramManager ProfTsaturation" << endl; }
    const float temp_saturation = query.value(nameCol).toFloat();
    d_saturation = temp_saturation;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_saturation = temp_saturation;
    }

    //Whether to convert to monochrome
    nameCol = rec.indexOf("ProfTmonochrome");
    if (-1 == nameCol) { std::cout << "paramManager ProfTmonochrome" << endl; }
    const bool temp_monochrome = query.value(nameCol).toBool();
    d_monochrome = temp_monochrome;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_monochrome = temp_monochrome;
    }

    //Red weight multiplier for b&w conversion
    nameCol = rec.indexOf("ProfTbwRmult");
    if (-1 == nameCol) { std::cout << "paramManager ProfTbwRmult" << endl; }
    const float temp_bwRmult = query.value(nameCol).toFloat();
    d_bwRmult = temp_bwRmult;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_bwRmult = temp_bwRmult;
    }

    //Green weight multiplier for b&w conversion
    nameCol = rec.indexOf("ProfTbwGmult");
    if (-1 == nameCol) { std::cout << "paramManager ProfTbwGmult" << endl; }
    const float temp_bwGmult = query.value(nameCol).toFloat();
    d_bwGmult = temp_bwGmult;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_bwGmult = temp_bwGmult;
    }

    //Blue weight multiplier for b&w conversion
    nameCol = rec.indexOf("ProfTbwBmult");
    if (-1 == nameCol) { std::cout << "paramManager ProfTbwBmult" << endl; }
    const float temp_bwBmult = query.value(nameCol).toFloat();
    d_bwBmult = temp_bwBmult;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_bwBmult = temp_bwBmult;
    }

    //Rotation
    if ("" == filename)
    {
        //There is no default rotation; it's just 0.
        d_rotation = 0;
        if (copyDefaults == CopyDefaults::loadToParams)
        {
            m_rotation = 0;
        }
    }
    else
    {
        d_rotation = exifDefaultRotation(absFilePath);
        if (copyDefaults == CopyDefaults::loadToParams)
        {
            m_rotation = d_rotation;
        }
    }
}


//This loads all of the processing params from the imageID into the param manager.
//TODO: for partial copying, make a loadParams that lets you select which ones
// you want. It'll have to load some sort of null for other things so that it doesn't write.
void ParameterManager::loadParams(QString imageID)
{
    //Once we're told what to load, then we're well past initialization
    justInitialized = false;
    QSqlRecord rec;
    int nameCol;

    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    QSqlQuery query(db);

    //tiffIn should be false.
    //For now. When we add tiff input, then it'll need to be different.
    m_tiffIn = false;

    //So should jpegIn.
    m_jpegIn = false;


    //Everything else can be pulled from sql.
    //This query will be shared.
    query.prepare("SELECT * FROM ProcessingTable WHERE ProcTprocID = ?;");
    query.bindValue(0, imageID);
    query.exec();
    query.first();
    rec = query.record();

    //First is auto CA correct.
    nameCol = rec.indexOf("ProcTcaEnabled");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcaEnabled" << endl; }
    const int temp_caEnabled = query.value(nameCol).toInt();
    if (temp_caEnabled != m_caEnabled)
    {
        //cout << "ParameterManager::loadParams caEnabled" << endl;
        m_caEnabled = temp_caEnabled;
        validity = min(validity, Valid::load);
    }

    //highlights (highlight recovery)
    nameCol = rec.indexOf("ProcThighlightRecovery");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightRecovery" << endl; }
    const int temp_highlights = query.value(nameCol).toInt();
    if (temp_highlights != m_highlights)
    {
        //cout << "ParameterManager::loadParams highlights" << endl;
        m_highlights = temp_highlights;
        validity = min(validity, Valid::load);
    }

    //Lensfun lens name
    nameCol = rec.indexOf("ProcTlensfunName");
    if (-1 == nameCol) { std::cout << "paramManager ProcTlensfunName" << endl; }
    const QString temp_lensfunName = query.value(nameCol).toString();
    if (temp_lensfunName != m_lensfunName)
    {
        //cout << "ParameterManager::loadParams lensfunName" << endl;
        m_lensfunName = temp_lensfunName;
        validity = min(validity, Valid::load);
    }

    //Lensfun CA correction
    nameCol = rec.indexOf("ProcTlensfunCa");
    if (-1 == nameCol) { std::cout << "paramManager ProcTlensfunCa" << endl; }
    const int temp_lensfunCa = query.value(nameCol).toInt();
    if (temp_lensfunCa != m_lensfunCa)
    {
        //cout << "ParameterManager::loadParams lensfunCa" << endl;
        m_lensfunCa = temp_lensfunCa;
        validity = min(validity, Valid::load);
    }

    //Lensfun vignetting correction
    nameCol = rec.indexOf("ProcTlensfunVign");
    if (-1 == nameCol) { std::cout << "paramManager ProcTlensfunVign" << endl; }
    const int temp_lensfunVign = query.value(nameCol).toInt();
    if (temp_lensfunVign != m_lensfunVign)
    {
        //cout << "ParameterManager::loadParams lensfunVign" << endl;
        m_lensfunVign = temp_lensfunVign;
        validity = min(validity, Valid::load);
    }

    //Lensfun distortion correction
    nameCol = rec.indexOf("ProcTlensfunDist");
    if (-1 == nameCol) { std::cout << "paramManager ProcTlensfunDist" << endl; }
    const int temp_lensfunDist = query.value(nameCol).toInt();
    if (temp_lensfunDist != m_lensfunDist)
    {
        //cout << "ParameterManager::loadParams lensfunDist" << endl;
        m_lensfunDist = temp_lensfunDist;
        validity = min(validity, Valid::load);
    }

    //Fine rotation angle
    nameCol = rec.indexOf("ProcTrotationAngle");
    if (-1 == nameCol) { std::cout << "paramManager ProcTrotationAngle" << endl; }
    const float temp_rotationAngle = query.value(nameCol).toFloat();
    if (temp_rotationAngle != m_rotationAngle)
    {
        //cout << "ParameterManager::loadParams rotationAngle" << endl;
        m_rotationAngle = temp_rotationAngle;
        validity = min(validity, Valid::load);
    }

    //Rotation reference point coordinates
    nameCol = rec.indexOf("ProcTrotationPointX");
    if (-1 == nameCol) { std::cout << "paramManager ProcTrotationPointX" << endl; }
    const float temp_rotationPointX = query.value(nameCol).toFloat();
    if (temp_rotationPointX != m_rotationPointX)
    {
        //cout << "ParameterManager::loadParams rotationPointX" << endl;
        m_rotationPointX = temp_rotationPointX;
        //the reference coordinates don't affect validity at all
    }
    nameCol = rec.indexOf("ProcTrotationPointY");
    if (-1 == nameCol) { std::cout << "paramManager ProcTrotationPointY" << endl; }
    const float temp_rotationPointY = query.value(nameCol).toFloat();
    if (temp_rotationPointY != m_rotationPointY)
    {
        //cout << "ParameterManager::loadParams rotationPointY" << endl;
        m_rotationPointY = temp_rotationPointY;
        //the reference coordinates don't affect validity at all
    }

    //Exposure compensation
    nameCol = rec.indexOf("ProcTexposureComp");
    if (-1 == nameCol) { std::cout << "paramManager ProcTexposureComp" << endl; }
    const float temp_exposureComp = query.value(nameCol).toFloat();
    if (temp_exposureComp != m_exposureComp)
    {
        //cout << "ParameterManager::loadParams exposureComp" << endl;
        m_exposureComp = temp_exposureComp;
        validity = min(validity, Valid::demosaic);
    }

    //Temperature
    nameCol = rec.indexOf("ProcTtemperature");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtemperature" << endl; }
    const float temp_temperature = query.value(nameCol).toFloat();
    if (temp_temperature != m_temperature)
    {
        //cout << "ParameterManager::loadParams temperature" << endl;
        m_temperature = temp_temperature;
        validity = min(validity, Valid::demosaic);
    }

    //Tint
    nameCol = rec.indexOf("ProcTtint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtint" << endl; }
    const float temp_tint = query.value(nameCol).toFloat();
    if (temp_tint != m_tint)
    {
        //cout << "ParameterManager::loadParams tint" << endl;
        m_tint = temp_tint;
        validity = min(validity, Valid::demosaic);
    }

    //Initial developer concentration
    nameCol = rec.indexOf("ProcTinitialDeveloperConcentration");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialDeveloperConcentration" << endl; }
    const float temp_initialDeveloperConcentration = query.value(nameCol).toFloat();
    if (temp_initialDeveloperConcentration != m_initialDeveloperConcentration)
    {
        //cout << "ParameterManager::loadParams initialDeveloperConcentration" << endl;
        m_initialDeveloperConcentration = temp_initialDeveloperConcentration;
        validity = min(validity, Valid::prefilmulation);
    }

    //Reservoir thickness
    nameCol = rec.indexOf("ProcTreservoirThickness");
    if (-1 == nameCol) { std::cout << "paramManager ProcTreservoirThickness" << endl; }
    const float temp_reservoirThickness = query.value(nameCol).toFloat();
    if (temp_reservoirThickness != m_reservoirThickness)
    {
        //cout << "ParameterManager::loadParams reservoirThickness" << endl;
        m_reservoirThickness = temp_reservoirThickness;
        validity = min(validity, Valid::prefilmulation);
    }

    //Active layer thickness
    nameCol = rec.indexOf("ProcTactiveLayerThickness");
    if (-1 == nameCol) { std::cout << "paramManager ProcTactiveLayerThickness" << endl; }
    const float temp_activeLayerThickness = query.value(nameCol).toFloat();
    if (temp_activeLayerThickness != m_activeLayerThickness)
    {
        //cout << "ParameterManager::loadParams activeLayerThickness" << endl;
        m_activeLayerThickness = temp_activeLayerThickness;
        validity = min(validity, Valid::prefilmulation);
    }

    //Crystals per pixel
    nameCol = rec.indexOf("ProcTcrystalsPerPixel");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcrystalsPerPixel" << endl; }
    const float temp_crystalsPerPixel = query.value(nameCol).toFloat();
    if (temp_crystalsPerPixel != m_crystalsPerPixel)
    {
        //cout << "ParameterManager::loadParams crystalsPerPixel" << endl;
        m_crystalsPerPixel = temp_crystalsPerPixel;
        validity = min(validity, Valid::prefilmulation);
    }

    //Initial crystal radius
    nameCol = rec.indexOf("ProcTinitialCrystalRadius");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialCrystalRadius" << endl; }
    const float temp_initialCrystalRadius = query.value(nameCol).toFloat();
    if (temp_initialCrystalRadius != m_initialCrystalRadius)
    {
        //cout << "ParameterManager::loadParams initialCrystalRadius" << endl;
        m_initialCrystalRadius = temp_initialCrystalRadius;
        validity = min(validity, Valid::prefilmulation);
    }

    //Initial silver salt area density
    nameCol = rec.indexOf("ProcTinitialSilverSaltDensity");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialSilverSaltDensity" << endl; }
    const float temp_initialSilverSaltDensity = query.value(nameCol).toFloat();
    if (temp_initialSilverSaltDensity != m_initialSilverSaltDensity)
    {
        //cout << "ParameterManager::loadParams initialSilverSaltDensity" << endl;
        m_initialSilverSaltDensity = temp_initialSilverSaltDensity;
        validity = min(validity, Valid::prefilmulation);
    }

    //Developer consumption rate constant
    nameCol = rec.indexOf("ProcTdeveloperConsumptionConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTdeveloperConsumptionConst" << endl; }
    const float temp_developerConsumptionConst = query.value(nameCol).toFloat();
    if (temp_developerConsumptionConst != m_developerConsumptionConst)
    {
        //cout << "ParameterManager::loadParams developerConsumptionConst" << endl;
        m_developerConsumptionConst = temp_developerConsumptionConst;
        validity = min(validity, Valid::prefilmulation);
    }

    //Crystal growth rate constant
    nameCol = rec.indexOf("ProcTcrystalGrowthConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcrystalGrowthConst" << endl; }
    const float temp_crystalGrowthConst = query.value(nameCol).toFloat();
    if (temp_crystalGrowthConst != m_crystalGrowthConst)
    {
        //cout << "ParameterManager::loadParams crystalGrowthConst" << endl;
        m_crystalGrowthConst = temp_crystalGrowthConst;
        validity = min(validity, Valid::prefilmulation);
    }

    //Silver halide consumption rate constant
    nameCol = rec.indexOf("ProcTsilverSaltConsumptionConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsilverSaltConsumptionConst" << endl; }
    const float temp_silverSaltConsumptionConst = query.value(nameCol).toFloat();
    if (temp_silverSaltConsumptionConst != m_silverSaltConsumptionConst)
    {
        //cout << "ParameterManager::loadParams silverSaltConsumptionConst" << endl;
        m_silverSaltConsumptionConst = temp_silverSaltConsumptionConst;
        validity = min(validity, Valid::prefilmulation);
    }

    //Total development time
    nameCol = rec.indexOf("ProcTtotalDevelopmentTime");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtotalDevelopmentTime" << endl; }
    const float temp_totalDevelopmentTime = query.value(nameCol).toFloat();
    if (temp_totalDevelopmentTime != m_totalDevelopmentTime)
    {
        //cout << "ParameterManager::loadParams totalDevelopmentTime" << endl;
        m_totalDevelopmentTime = temp_totalDevelopmentTime;
        validity = min(validity, Valid::prefilmulation);
    }

    //Number of agitations
    nameCol = rec.indexOf("ProcTagitateCount");
    if (-1 == nameCol) { std::cout << "paramManager ProcTagitateCount" << endl; }
    const int temp_agitateCount = query.value(nameCol).toInt();
    if (temp_agitateCount != m_agitateCount)
    {
        //cout << "ParameterManager::loadParams agitateCount" << endl;
        m_agitateCount = temp_agitateCount;
        validity = min(validity, Valid::prefilmulation);
    }

    //Number of simulation steps for development
    nameCol = rec.indexOf("ProcTdevelopmentSteps");
    if (-1 == nameCol) { std::cout << "paramManager ProcTdevelopmentSteps" << endl; }
    const int temp_developmentSteps = query.value(nameCol).toInt();
    if (temp_developmentSteps != m_developmentSteps)
    {
        //cout << "ParameterManager::loadParams developmentSteps" << endl;
        m_developmentSteps = temp_developmentSteps;
        validity = min(validity, Valid::prefilmulation);
    }

    //Area of film for the simulation
    nameCol = rec.indexOf("ProcTfilmArea");
    if (-1 == nameCol) { std::cout << "paramManager ProcTfilmArea" << endl; }
    const float temp_filmArea = query.value(nameCol).toFloat();
    if (temp_filmArea != m_filmArea)
    {
        //cout << "ParameterManager::loadParams filmArea" << endl;
        m_filmArea = temp_filmArea;
        validity = min(validity, Valid::prefilmulation);
    }

    //A constant for the size of the diffusion. It...affects the same thing as film area.
    nameCol = rec.indexOf("ProcTsigmaConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsigmaConst" << endl; }
    const float temp_sigmaConst = query.value(nameCol).toFloat();
    if (temp_sigmaConst != m_sigmaConst)
    {
        //cout << "ParameterManager::loadParams sigmaConst" << endl;
        m_sigmaConst = temp_sigmaConst;
        validity = min(validity, Valid::prefilmulation);
    }

    //Layer mix constant: the amount of active developer that gets exchanged with the reservoir.
    nameCol = rec.indexOf("ProcTlayerMixConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTlayerMixConst" << endl; }
    const float temp_layerMixConst = query.value(nameCol).toFloat();
    if (temp_layerMixConst != m_layerMixConst)
    {
        //cout << "ParameterManager::loadParams layerMixConst" << endl;
        m_layerMixConst = temp_layerMixConst;
        validity = min(validity, Valid::prefilmulation);
    }

    //Layer time divisor: Controls the relative intra-layer and inter-layer diffusion.
    nameCol = rec.indexOf("ProcTlayerTimeDivisor");
    if (-1 == nameCol) { std::cout << "paramManager ProcTlayerTimeDivisor" << endl; }
    const float temp_layerTimeDivisor = query.value(nameCol).toFloat();
    if (temp_layerTimeDivisor != m_layerTimeDivisor)
    {
        //cout << "ParameterManager::loadParams layerTimeDivisor" << endl;
        m_layerTimeDivisor = temp_layerTimeDivisor;
        validity = min(validity, Valid::prefilmulation);
    }

    //Rolloff boundary. This is where highlights start to roll off.
    nameCol = rec.indexOf("ProcTrolloffBoundary");
    if (-1 == nameCol) { std::cout << "paramManager ProcTrolloffBoundary" << endl; }
    const float temp_rolloffBoundary = query.value(nameCol).toFloat();
    if (temp_rolloffBoundary != m_rolloffBoundary)
    {
        //cout << "ParameterManager::loadParams rolloffBoundary" << endl;
        m_rolloffBoundary = temp_rolloffBoundary;
        validity = min(validity, Valid::prefilmulation);
    }

    //Toe boundary. This is the offset for the values where the toe starts to roll off.
    nameCol = rec.indexOf("ProcTtoeBoundary");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtoeBoundary" << endl; }
    const float temp_toeBoundary = query.value(nameCol).toFloat();
    if (temp_toeBoundary != m_toeBoundary)
    {
        //cout << "ParameterManager::loadParams toeBoundary" << endl;
        m_toeBoundary = temp_toeBoundary;
        validity = min(validity, Valid::prefilmulation);
    }

    //Post-filmulator black clipping point
    nameCol = rec.indexOf("ProcTblackpoint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTblackpoint" << endl; }
    const float temp_blackpoint = query.value(nameCol).toFloat();
    if (temp_blackpoint != m_blackpoint)
    {
        //cout << "ParameterManager::loadParams blackpoint" << endl;
        m_blackpoint = temp_blackpoint;
        validity = min(validity, Valid::filmulation);
    }

    //Post-filmulator white clipping point
    nameCol = rec.indexOf("ProcTwhitepoint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTwhitepoint" << endl; }
    const float temp_whitepoint = query.value(nameCol).toFloat();
    if (temp_whitepoint != m_whitepoint)
    {
        //cout << "ParameterManager::loadParams whitepoint" << endl;
        m_whitepoint = temp_whitepoint;
        validity = min(validity, Valid::filmulation);
    }

    //Height of the crop WRT image height
    nameCol = rec.indexOf("ProcTcropHeight");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcropHeight" << endl; }
    const float temp_cropHeight = query.value(nameCol).toFloat();
    if (temp_cropHeight != m_cropHeight)
    {
        //cout << "ParameterManager::loadParams cropHeight" << endl;
        m_cropHeight = temp_cropHeight;
        validity = min(validity, Valid::filmulation);
    }

    //Aspect ratio of the crop
    nameCol = rec.indexOf("ProcTcropAspect");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcropAspect" << endl; }
    const float temp_cropAspect = query.value(nameCol).toFloat();
    if (temp_cropAspect != m_cropAspect)
    {
        //cout << "ParameterManager::loadParams cropAspect" << endl;
        m_cropAspect = temp_cropAspect;
        validity = min(validity, Valid::filmulation);
    }

    //Vertical position offset relative to center, WRT image height
    nameCol = rec.indexOf("ProcTcropVoffset");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcropVoffset" << endl; }
    const float temp_cropVoffset = query.value(nameCol).toFloat();
    if (temp_cropVoffset != m_cropVoffset)
    {
        //cout << "ParameterManager::loadParams cropVoffset" << endl;
        m_cropVoffset = temp_cropVoffset;
        validity = min(validity, Valid::filmulation);
    }

    //Horizontal position offset relative to center, WRT image width
    nameCol = rec.indexOf("ProcTcropHoffset");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcropHoffset" << endl; }
    const float temp_cropHoffset = query.value(nameCol).toFloat();
    if (temp_cropHoffset != m_cropHoffset)
    {
        //cout << "ParameterManager::loadParams cropHoffset" << endl;
        m_cropHoffset = temp_cropHoffset;
        validity = min(validity, Valid::filmulation);
    }

    //Shadow control point x value
    nameCol = rec.indexOf("ProcTshadowsX");
    if (-1 == nameCol) { std::cout << "paramManager ProcTshadowsX" << endl; }
    const float temp_shadowsX = query.value(nameCol).toFloat();
    if (temp_shadowsX != m_shadowsX)
    {
        //cout << "ParameterManager::loadParams shadowsX" << endl;
        m_shadowsX = temp_shadowsX;
        validity = min(validity, Valid::blackwhite);
    }

    //Shadow control point y value
    nameCol = rec.indexOf("ProcTshadowsY");
    if (-1 == nameCol) { std::cout << "paramManager ProcTshadowsY" << endl; }
    const float temp_shadowsY = query.value(nameCol).toFloat();
    if (temp_shadowsY != m_shadowsY)
    {
        //cout << "ParameterManager::loadParams shadowsY" << endl;
        m_shadowsY = temp_shadowsY;
        validity = min(validity, Valid::blackwhite);
    }

    //Highlight control point x value
    nameCol = rec.indexOf("ProcThighlightsX");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightsX" << endl; }
    const float temp_highlightsX = query.value(nameCol).toFloat();
    if (temp_highlightsX != m_highlightsX)
    {
        //cout << "ParameterManager::loadParams highlightsX" << endl;
        m_highlightsX = temp_highlightsX;
        validity = min(validity, Valid::blackwhite);
    }

    //Highlight control point y value
    nameCol = rec.indexOf("ProcThighlightsY");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightsY" << endl; }
    const float temp_highlightsY = query.value(nameCol).toFloat();
    if (temp_highlightsY != m_highlightsY)
    {
        //cout << "ParameterManager::loadParams highlightsY" << endl;
        m_highlightsY = temp_highlightsY;
        validity = min(validity, Valid::blackwhite);
    }

    //Vibrance (saturation of less-saturated things)
    nameCol = rec.indexOf("ProcTvibrance");
    if (-1 == nameCol) { std::cout << "paramManager ProcTvibrance" << endl; }
    const float temp_vibrance = query.value(nameCol).toFloat();
    if (temp_vibrance != m_vibrance)
    {
        //cout << "ParameterManager::loadParams vibrance" << endl;
        m_vibrance = temp_vibrance;
        validity = min(validity, Valid::blackwhite);
    }

    //Saturation
    nameCol = rec.indexOf("ProcTsaturation");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsaturation" << endl; }
    const float temp_saturation = query.value(nameCol).toFloat();
    if (temp_saturation != m_saturation)
    {
        //cout << "ParameterManager::loadParams saturation" << endl;
        m_saturation = temp_saturation;
        validity = min(validity, Valid::blackwhite);
    }

    //Whether to convert to monochrome
    nameCol = rec.indexOf("ProcTmonochrome");
    if (-1 == nameCol) { std::cout << "paramManager ProcTmonochrome" << endl; }
    const bool temp_monochrome = query.value(nameCol).toBool();
    if (temp_monochrome != m_monochrome)
    {
        //cout << "ParameterManager::loadParams monochrome" << endl;
        m_monochrome = temp_monochrome;
        validity = min(validity, Valid::blackwhite);
    }

    //Red weight multiplier for b&w conversion
    nameCol = rec.indexOf("ProcTbwRmult");
    if (-1 == nameCol) { std::cout << "paramManager ProcTbwRmult" << endl; }
    const float temp_bwRmult = query.value(nameCol).toFloat();
    if (temp_bwRmult != m_bwRmult)
    {
        //cout << "ParameterManager::loadParams bwRmult" << endl;
        m_bwRmult = temp_bwRmult;
        validity = min(validity, Valid::blackwhite);
    }

    //Green weight multiplier for b&w conversion
    nameCol = rec.indexOf("ProcTbwGmult");
    if (-1 == nameCol) { std::cout << "paramManager ProcTbwGmult" << endl; }
    const float temp_bwGmult = query.value(nameCol).toFloat();
    if (temp_bwGmult != m_bwGmult)
    {
        //cout << "ParameterManager::loadParams bwGmult" << endl;
        m_bwGmult = temp_bwGmult;
        validity = min(validity, Valid::blackwhite);
    }

    //Blue weight multiplier for b&w conversion
    nameCol = rec.indexOf("ProcTbwBmult");
    if (-1 == nameCol) { std::cout << "paramManager ProcTbwBmult" << endl; }
    const float temp_bwBmult = query.value(nameCol).toFloat();
    if (temp_bwBmult != m_bwBmult)
    {
        //cout << "ParameterManager::loadParams bwBmult" << endl;
        m_bwBmult = temp_bwBmult;
        validity = min(validity, Valid::blackwhite);
    }

    //Rotation
    nameCol = rec.indexOf("ProcTrotation");
    if (-1 == nameCol) { std::cout << "paramManager ProcTrotation" << endl; }
    const int temp_rotation = query.value(nameCol).toInt();
    if (temp_rotation != m_rotation)
    {
        //cout << "ParameterManager::loadParams rotation" << endl;
        m_rotation = temp_rotation;
        validity = min(validity, Valid::filmulation);
    }
}

//This clones parameters from another manager.
//It's intended to be used for dual pipelines and dual parametermanagers
//It's a slot that updates stuff when the other param manager gets edited.
//The other param manager emits updateClone.
//
//This is very similar to selectImage but it doesn't have to worry about defaults at all
//
//When the parameter manager is a clone, then it cancels computation using
// changeMadeSinceCheck.
void ParameterManager::cloneParams(ParameterManager * sourceParams)
{
    QMutexLocker paramLocker(&paramMutex);//Make all the param changes happen together.
    disableParamChange();//Prevent aborting of computation.

    //Make sure that we always abort after any change while executing,
    // even if it's later in the pipeline,
    // because we want to redo the small preview immediately.
    if (isClone)
    {
        changeMadeSinceCheck = true;
    }

    //Load the image index
    const QString temp_imageIndex = sourceParams->getImageIndex();
    if (temp_imageIndex != imageIndex)
    {
        imageIndex = temp_imageIndex;
        validity = min(validity, Valid::none);
    }

    //do stuff to load filename and other file info; taken from selectImage
    QString tempString = imageIndex;
    tempString.truncate(32);//length of md5

    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    QSqlQuery query(db);
    query.prepare("SELECT \
                  FTfilePath,FTsensitivity,FTexposureTime,FTaperture,FTfocalLength,FTcameraMake,FTcameraModel \
                  FROM FileTable WHERE FTfileID = ?;");
    query.bindValue(0, QVariant(tempString));
    query.exec();
    query.first();

    //This will help us get the column index of the desired column name.
    int nameCol;
    QSqlRecord rec = query.record();

    //Filename. First is the full path for the image pipeline.
    nameCol = rec.indexOf("FTfilePath");
    if (-1 == nameCol) { std::cout << "paramManager FTfilePath" << endl; }
    QString name = query.value(nameCol).toString();
    m_fullFilename = name.toStdString();
    filename = name.right(name.size() - name.lastIndexOf(QString("/")) - 1);
    emit filenameChanged();

    nameCol = rec.indexOf("FTsensitivity");
    if (-1 == nameCol) { std::cout << "paramManager FTsensitivity" << endl; }
    sensitivity = query.value(nameCol).toInt();
    emit sensitivityChanged();

    nameCol = rec.indexOf("FTexposureTime");
    if (-1 == nameCol) { std::cout << "paramManager FTexposureTime" << endl; }
    QString expTimeTemp = query.value(nameCol).toString();
    bool okNum;
    unsigned int numerator = expTimeTemp.left(expTimeTemp.lastIndexOf("/")).toInt(&okNum, 10);
    bool okDen;
    unsigned int denominator = expTimeTemp.right(expTimeTemp.size() - expTimeTemp.lastIndexOf("/") - 1).toInt(&okDen, 10);
    if (okNum && okDen)
    {
        unsigned int divisor = gcd(numerator, denominator);
        numerator /= divisor;
        denominator /= divisor;
        double expTime = double(numerator)/double(denominator);
        if (expTime > 0.5)
        {
            exposureTime = QString::number(expTime, 'f', 1);
        }
        else
        {
            exposureTime = QString("%1/%2").arg(numerator).arg(denominator);
        }
    }
    else
    {
        exposureTime = expTimeTemp;
    }
    emit exposureTimeChanged();

    nameCol = rec.indexOf("FTaperture");
    if (-1 == nameCol) { std::cout << "paramManager FTaperture" << endl; }
    fnumber = query.value(nameCol).toFloat();
    if (fnumber >= 8)
    {
        aperture = QString::number(fnumber,'f',0);
    }
    else //fnumber < 8
    {
        aperture = QString::number(fnumber,'f',1);
    }
    emit apertureChanged();

    nameCol = rec.indexOf("FTfocalLength");
    if (-1 == nameCol) { std::cout << "paramManager FTfocalLength" << endl; }
    focalLength = query.value(nameCol).toFloat();
    emit focalLengthChanged();

    nameCol = rec.indexOf("FTcameraMake");
    if (-1 == nameCol) { std::cout << "paramManager FTcameraMake" << endl; }
    make = query.value(nameCol).toString();
    emit makeChanged();

    nameCol = rec.indexOf("FTcameraModel");
    if (-1 == nameCol) { std::cout << "paramManager FTcameraModel" << endl; }
    model = query.value(nameCol).toString();
    emit modelChanged();

    exifLensName = exifLens(m_fullFilename);
    emit exifLensNameChanged();

    //Now instead of loading the parameters from the database,
    // which might not have been written back to yet, we
    // copy from the sourceParams.

    //tiffIn should be false.
    const bool temp_tiffIn = sourceParams->getTiffIn();
    if (temp_tiffIn != m_tiffIn)
    {
        m_tiffIn = temp_tiffIn;
        validity = min(validity, Valid::none);
    }

    //So should jpegIn.
    const bool temp_jpegIn = sourceParams->getJpegIn();
    if (temp_jpegIn != m_jpegIn)
    {
        m_jpegIn = temp_jpegIn;
        validity = min(validity, Valid::none);
    }

    //Then is caEnabled.
    const int temp_caEnabled = sourceParams->getCaEnabled();
    if (temp_caEnabled != s_caEnabled)
    {
        //cout << "ParameterManager::cloneParams caEnabled" << endl;
        s_caEnabled = temp_caEnabled;
        validity = min(validity, Valid::load);
    }

    //Highlight recovery
    const int temp_highlights = sourceParams->getHighlights();
    if (temp_highlights != m_highlights)
    {
        //cout << "ParameterManager::cloneParams highlights" << endl;
        m_highlights = temp_highlights;
        validity = min(validity, Valid::load);
    }

    //Lensfun lens name
    const QString temp_lensfunName = sourceParams->getLensfunName();
    if (temp_lensfunName != s_lensfunName)
    {
        //cout << "ParameterManager::cloneParams lensfunName" << endl;
        s_lensfunName = temp_lensfunName;
        validity = min(validity, Valid::load);
    }

    //Lensfun CA correction
    const int temp_lensfunCa = sourceParams->getLensfunCa();
    if (temp_lensfunCa != s_lensfunCa)
    {
        //cout << "ParameterManager::cloneParams lensfunCa" << endl;
        s_lensfunCa = temp_lensfunCa;
        validity = min(validity, Valid::load);
    }

    //Lensfun vignetting correction
    const int temp_lensfunVign = sourceParams->getLensfunVign();
    if (temp_lensfunVign != s_lensfunVign)
    {
        //cout << "ParameterManager::cloneParams lensfunVign" << endl;
        s_lensfunVign = temp_lensfunVign;
        validity = min(validity, Valid::load);
    }

    //Lensfun distortion correction
    const int temp_lensfunDist = sourceParams->getLensfunDist();
    if (temp_lensfunDist != s_lensfunDist)
    {
        //cout << "ParameterManager::cloneParams lensfunDist" << endl;
        s_lensfunDist = temp_lensfunDist;
        validity = min(validity, Valid::load);
    }

    //Fine rotation angle
    const float temp_rotationAngle = sourceParams->getRotationAngle();
    if (temp_rotationAngle != m_rotationAngle)
    {
        //cout << "ParameterManager::cloneParams rotationAngle" << endl;
        m_rotationAngle = temp_rotationAngle;
        validity = min(validity, Valid::load);
    }

    //Rotation reference point coordinates
    const float temp_rotationPointX = sourceParams->getRotationPointX();
    if (temp_rotationPointX != m_rotationPointX)
    {
        //cout << "ParameterManager::cloneParams rotationPointX" << endl;
        m_rotationPointX = temp_rotationPointX;
        validity = min(validity, Valid::load);
    }
    const float temp_rotationPointY = sourceParams->getRotationPointY();
    if (temp_rotationPointY != m_rotationPointY)
    {
        //cout << "ParameterManager::cloneParams rotationPointY" << endl;
        m_rotationPointY = temp_rotationPointY;
        validity = min(validity, Valid::load);
    }

    //Exposure compensation
    const float temp_exposureComp = sourceParams->getExposureComp();
    if (temp_exposureComp != m_exposureComp)
    {
        //cout << "ParameterManager::cloneParams exposureComp" << endl;
        m_exposureComp = temp_exposureComp;
        validity = min(validity, Valid::demosaic);
    }

    //Temperature
    const float temp_temperature = sourceParams->getTemperature();
    if (temp_temperature != m_temperature)
    {
        //cout << "ParameterManager::cloneParams temperature" << endl;
        m_temperature = temp_temperature;
        validity = min(validity, Valid::demosaic);
    }

    //Tint
    const float temp_tint = sourceParams->getTint();
    if (temp_tint != m_tint)
    {
        //cout << "ParameterManager::cloneParams tint" << endl;
        m_tint = temp_tint;
        validity = min(validity, Valid::demosaic);
    }

    //Initial developer concentration
    const float temp_initialDeveloperConcentration = sourceParams->getInitialDeveloperConcentration();
    if (temp_initialDeveloperConcentration != m_initialDeveloperConcentration)
    {
        //cout << "ParameterManager::cloneParams initialDeveloperConcentration" << endl;
        m_initialDeveloperConcentration = temp_initialDeveloperConcentration;
        validity = min(validity, Valid::prefilmulation);
    }

    //Reservoir thickness
    const float temp_reservoirThickness = sourceParams->getReservoirThickness();
    if (temp_reservoirThickness != m_reservoirThickness)
    {
        //cout << "ParameterManager::cloneParams reservoirThickness" << endl;
        m_reservoirThickness = temp_reservoirThickness;
        validity = min(validity, Valid::prefilmulation);
    }

    //Active layer thickness
    const float temp_activeLayerThickness = sourceParams->getActiveLayerThickness();
    if (temp_activeLayerThickness != m_activeLayerThickness)
    {
        //cout << "ParameterManager::cloneParams activeLayerThickness" << endl;
        m_activeLayerThickness = temp_activeLayerThickness;
        validity = min(validity, Valid::prefilmulation);
    }

    //Crystals per pixel
    const float temp_crystalsPerPixel = sourceParams->getCrystalsPerPixel();
    if (temp_crystalsPerPixel != m_crystalsPerPixel)
    {
        //cout << "ParameterManager::cloneParams crystalsPerPixel" << endl;
        m_crystalsPerPixel = temp_crystalsPerPixel;
        validity = min(validity, Valid::prefilmulation);
    }

    //Initial crystal radius
    const float temp_initialCrystalRadius = sourceParams->getInitialCrystalRadius();
    if (temp_initialCrystalRadius != m_initialCrystalRadius)
    {
        //cout << "ParameterManager::cloneParams initialCrystalRadius" << endl;
        m_initialCrystalRadius = temp_initialCrystalRadius;
        validity = min(validity, Valid::prefilmulation);
    }

    //Initial silver salt area density
    const float temp_initialSilverSaltDensity = sourceParams->getInitialSilverSaltDensity();
    if (temp_initialSilverSaltDensity != m_initialSilverSaltDensity)
    {
        //cout << "ParameterManager::cloneParams initialSilverSaltDensity" << endl;
        m_initialSilverSaltDensity = temp_initialSilverSaltDensity;
        validity = min(validity, Valid::prefilmulation);
    }

    //Developer consumption rate constant
    const float temp_developerConsumptionConst = sourceParams->getDeveloperConsumptionConst();
    if (temp_developerConsumptionConst != m_developerConsumptionConst)
    {
        //cout << "ParameterManager::cloneParams developerConsumptionConst" << endl;
        m_developerConsumptionConst = temp_developerConsumptionConst;
        validity = min(validity, Valid::prefilmulation);
    }

    //Crystal growth rate constant
    const float temp_crystalGrowthConst = sourceParams->getCrystalGrowthConst();
    if (temp_crystalGrowthConst != m_crystalGrowthConst)
    {
        //cout << "ParameterManager::cloneParams crystalGrowthConst" << endl;
        m_crystalGrowthConst = temp_crystalGrowthConst;
        validity = min(validity, Valid::prefilmulation);
    }

    //Silver halide consumption rate constant
    const float temp_silverSaltConsumptionConst = sourceParams->getSilverSaltConsumptionConst();
    if (temp_silverSaltConsumptionConst != m_silverSaltConsumptionConst)
    {
        //cout << "ParameterManager::cloneParams silverSaltConsumptionConst" << endl;
        m_silverSaltConsumptionConst = temp_silverSaltConsumptionConst;
        validity = min(validity, Valid::prefilmulation);
    }

    //Total development time
    const float temp_totalDevelopmentTime = sourceParams->getTotalDevelopmentTime();
    if (temp_totalDevelopmentTime != m_totalDevelopmentTime)
    {
        //cout << "ParameterManager::cloneParams totalDevelopmentTime" << endl;
        m_totalDevelopmentTime = temp_totalDevelopmentTime;
        validity = min(validity, Valid::prefilmulation);
    }

    //Number of agitations
    const int temp_agitateCount = sourceParams->getAgitateCount();
    if (temp_agitateCount != m_agitateCount)
    {
        //cout << "ParameterManager::cloneParams agitateCount" << endl;
        m_agitateCount = temp_agitateCount;
        validity = min(validity, Valid::prefilmulation);
    }

    //Number of simulation steps for development
    const int temp_developmentSteps = sourceParams->getDevelopmentSteps();
    if (temp_developmentSteps != m_developmentSteps)
    {
        //cout << "ParameterManager::cloneParams developmentSteps" << endl;
        m_developmentSteps = temp_developmentSteps;
        validity = min(validity, Valid::prefilmulation);
    }

    //Area of film for the simulation
    const float temp_filmArea = sourceParams->getFilmArea();
    if (temp_filmArea != m_filmArea)
    {
        //cout << "ParameterManager::cloneParams filmArea" << endl;
        m_filmArea = temp_filmArea;
        validity = min(validity, Valid::prefilmulation);
    }

    //A constant for the size of the diffusion. It...affects the same thing as film area.
    const float temp_sigmaConst = sourceParams->getSigmaConst();
    if (temp_sigmaConst != m_sigmaConst)
    {
        //cout << "ParameterManager::cloneParams sigmaConst" << endl;
        m_sigmaConst = temp_sigmaConst;
        validity = min(validity, Valid::prefilmulation);
    }

    //Layer mix constant: the amount of active developer that gets exchanged with the reservoir.
    const float temp_layerMixConst = sourceParams->getLayerMixConst();
    if (temp_layerMixConst != m_layerMixConst)
    {
        //cout << "ParameterManager::cloneParams layerMixConst" << endl;
        m_layerMixConst = temp_layerMixConst;
        validity = min(validity, Valid::prefilmulation);
    }

    //Layer time divisor: Controls the relative intra-layer and inter-layer diffusion.
    const float temp_layerTimeDivisor = sourceParams->getLayerTimeDivisor();
    if (temp_layerTimeDivisor != m_layerTimeDivisor)
    {
        //cout << "ParameterManager::cloneParams layerTimeDivisor" << endl;
        m_layerTimeDivisor = temp_layerTimeDivisor;
        validity = min(validity, Valid::prefilmulation);
    }

    //Rolloff boundary. This is where highlights start to roll off.
    const float temp_rolloffBoundary = sourceParams->getRolloffBoundary();
    if (temp_rolloffBoundary != m_rolloffBoundary)
    {
        //cout << "ParameterManager::cloneParams rolloffBoundary" << endl;
        m_rolloffBoundary = temp_rolloffBoundary;
        validity = min(validity, Valid::prefilmulation);
    }

    //Toe boundary. This is the offset for the values where the toe starts to roll off.
    const float temp_toeBoundary = sourceParams->getToeBoundary();
    if (temp_toeBoundary != m_toeBoundary)
    {
        //cout << "ParameterManager::cloneParams toeBoundary" << endl;
        m_toeBoundary = temp_toeBoundary;
        validity = min(validity, Valid::prefilmulation);
    }

    //Post-filmulator black clipping point
    const float temp_blackpoint = sourceParams->getBlackpoint();
    if (temp_blackpoint != m_blackpoint)
    {
        //cout << "ParameterManager::cloneParams blackpoint" << endl;
        m_blackpoint = temp_blackpoint;
        validity = min(validity, Valid::filmulation);
    }

    //Post-filmulator white clipping point
    const float temp_whitepoint = sourceParams->getWhitepoint();
    if (temp_whitepoint != m_whitepoint)
    {
        //cout << "ParameterManager::cloneParams whitepoint" << endl;
        m_whitepoint = temp_whitepoint;
        validity = min(validity, Valid::filmulation);
    }

    //Height of the crop WRT image height
    const float temp_cropHeight = sourceParams->getCropHeight();
    if (temp_cropHeight != m_cropHeight)
    {
        //cout << "ParameterManager::cloneParams cropHeight" << endl;
        m_cropHeight = temp_cropHeight;
        validity = min(validity, Valid::filmulation);
    }

    //Aspect ratio of the crop
    const float temp_cropAspect = sourceParams->getCropAspect();
    if (temp_cropAspect != m_cropAspect)
    {
        //cout << "ParameterManager::cloneParams cropAspect" << endl;
        m_cropAspect = temp_cropAspect;
        validity = min(validity, Valid::filmulation);
    }

    //Vertical position offset relative to center, WRT image height
    const float temp_cropVoffset = sourceParams->getCropVoffset();
    if (temp_cropVoffset != m_cropVoffset)
    {
        //cout << "ParameterManager::cloneParams cropVoffset" << endl;
        m_cropVoffset = temp_cropVoffset;
        validity = min(validity, Valid::filmulation);
    }

    //Horizontal position offset relative to center, WRT image width
    const float temp_cropHoffset = sourceParams->getCropHoffset();
    if (temp_cropHoffset != m_cropHoffset)
    {
        //cout << "ParameterManager::cloneParams cropHoffset" << endl;
        m_cropHoffset = temp_cropHoffset;
        validity = min(validity, Valid::filmulation);
    }

    //Shadow control point x value
    const float temp_shadowsX = sourceParams->getShadowsX();
    if (temp_shadowsX != m_shadowsX)
    {
        //cout << "ParameterManager::cloneParams shadowsX" << endl;
        m_shadowsX = temp_shadowsX;
        validity = min(validity, Valid::blackwhite);
    }

    //Shadow control point y value
    const float temp_shadowsY = sourceParams->getShadowsY();
    if (temp_shadowsY != m_shadowsY)
    {
        //cout << "ParameterManager::cloneParams shadowsY" << endl;
        m_shadowsY = temp_shadowsY;
        validity = min(validity, Valid::blackwhite);
    }

    //Highlight control point x value
    const float temp_highlightsX = sourceParams->getHighlightsX();
    if (temp_highlightsX != m_highlightsX)
    {
        //cout << "ParameterManager::cloneParams highlightsX" << endl;
        m_highlightsX = temp_highlightsX;
        validity = min(validity, Valid::blackwhite);
    }

    //Highlight control point y value
    const float temp_highlightsY = sourceParams->getHighlightsY();
    if (temp_highlightsY != m_highlightsY)
    {
        //cout << "ParameterManager::cloneParams highlightsY" << endl;
        m_highlightsY = temp_highlightsY;
        validity = min(validity, Valid::blackwhite);
    }

    //Vibrance (saturation of less-saturated things)
    const float temp_vibrance = sourceParams->getVibrance();
    if (temp_vibrance != m_vibrance)
    {
        //cout << "ParameterManager::cloneParams vibrance" << endl;
        m_vibrance = temp_vibrance;
        validity = min(validity, Valid::blackwhite);
    }

    //Saturation
    const float temp_saturation = sourceParams->getSaturation();
    if (temp_saturation != m_saturation)
    {
        //cout << "ParameterManager::cloneParams saturation" << endl;
        m_saturation = temp_saturation;
        validity = min(validity, Valid::blackwhite);
    }

    //Whether to convert to monochrome
    const bool temp_monochrome = sourceParams->getMonochrome();
    if (temp_monochrome != m_monochrome)
    {
        //cout << "ParameterManager::cloneParams monochrome" << endl;
        m_monochrome = temp_monochrome;
        validity = min(validity, Valid::blackwhite);
    }

    //Red weight multiplier for b&w conversion
    const float temp_bwRmult = sourceParams->getBwRmult();
    if (temp_bwRmult != m_bwRmult)
    {
        //cout << "ParameterManager::cloneParams bwRmult" << endl;
        m_bwRmult = temp_bwRmult;
        validity = min(validity, Valid::blackwhite);
    }

    //Green weight multiplier for b&w conversion
    const float temp_bwGmult = sourceParams->getBwGmult();
    if (temp_bwGmult != m_bwGmult)
    {
        //cout << "ParameterManager::cloneParams bwGmult" << endl;
        m_bwGmult = temp_bwGmult;
        validity = min(validity, Valid::blackwhite);
    }

    //Blue weight multiplier for b&w conversion
    const float temp_bwBmult = sourceParams->getBwBmult();
    if (temp_bwBmult != m_bwBmult)
    {
        //cout << "ParameterManager::cloneParams bwBmult" << endl;
        m_bwBmult = temp_bwBmult;
        validity = min(validity, Valid::blackwhite);
    }

    //Rotation
    const int temp_rotation = sourceParams->getRotation();
    if (temp_rotation != m_rotation)
    {
        //cout << "ParameterManager::cloneParams rotation" << endl;
        m_rotation = temp_rotation;
        validity = min(validity, Valid::filmulation);
    }

    enableParamChange();//Re-enable updating of the image.
    paramChangeWrapper(QString("cloneParams"));
}

void ParameterManager::cancelComputation()
{
    changeMadeSinceCheck = true;
}

//This prevents the back-and-forth between this object and QML from aborting
// computation, and also prevents the sliders' moving from marking the photo
// as edited.
void ParameterManager::disableParamChange()
{
    paramChangeEnabled = false;
}
void ParameterManager::enableParamChange()
{
    paramChangeEnabled = true;
}
void ParameterManager::paramChangeWrapper(QString source)
{
    if (paramChangeEnabled)
    {
        emit paramChanged(source);

        //update any follower parametermanagers
        emit updateClone(this);
        if (QString("selectImage") == source) {
            emit updateImage(true);// it is a new image
        }
        else
        {
            emit updateImage(false);
        }
    }
    copyFromImageIndex = "";
    pasteable = false;
    emit pasteableChanged();
}

//This is for copying and pasting.
//TODO: think about what lens corrections means for copy/paste
//probably:
// copy on/off preferences, not correction parameters nor lens name
void ParameterManager::copyAll(QString fromImageID)
{
    std::cout << "copy all" << std::endl;
    copyFromImageIndex = fromImageID;
    pasteable = true;
    pasteSome = false;
    emit pasteableChanged();
}

void ParameterManager::paste(QString toImageID)
{
    if (pasteable)
    {
        if (!pasteSome)
        {
            ParameterManager tempParams;
            tempParams.loadParams(copyFromImageIndex);
            tempParams.writeToDB(toImageID);
            //The tempParams do not have their updateTableOut connected.
            emit updateTableOut("ProcessingTable", 0);//0 means edit
            emit updateTableOut("SearchTable", 0);//0 means edit
            emit updateTableOut("QueueTable", 0);//0 means edit
        }
        else// we only want to copy some of the parameters.
        {
            //ParameterManager tempParams;
            //tempParams.loadParams(copyFromImageIndex);

            //do something to only copy some of them.
            //tempParams.writeSomeToDB(some,toImageID);
        }
    }
}

void ParameterManager::updateLensfunAvailability()
{
    cout << "Updating availability" << endl;
    std::string camModel = model.toStdString();
    const lfCamera * camera = NULL;
    const lfCamera ** cameraList = ldb->FindCamerasExt(NULL, camModel.c_str());
    if (cameraList)
    {
        //If the lens name starts with a backslash, don't filter by camera
        QString temp_lensfunName = s_lensfunName;
        if (temp_lensfunName.length() > 0)
        {
            if (temp_lensfunName.front() == "\\")
            {
                temp_lensfunName.remove(0,1);
            } else {
                camera = cameraList[0];
            }
        }
        const float cropFactor = cameraList[0]->CropFactor;
        const std::string lensModel = temp_lensfunName.toStdString();
        if (s_lensfunName.length() > 0)
        {
            const lfLens ** lensList = ldb->FindLenses(camera, NULL, lensModel.c_str());
            if (lensList)
            {
                //Check if sensor is monochrome
                const bool isCR3 = fullFilenameQstr.endsWith(".cr3", Qt::CaseInsensitive);
                bool isMonochrome = false;
                if (!isCR3) //no CR3 cameras are monochrome
                {
                    cout << "updateAvailability exiv filename: " << m_fullFilename << endl;
                    auto exifImage = Exiv2::ImageFactory::open(m_fullFilename);
                    exifImage->readMetadata();
                    Exiv2::ExifData exifData = exifImage->exifData();
                    std::string wb = exifData["Exif.Photo.WhiteBalance"].toString();
                    isMonochrome = wb.length()==0;
                }

                auto calibrationSet = lensList[0]->GetCalibrationSets();

                int i = 0;
                while (calibrationSet[i])
                {
                    auto c = calibrationSet[i];
                    const float r = cropFactor / c->Attributes.CropFactor;
                    if ((r >= 0.96) || (cropFactor < 1e-6f))
                    {
                        lensfunCaAvail   = (c->HasTCA() && !isMonochrome);
                        lensfunVignAvail = (c->HasVignetting());
                        lensfunDistAvail = (c->HasDistortion());
                    }
                    i++;
                }

                //The latter function is only available in lensfun master, not lensfun 3.95
                // So I duplicated its functionality above.
                //const int availableMods = lensList[0]->AvailableModifications(cropFactor);
                //lensfunCaAvail   = (availableMods & LF_MODIFY_TCA) && !isMonochrome;
                //lensfunVignAvail = availableMods & LF_MODIFY_VIGNETTING;
                //lensfunDistAvail = availableMods & LF_MODIFY_DISTORTION;
                emit lensfunCaAvailChanged();
                emit lensfunVignAvailChanged();
                emit lensfunDistAvailChanged();
            } else {
                //If there is no matching lens, we can't do any corrections
                //This shouldn't really happen because either it'll be empty or
                // there will be a real lens selected by the UI.
                lensfunCaAvail = false;
                lensfunVignAvail = false;
                lensfunDistAvail = false;
                emit lensfunCaAvailChanged();
                emit lensfunVignAvailChanged();
                emit lensfunDistAvailChanged();
            }
            lf_free(lensList);
        } else {
            //If there is no lens selected, we can't do any corrections
            lensfunCaAvail = false;
            lensfunVignAvail = false;
            lensfunDistAvail = false;
            emit lensfunCaAvailChanged();
            emit lensfunVignAvailChanged();
            emit lensfunDistAvailChanged();
        }
    } else {
        //If we don't know the crop factor, we can't do any corrections
        lensfunCaAvail = false;
        lensfunVignAvail = false;
        lensfunDistAvail = false;
        emit lensfunCaAvailChanged();
        emit lensfunVignAvailChanged();
        emit lensfunDistAvailChanged();
    }
    lf_free(cameraList);
}

void ParameterManager::setLensPreferences()
{
    QSqlQuery query;
    query.exec("BEGIN TRANSACTION;");

    //First we delete anything matching
    query.prepare("DELETE FROM LensPrefs WHERE ExifCamera = ? AND ExifLens = ?");
    query.bindValue(0, model);
    query.bindValue(1, exifLensName);
    query.exec();

    //Now we insert a fresh entry
    query.prepare("INSERT INTO LensPrefs ("
                  "ExifCamera, "
                  "ExifLens, "
                  "LensfunLens, "
                  "LensfunCa, "
                  "LensfunVign, "
                  "LensfunDist, "
                  "AutoCa) "
                  "VALUES (?,?,?,?,?,?,?);");
    query.bindValue(0, model);
    query.bindValue(1, exifLensName);
    query.bindValue(2, s_lensfunName);
    query.bindValue(3, s_lensfunCa);
    query.bindValue(4, s_lensfunVign);
    query.bindValue(5, s_lensfunDist);
    query.bindValue(6, s_caEnabled);
    query.exec();

    query.exec("END TRANSACTION;");
}

void ParameterManager::eraseLensPreferences()
{
    QSqlQuery query;
    query.exec("BEGIN TRANSACTION;");

    //All we do is delete anything matching
    query.prepare("DELETE FROM LensPrefs WHERE ExifCamera = ? AND ExifLens = ?");
    query.bindValue(0, model);
    query.bindValue(1, exifLensName);
    query.exec();

    query.exec("END TRANSACTION;");
}

void ParameterManager::updateCustomWbAvailability()
{
    QString makemodel = make;
    makemodel.append(model);
    customWbAvail = false;
    for (uint64 i = 0; i < wbList.size(); i++)
    {
        const QString currModel = std::get<0>(wbList.at(i));
        if (currModel == makemodel)
        {
            customWbAvail = true;
        }
    }
    emit customWbAvailChanged();
}

void ParameterManager::saveCustomWb()
{
    QString makemodel = make;
    makemodel.append(model);
    int index = -1;
    for (uint64 i = 0; i < wbList.size(); i++)
    {
        const QString currModel = std::get<0>(wbList.at(i));
        if (currModel == makemodel)
        {
            index = i;
        }
    }

    std::tuple<QString, float, float> wbEntry = std::tie(makemodel, m_temperature, m_tint);

    if (index >= 0) //it was already in the list
    {
        wbList.at(index) = wbEntry;
    } else {
        wbList.push_back(wbEntry);
    }

    customWbAvail = true;
    emit customWbAvailChanged();
}

void ParameterManager::recallCustomWb()
{
    QString makemodel = make;
    makemodel.append(model);
    for (uint64 i = 0; i < wbList.size(); i++)
    {
        const QString currModel = std::get<0>(wbList.at(i));
        if (currModel == makemodel)
        {
            const float temp = std::get<1>(wbList.at(i));
            const float tint = std::get<2>(wbList.at(i));
            setWB(temp, tint);
        }
    }
}
