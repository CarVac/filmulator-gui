#include "parameterManager.h"

using std::min;
using std::cout;
using std::endl;

ParameterManager::ParameterManager() : QObject(0)
{
    paramChangeEnabled = true;
    m_tiffIn = false;
    m_jpegIn = false;
    m_caEnabled = false;
    m_highlights = 0;
    m_exposureComp = 0.0f;
    m_temperature = 5200.0f;
    m_tint = 1.0f;
    m_initialDeveloperConcentration = 1.0f;
    m_reservoirThickness = 1000.0f;
    m_activeLayerThickness = 0.1f;
    m_crystalsPerPixel = 500.0f;
    m_initialCrystalRadius = 0.00001f;
    m_initialSilverSaltDensity = 1.0f;
    m_initialSilverSaltDensity = 2000000.0f;
    m_crystalGrowthConst =  0.00001f;
    m_silverSaltConsumptionConst = 2000000.0f;
    m_totalDevelopmentTime = 100.0f;
    m_agitateCount = 1;
    m_developmentSteps = 12;
    m_filmArea = 864.0f;
    m_sigmaConst = 0.2f;
    m_layerMixConst = 0.2f;
    m_layerTimeDivisor = 20.0f;
    m_rolloffBoundary = 51275;
    m_blackpoint = 0.0f;
    m_whitepoint = 0.002f;
    m_shadowsX = 0.25f;
    m_shadowsY = 0.25f;
    m_highlightsX = 0.75f;
    m_highlightsY = 0.75f;
    m_vibrance = 0.0f;
    m_saturation = 0.0f;
    m_rotation = 0;

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
    params.blackpoint = m_blackpoint;
    params.whitepoint = m_whitepoint;
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
void ParameterManager::writeToDB(QString imageID)
{
    //Write back the slider to the database.
    QSqlQuery query;
    query.exec("BEGIN;");//Stick these all into one db action for speed.
    query.prepare("UPDATE ProcessingTable SET "
                  "ProcTinitialDeveloperConcentration=?,"   //00
                  "ProcTreservoirThickness=?,"              //01
                  "ProcTactiveLayerThickness=?,"            //02
                  "ProcTcrystalsPerPixel=?,"                //03
                  "ProcTinitialCrystalRadius=?,"            //04
                  "ProcTinitialSilverSaltDensity=?,"        //05
                  "ProcTdeveloperConsumptionConst=?,"       //06
                  "ProcTcrystalGrowthConst=?,"              //07
                  "ProcTsilverSaltConsumptionConst=?,"      //08
                  "ProcTtotalDevelopmentTime=?,"            //09
                  "ProcTagitateCount=?,"                    //10
                  "ProcTdevelopmentSteps=?,"                //11
                  "ProcTfilmArea=?,"                        //12
                  "ProcTsigmaConst=?,"                      //13
                  "ProcTlayerMixConst=?,"                   //14
                  "ProcTlayerTimeDivisor=?,"                //15
                  "ProcTrolloffBoundary=?,"                 //16
                  "ProcTexposureComp=?,"                    //17
                  "ProcTwhitepoint=?,"                      //18
                  "ProcTblackpoint=?,"                      //19
                  "ProcTshadowsX=?,"                        //20
                  "ProcTshadowsY=?,"                        //21
                  "ProcThighlightsX=?,"                     //22
                  "ProcThighlightsY=?,"                     //23
                  "ProcThighlightRecovery=?,"               //24
                  "ProcTcaEnabled=?,"                       //25
                  "ProcTtemperature=?,"                     //26
                  "ProcTtint=?,"                            //27
                  "ProcTvibrance=?,"                        //28
                  "ProcTsaturation=?,"                      //29
                  "ProcTrotation=? "                        //30
                  "WHERE ProcTprocID = ?;");                //31
    query.bindValue( 0, m_initialDeveloperConcentration);
    query.bindValue( 1, m_reservoirThickness);
    query.bindValue( 2, m_activeLayerThickness);
    query.bindValue( 3, m_crystalsPerPixel);
    query.bindValue( 4, m_initialCrystalRadius);
    query.bindValue( 5, m_initialSilverSaltDensity);
    query.bindValue( 6, m_developerConsumptionConst);
    query.bindValue( 7, m_crystalGrowthConst);
    query.bindValue( 8, m_silverSaltConsumptionConst);
    query.bindValue( 9, m_totalDevelopmentTime);
    query.bindValue(10, m_agitateCount);
    query.bindValue(11, m_developmentSteps);
    query.bindValue(12, m_filmArea);
    query.bindValue(13, m_sigmaConst);
    query.bindValue(14, m_layerMixConst);
    query.bindValue(15, m_layerTimeDivisor);
    query.bindValue(16, m_rolloffBoundary);
    query.bindValue(17, m_exposureComp);
    query.bindValue(18, m_whitepoint);
    query.bindValue(19, m_blackpoint);
    query.bindValue(20, m_shadowsX);
    query.bindValue(21, m_shadowsY);
    query.bindValue(22, m_highlightsX);
    query.bindValue(23, m_highlightsY);
    query.bindValue(24, m_highlights);
    query.bindValue(25, m_caEnabled);
    query.bindValue(26, m_temperature);
    query.bindValue(27, m_tint);
    query.bindValue(28, m_vibrance);
    query.bindValue(29, m_saturation);
    query.bindValue(30, m_rotation);
    query.bindValue(31, imageID);
    query.exec();
    //Write that it's been edited to the SearchTable (actually writing the edit time)
    QDateTime now = QDateTime::currentDateTime();
    query.prepare("UPDATE SearchTable SET STlastProcessedTime = ? WHERE STsearchID = ?;");
    query.bindValue(0, QVariant(now.toTime_t()));
    query.bindValue(1, imageID);
    query.exec();
    //Write that it's been edited to the QueueTable
    query.prepare("UPDATE QueueTable SET QTprocessed = ? WHERE QTsearchID = ?;");
    query.bindValue(0, QVariant(true));
    query.bindValue(1, imageID);
    query.exec();
    query.exec("COMMIT;");//Apply all the changes together.
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
void ParameterManager::selectImage(QString imageID)
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
    QString name = query.value(0).toString();
    m_fullFilename = name.toStdString();
    filename = name.right(name.size() - name.lastIndexOf(QString("/")) - 1);
    emit filenameChanged();

    nameCol = rec.indexOf("FTsensitivity");
    if (-1 == nameCol) { std::cout << "paramManager FTsensitivity" << endl; }
    sensitivity = query.value(1).toInt();
    emit sensitivityChanged();

    nameCol = rec.indexOf("FTexposureTime");
    if (-1 == nameCol) { std::cout << "paramManager FTexposureTime" << endl; }
    QString expTimeTemp = query.value(2).toString();
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
    aperture = query.value(3).toFloat();
    emit apertureChanged();

    nameCol = rec.indexOf("FTfocalLength");
    if (-1 == nameCol) { std::cout << "paramManager FTfocalLength" << endl; }
    focalLength = query.value(4).toFloat();
    emit focalLengthChanged();

    //Copy all of the processing parameters from the db into this param manager.
    loadParams(imageID);

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


    //Mark that it's safe for sliders to move again.
    QMutexLocker signalLocker(&signalMutex);
    enableParamChange();//Re-enable updating of the image.
    paramChangeWrapper(QString("selectImage"));

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

