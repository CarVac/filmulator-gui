#include "parameterManager.h"

using std::min;
using std::cout;
using std::endl;

ParameterManager::ParameterManager() : QObject(0)
{
    paramChangeEnabled = true;

    //Load the defaults, copy to the parameters, there's no filename yet.
    loadDefaults(CopyDefaults::loadToParams, "");

    validity = Valid::none;

    pasteable = false;
    pasteSome = false;
}

std::tuple<Valid,AbortStatus,LoadParams> ParameterManager::claimLoadParams()
{
    QMutexLocker paramLocker(&paramMutex);
    AbortStatus abort;
    if (validity < Valid::none)//If something earlier than this has changed
    {
        abort = AbortStatus::restart;//not actually possible
    }
    else
    {
        abort = AbortStatus::proceed;
        validity = Valid::load;//mark it as started
    }
    LoadParams params;
    params.fullFilename = m_fullFilename;
    params.tiffIn = m_tiffIn;
    params.jpegIn = m_jpegIn;
    std::tuple<Valid,AbortStatus,LoadParams> tup(validity, abort, params);
    return tup;
}

void ParameterManager::setTiffIn(bool tiffIn)
{
    QMutexLocker paramLocker(&paramMutex);
    m_tiffIn = tiffIn;
    validity = min(validity, Valid::none);
    paramLocker.unlock();
    emit tiffInChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setTiff"));
}

void ParameterManager::setJpegIn(bool jpegIn)
{
    QMutexLocker paramLocker(&paramMutex);
    m_jpegIn = jpegIn;
    validity = min(validity, Valid::none);
    paramLocker.unlock();
    emit jpegInChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setJpeg"));
}

std::tuple<Valid,AbortStatus,DemosaicParams> ParameterManager::claimDemosaicParams()
{
    QMutexLocker paramLocker(&paramMutex);
    AbortStatus abort;
    if (validity < Valid::load)
    {
        abort = AbortStatus::restart;
    }
    else
    {
        abort = AbortStatus::proceed;
        validity = Valid::demosaic;//mark it as started
    }
    DemosaicParams params;
    params.caEnabled = m_caEnabled;
    params.highlights = m_highlights;
    std::tuple<Valid,AbortStatus,DemosaicParams> tup(validity, abort, params);
    return tup;
}

void ParameterManager::setCaEnabled(bool caEnabled)
{
    QMutexLocker paramLocker(&paramMutex);
    m_caEnabled = caEnabled;
    validity = min(validity, Valid::load);
    paramLocker.unlock();
    emit caEnabledChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setCaEnabled"));
}

void ParameterManager::setHighlights(int highlights)
{
    QMutexLocker paramLocker(&paramMutex);
    m_highlights = highlights;
    validity = min(validity, Valid::load);
    paramLocker.unlock();
    emit highlightsChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setHighlights"));
}

std::tuple<Valid,AbortStatus,PrefilmParams> ParameterManager::claimPrefilmParams()
{
    QMutexLocker paramLocker(&paramMutex);
    AbortStatus abort;
    if (validity < Valid::demosaic)
    {
        abort = AbortStatus::restart;
    }
    else
    {
        abort = AbortStatus::proceed;
        validity = Valid::prefilmulation;//mark it as started
    }
    PrefilmParams params;
    params.exposureComp = m_exposureComp;
    params.temperature = m_temperature;
    params.tint = m_tint;
    params.fullFilename = m_fullFilename;//it's okay to include previous things in later params if necessary
    std::tuple<Valid,AbortStatus,PrefilmParams> tup(validity, abort, params);
    return tup;
}

void ParameterManager::setExposureComp(float exposureComp)
{
    QMutexLocker paramLocker(&paramMutex);
    m_exposureComp = exposureComp;
    validity = min(validity, Valid::demosaic);
    paramLocker.unlock();
    emit exposureCompChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setExposureComp"));
}

void ParameterManager::setTemperature(float temperature)
{
    QMutexLocker paramLocker(&paramMutex);
    m_temperature = temperature;
    validity = min(validity, Valid::demosaic);
    paramLocker.unlock();
    emit temperatureChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setTemperature"));
}

void ParameterManager::setTint(float tint)
{
    QMutexLocker paramLocker(&paramMutex);
    m_tint = tint;
    validity = min(validity, Valid::demosaic);
    paramLocker.unlock();
    emit tintChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setTint"));
}

std::tuple<Valid,AbortStatus,FilmParams> ParameterManager::claimFilmParams(FilmFetch fetch)
{
    QMutexLocker paramLocker(&paramMutex);
    AbortStatus abort;

    //If it's the first time, the source data is from prefilmulation.
    if (fetch == FilmFetch::initial && validity < Valid::prefilmulation)
    {
        abort = AbortStatus::restart;
    }
    //If it's not the first time, the source data is from the last filmulation round
    // and will be invalidated by a filmulation param being modified.
    else if (fetch == FilmFetch::subsequent && validity < Valid::filmulation)
    {
        abort = AbortStatus::restart;
    }
    else
    {
        abort = AbortStatus::proceed;
        validity = Valid::filmulation;//mark it as started
    }
    FilmParams params;
    params.initialDeveloperConcentration = m_initialDeveloperConcentration,
    params.reservoirThickness = m_reservoirThickness,
    params.activeLayerThickness = m_activeLayerThickness,
    params.crystalsPerPixel = m_crystalsPerPixel,
    params.initialCrystalRadius = m_initialCrystalRadius,
    params.initialSilverSaltDensity = m_initialSilverSaltDensity,
    params.developerConsumptionConst = m_developerConsumptionConst,
    params.crystalGrowthConst = m_crystalGrowthConst,
    params.silverSaltConsumptionConst = m_silverSaltConsumptionConst,
    params.totalDevelopmentTime = m_totalDevelopmentTime,
    params.agitateCount = m_agitateCount,
    params.developmentSteps = m_developmentSteps,
    params.filmArea = m_filmArea,
    params.sigmaConst = m_sigmaConst,
    params.layerMixConst = m_layerMixConst,
    params.layerTimeDivisor = m_layerTimeDivisor,
    params.rolloffBoundary = m_rolloffBoundary;
    std::tuple<Valid,AbortStatus,FilmParams> tup(validity,abort, params);
    return tup;
}

void ParameterManager::setInitialDeveloperConcentration(float initialDeveloperConcentration)
{
    QMutexLocker paramLocker(&paramMutex);
    m_initialDeveloperConcentration = initialDeveloperConcentration;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit initialDeveloperConcentrationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setInitialDeveloperConcentration"));
}

void ParameterManager::setReservoirThickness(float reservoirThickness)
{
    QMutexLocker paramLocker(&paramMutex);
    m_reservoirThickness = reservoirThickness;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit reservoirThicknessChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setReservoirThickness"));
}

void ParameterManager::setActiveLayerThickness(float activeLayerThickness)
{
    QMutexLocker paramLocker(&paramMutex);
    m_activeLayerThickness = activeLayerThickness;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit activeLayerThicknessChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setActiveLayerThickness"));
}

void ParameterManager::setCrystalsPerPixel(float crystalsPerPixel)
{
    QMutexLocker paramLocker(&paramMutex);
    m_crystalsPerPixel = crystalsPerPixel;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit crystalsPerPixelChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setCrystalsPerPixel"));
}

void ParameterManager::setInitialCrystalRadius(float initialCrystalRadius)
{
    QMutexLocker paramLocker(&paramMutex);
    m_initialCrystalRadius = initialCrystalRadius;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit initialCrystalRadiusChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setInitialCrystalRadius"));
}

void ParameterManager::setInitialSilverSaltDensity(float initialSilverSaltDensity)
{
    QMutexLocker paramLocker(&paramMutex);
    m_initialSilverSaltDensity = initialSilverSaltDensity;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit initialSilverSaltDensityChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setInitialSilverSaltDensity"));
}

void ParameterManager::setDeveloperConsumptionConst(float developerConsumptionConst)
{
    QMutexLocker paramLocker(&paramMutex);
    m_developerConsumptionConst = developerConsumptionConst;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit developerConsumptionConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setDeveloperConsumptionConst"));
}

void ParameterManager::setCrystalGrowthConst(float crystalGrowthConst)
{
    QMutexLocker paramLocker(&paramMutex);
    m_crystalGrowthConst = crystalGrowthConst;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit crystalGrowthConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setCrystalGrowthConst"));
}

void ParameterManager::setSilverSaltConsumptionConst(float silverSaltConsumptionConst)
{
    QMutexLocker paramLocker(&paramMutex);
    m_silverSaltConsumptionConst = silverSaltConsumptionConst;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit silverSaltConsumptionConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setSilverSaltConsumptionConst"));
}

void ParameterManager::setTotalDevelopmentTime(float totalDevelopmentTime)
{
    QMutexLocker paramLocker(&paramMutex);
    m_totalDevelopmentTime = totalDevelopmentTime;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit totalDevelopmentTimeChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setTotalDevelopmentTime"));
}

void ParameterManager::setAgitateCount(int agitateCount)
{
    QMutexLocker paramLocker(&paramMutex);
    m_agitateCount = agitateCount;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit agitateCountChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setAgitateCount"));
}

void ParameterManager::setDevelopmentSteps(int developmentSteps)
{
    QMutexLocker paramLocker(&paramMutex);
    m_developmentSteps = developmentSteps;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit developmentStepsChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setDevelopmentSteps"));
}

void ParameterManager::setFilmArea(float filmArea)
{
    QMutexLocker paramLocker(&paramMutex);
    m_filmArea = filmArea;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit filmAreaChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setFilmArea"));
}

void ParameterManager::setSigmaConst(float sigmaConst)
{
    QMutexLocker paramLocker(&paramMutex);
    m_sigmaConst = sigmaConst;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit sigmaConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setSigmaConst"));
}

void ParameterManager::setLayerMixConst(float layerMixConst)
{
    QMutexLocker paramLocker(&paramMutex);
    m_layerMixConst = layerMixConst;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit layerMixConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setLayerMixConst"));
}

void ParameterManager::setLayerTimeDivisor(float layerTimeDivisor)
{
    QMutexLocker paramLocker(&paramMutex);
    m_layerTimeDivisor = layerTimeDivisor;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit layerTimeDivisorChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setLayerTimeDivisor"));
}

void ParameterManager::setRolloffBoundary(float rolloffBoundary)
{
    QMutexLocker paramLocker(&paramMutex);
    m_rolloffBoundary = rolloffBoundary;
    validity = min(validity, Valid::prefilmulation);
    paramLocker.unlock();
    emit rolloffBoundaryChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setRolloffBoundary"));
}

std::tuple<Valid,AbortStatus,BlackWhiteParams> ParameterManager::claimBlackWhiteParams()
{
    QMutexLocker paramLocker(&paramMutex);
    AbortStatus abort;
    if (validity < Valid::filmulation)
    {
        abort = AbortStatus::restart;
    }
    else
    {
        abort = AbortStatus::proceed;
        validity = Valid::blackwhite;//mark it as started
    }
    BlackWhiteParams params;
    params.blackpoint  = m_blackpoint;
    params.whitepoint  = m_whitepoint;
    params.cropHeight  = m_cropHeight;
    params.cropAspect  = m_cropAspect;
    params.cropVoffset = m_cropVoffset;
    params.cropHoffset = m_cropHoffset;
    std::tuple<Valid,AbortStatus,BlackWhiteParams> tup(validity, abort, params);
    return tup;
}

void ParameterManager::setBlackpoint(float blackpoint)
{
    QMutexLocker paramLocker(&paramMutex);
    m_blackpoint = blackpoint;
    validity = min(validity, Valid::filmulation);
    paramLocker.unlock();
    emit blackpointChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setBlackpoint"));
}

void ParameterManager::setWhitepoint(float whitepoint)
{
    QMutexLocker paramLocker(&paramMutex);
    m_whitepoint = whitepoint;
    validity = min(validity, Valid::filmulation);
    paramLocker.unlock();
    emit whitepointChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setWhitepoint"));
}

void ParameterManager::setCropHeight(float cropHeight)
{
    QMutexLocker paramLocker(&paramMutex);
    m_cropHeight = cropHeight;
    validity = min(validity, Valid::filmulation);
    paramLocker.unlock();
    emit cropHeightChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setCropHeight"));
}

void ParameterManager::setCropAspect(float cropAspect)
{
    QMutexLocker paramLocker(&paramMutex);
    m_cropAspect = cropAspect;
    validity = min(validity, Valid::filmulation);
    paramLocker.unlock();
    emit cropHeightChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setCropAspect"));
}

void ParameterManager::setCropVoffset(float cropVoffset)
{
    QMutexLocker paramLocker(&paramMutex);
    m_cropVoffset = cropVoffset;
    validity = min(validity, Valid::filmulation);
    paramLocker.unlock();
    emit cropHeightChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setCropVoffset"));
}

void ParameterManager::setCropHoffset(float cropHoffset)
{
    QMutexLocker paramLocker(&paramMutex);
    m_cropHoffset = cropHoffset;
    validity = min(validity, Valid::filmulation);
    paramLocker.unlock();
    emit cropHeightChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setCropHoffset"));
}

//We don't have any color curves, so this one short-circuits those
// and checks back to blackwhite validity.
//If we add color curves in that place, we do need to replace the following
// uses of 'Valid::blackwhite' with 'Valid::colorcurve'
std::tuple<Valid,AbortStatus,FilmlikeCurvesParams> ParameterManager::claimFilmlikeCurvesParams()
{
    QMutexLocker paramLocker(&paramMutex);
    AbortStatus abort;
    if (validity < Valid::blackwhite)
    {
        abort = AbortStatus::restart;
    }
    else
    {
        abort = AbortStatus::proceed;
        validity = Valid::filmlikecurve;//mark it as started
    }
    FilmlikeCurvesParams params;
    params.shadowsX = m_shadowsX;
    params.shadowsY = m_shadowsY;
    params.highlightsX = m_highlightsX;
    params.highlightsY = m_highlightsY;
    params.vibrance = m_vibrance;
    params.saturation = m_saturation;
    std::tuple<Valid,AbortStatus,FilmlikeCurvesParams> tup(validity, abort, params);
    return tup;
}

void ParameterManager::setShadowsX(float shadowsX)
{
    QMutexLocker paramLocker(&paramMutex);
    m_shadowsX = shadowsX;
    validity = min(validity, Valid::blackwhite);
    paramLocker.unlock();
    emit shadowsXChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setShadowsX"));
}

void ParameterManager::setShadowsY(float shadowsY)
{
    QMutexLocker paramLocker(&paramMutex);
    m_shadowsY = shadowsY;
    validity = min(validity, Valid::blackwhite);
    paramLocker.unlock();
    emit shadowsYChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setShadowsY"));
}

void ParameterManager::setHighlightsX(float highlightsX)
{
    QMutexLocker paramLocker(&paramMutex);
    m_highlightsX = highlightsX;
    validity = min(validity, Valid::blackwhite);
    paramLocker.unlock();
    emit highlightsXChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setHighlightsX"));
}

void ParameterManager::setHighlightsY(float highlightsY)
{
    QMutexLocker paramLocker(&paramMutex);
    m_highlightsY = highlightsY;
    validity = min(validity, Valid::blackwhite);
    paramLocker.unlock();
    emit highlightsYChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setHighlightsY"));
}

void ParameterManager::setVibrance(float vibrance)
{
    QMutexLocker paramLocker(&paramMutex);
    m_vibrance = vibrance;
    validity = min(validity, Valid::blackwhite);
    paramLocker.unlock();
    emit vibranceChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setVibrance"));
}

void ParameterManager::setSaturation(float saturation)
{
    QMutexLocker paramLocker(&paramMutex);
    m_saturation = saturation;
    validity = min(validity, Valid::blackwhite);
    paramLocker.unlock();
    emit saturationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setSaturation"));
}

std::tuple<Valid,AbortStatus,OrientationParams> ParameterManager::claimOrientationParams()
{
    QMutexLocker paramLocker(&paramMutex);
    AbortStatus abort;
    if (validity < Valid::filmlikecurve)
    {
        abort = AbortStatus::restart;
    }
    else
    {
        abort = AbortStatus::proceed;
        validity = Valid::count;
    }
    OrientationParams params;
    params.rotation = m_rotation;
    std::tuple<Valid,AbortStatus,OrientationParams> tup(validity, abort, params);
    return tup;
}

void ParameterManager::setRotation(int rotation)
{
    QMutexLocker paramLocker(&paramMutex);
    m_rotation = rotation;
    validity = min(validity, Valid::filmlikecurve);
    paramLocker.unlock();
    emit rotationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setRotation"));
}

void ParameterManager::rotateRight()
{
    QMutexLocker paramLocker(&paramMutex);
    int rotation = m_rotation - 1;
    if (rotation < 0)
    {
        rotation += 4;
    }
    m_rotation = rotation;
    validity = min(validity, Valid::filmlikecurve);
    paramLocker.unlock();
    emit rotationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("rotateRight"));
    writeback();//Normally the slider has to call this when released, but this isn't a slider.
}

void ParameterManager::rotateLeft()
{
    QMutexLocker paramLocker(&paramMutex);
    int rotation = m_rotation + 1;
    if (rotation > 3)
    {
        rotation -= 4;
    }
    m_rotation = rotation;
    validity = min(validity, Valid::filmlikecurve);
    paramLocker.unlock();
    emit rotationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("rotateLeft"));
    writeback();//Normally the slider has to call this when released, but this isn't a slider.
}

Valid ParameterManager::getValid()
{
    QMutexLocker paramLocker(&paramMutex);
    return validity;
}

//This gets called by a slider (from qml) when it is released.
//It syncs the main (slider-interfaced) settings with the database.
void ParameterManager::writeback()
{
    //Writeback gets called by setters, which themselves are called by
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
    //Write back the slider to the database.
    QSqlQuery query;
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
                  "ProcTrotation) "                       //31
                  " values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
                  //        0 1 2 3 4 5 6 7 8 910 1 2 3 4 5 6 7 8 920 1 2 3 4 5 6 7 8 930 1
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
    query.exec();
    //Write that it's been edited to the SearchTable (actually writing the edit time)
    QDateTime now = QDateTime::currentDateTime();
    query.prepare("UPDATE SearchTable SET STlastProcessedTime = ?, "
                  "STthumbWritten = 0, "
                  "STbigThumbWritten = 0 "
                  "WHERE STsearchID = ?;");
    query.bindValue(0, QVariant(now.toTime_t()));
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

//Returns greatest common denominator. From wikipedia.
unsigned int gcd(unsigned int u, unsigned int v)
{
    // simple cases (termination)
    if (u == v) { return u; }

    if (u == 0) { return v; }

    if (v == 0) { return u; }

    // look for factors of 2
    if (~u & 1) // u is even
    {
        if (v & 1) // v is odd
        {
            return gcd(u >> 1, v);
        }
        else // both u and v are even
        {
            return gcd(u >> 1, v >> 1) << 1;
        }
    }

    if (~v & 1) // u is odd, v is even
    {
        return gcd(u, v >> 1);
    }

    // reduce larger argument
    if (u > v)
    {
        return gcd((u - v) >> 1, v);
    }

    return gcd((v - u) >> 1, u);
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
        emit imageIndexChanged();
        validity = Valid::none;
    }

    QString tempString = imageID;
    tempString.truncate(32);//length of md5
    QSqlQuery query;
    query.prepare("SELECT \
                  FTfilePath,FTsensitivity,FTexposureTime,FTaperture,FTfocalLength \
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
    float numAperture = query.value(nameCol).toFloat();
    if (numAperture >= 8)
    {
        aperture = QString::number(numAperture,'f',0);
    }
    else //numAperture < 8
    {
        aperture = QString::number(numAperture,'f',1);
    }
    emit apertureChanged();

    nameCol = rec.indexOf("FTfocalLength");
    if (-1 == nameCol) { std::cout << "paramManager FTfocalLength" << endl; }
    focalLength = query.value(nameCol).toFloat();
    emit focalLengthChanged();

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

    paramLocker.unlock();

    //Emit that the things have changed.
    emit caEnabledChanged();
    emit highlightsChanged();
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
    emit shadowsXChanged();
    emit shadowsYChanged();
    emit highlightsXChanged();
    emit highlightsYChanged();
    emit vibranceChanged();
    emit saturationChanged();
    emit rotationChanged();

    emit defCaEnabledChanged();
    emit defHighlightsChanged();
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
    emit defRotationChanged();


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
    QSqlQuery query;

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

    //First is caEnabled.
    nameCol = rec.indexOf("ProfTcaEnabled");
    if (-1 == nameCol) { std::cout << "paramManager ProfTcaEnabled" << endl; }
    const bool temp_caEnabled = query.value(nameCol).toBool();
    d_caEnabled = temp_caEnabled;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_caEnabled = temp_caEnabled;
    }

    //Next is highlights (highlight recovery)
    nameCol = rec.indexOf("ProfThighlightRecovery");
    if (-1 == nameCol) { std::cout << "paramManager ProfThighlightRecovery" << endl; }
    const int temp_highlights = query.value(nameCol).toInt();
    d_highlights = temp_highlights;
    if (copyDefaults == CopyDefaults::loadToParams)
    {
        m_highlights = temp_highlights;
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
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(absFilePath);
        image->readMetadata();
        Exiv2::ExifData exifData = image->exifData();
        d_rotation = exifDefaultRotation(exifData);
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
    QSqlRecord rec;
    int nameCol;
    QSqlQuery query;

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

    //First is caEnabled.
    nameCol = rec.indexOf("ProcTcaEnabled");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcaEnabled" << endl; }
    const bool temp_caEnabled = query.value(nameCol).toBool();
    if (temp_caEnabled != m_caEnabled)
    {
        //cout << "ParameterManager::loadParams caEnabled" << endl;
        m_caEnabled = temp_caEnabled;
        validity = min(validity, Valid::none);
    }

    //Next is highlights (highlight recovery)
    nameCol = rec.indexOf("ProcThighlightRecovery");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightRecovery" << endl; }
    const int temp_highlights = query.value(nameCol).toInt();
    if (temp_highlights != m_highlights)
    {
        //cout << "ParameterManager::loadParams highlights" << endl;
        m_highlights = temp_highlights;
        validity = min(validity, Valid::none);
    }

    //Exposure compensation
    nameCol = rec.indexOf("ProcTexposureComp");
    if (-1 == nameCol) { std::cout << "paramManager ProcTexposureComp" << endl; }
    const float temp_exposureComp = query.value(nameCol).toFloat();
    if (temp_exposureComp != m_exposureComp)
    {
        //cout << "ParameterManager::loadParams exposureComp" << endl;
        m_exposureComp = temp_exposureComp;
        validity = min(validity, Valid::load);
    }

    //Temperature
    nameCol = rec.indexOf("ProcTtemperature");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtemperature" << endl; }
    const float temp_temperature = query.value(nameCol).toFloat();
    if (temp_temperature != m_temperature)
    {
        //cout << "ParameterManager::loadParams temperature" << endl;
        m_temperature = temp_temperature;
        validity = min(validity, Valid::load);
    }

    //Tint
    nameCol = rec.indexOf("ProcTtint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtint" << endl; }
    const float temp_tint = query.value(nameCol).toFloat();
    if (temp_tint != m_tint)
    {
        //cout << "ParameterManager::loadParams tint" << endl;
        m_tint = temp_tint;
        validity = min(validity, Valid::load);
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

    //Rotation
    nameCol = rec.indexOf("ProcTrotation");
    if (-1 == nameCol) { std::cout << "paramManager ProcTrotation" << endl; }
    const int temp_rotation = query.value(nameCol).toInt();
    if (temp_rotation != m_rotation)
    {
        //cout << "ParameterManager::loadParams rotation" << endl;
        m_rotation = temp_rotation;
        validity = min(validity, Valid::filmlikecurve);
    }
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
        emit updateImage();
    }
    copyFromImageIndex = "";
    pasteable = false;
    emit pasteableChanged();
}

//This is for copying and pasting.
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

