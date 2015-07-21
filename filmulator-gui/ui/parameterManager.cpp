#include "parameterManager.h"

using std::cout;
using std::endl;

ParameterManager::ParameterManager() : QObject(0)
{
    paramChangeEnabled = true;
    std::vector<std::string> inputFilenameList;
    inputFilenameList.push_back("");
    m_filenameList = inputFilenameList;
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

std::tuple<AbortStatus,LoadParams> ParameterManager::claimLoadParams()
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
    LoadParams params = LoadParams(m_fullFilename,
                                   m_tiffIn,
                                   m_jpegIn);
    std::tuple<AbortStatus,LoadParams> tup (abort, params);
    return tup;
}

void ParameterManager::setTiffIn(bool tiffIn)
{
    QMutexLocker paramLocker(&paramMutex);
    m_tiffIn = tiffIn;
    validity = min(validity, Valid::none);
    emit tiffInChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setTiff"));
}

void ParameterManager::setJpegIn(bool jpegIn)
{
    QMutexLocker paramLocker(&paramMutex);
    m_jpegIn = jpegIn;
    validity = min(validity, Valid::none);
    emit jpegInChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setJpeg"));
}

std::tuple<AbortStatus,DemosaicParams> ParameterManager::claimDemosaicParams()
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
    DemosaicParams params = DemosaicParams(m_caEnabled,
                                           m_highlights);
    std::tuple<AbortStatus,DemosaicParams> tup(abort, params);
    return tup;
}

void ParameterManager::setCaEnabled(bool caEnabled)
{
    QMutexLocker paramLocker(&paramMutex);
    m_caEnabled = caEnabled;
    validity = min(validity, Valid::load);
    emit caEnabledChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setCaEnabled"));
}

void ParameterManager::setHighlights(int highlights)
{
    QMutexLocker paramLocker(&paramMutex);
    m_highlights = highlights;
    validity = min(validity, Valid::load);
    emit highlightsChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setHighlights"));
}

std::tuple<AbortStatus,PrefilmParams> ParameterManager::claimPrefilmParams()
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
    PrefilmParams params = PrefilmParams(m_exposurecomp,
                                         m_temperature,
                                         m_tint);
    std::tuple<AbortStatus,PrefilmParams> tup(abort, params);
    return tup;
}

void ParameterManager::setExposureComp(float exposureComp)
{
    QMutexLocker paramLocker(&paramMutex);
    m_exposureComp = exposureComp;
    validity = min(validity, Valid::demosaic);
    emit exposureCompChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setExposureComp"));
}

void ParameterManager::setTemperature(float temperature)
{
    QMutexLocker paramLocker(&paramMutex);
    m_temperature = temperature;
    validity = min(validity, Valid::demosaic);
    emit temperatureChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setTemperature"));
}

void ParameterManager::setTint(float tint)
{
    QMutexLocker paramLocker(&paramMutex);
    m_tint = tint;
    validity = min(validity, Valid::demosaic);
    emit tintChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setTint"));
}

std::tuple<AbortStatus,FilmParams> ParameterManager::claimFilmParams()
{
    QMutexLocker paramLocker(&paramMutex);
    AbortStatus abort;
    if (validity < Valid::prefilmulation)
    {
        abort = AbortStatus::restart;
    }
    else
    {
        abort = AbortStatus::proceed;
        validity = Valid::filmulation;//mark it as started
    }
    FilmParams params = FilmParams(m_initialDeveloperConcentration,
                                   m_reservoirThickness,
                                   m_activeLayerThickness,
                                   m_crystalsPerPixel,
                                   m_initialCrystalRadius,
                                   m_initialSilverSaltDensity,
                                   m_developerConsumptionConst,
                                   m_crystalGrowthConst,
                                   m_silverSaltConsumptionConst,
                                   m_totalDevelopmentTime,
                                   m_agitateCount,
                                   m_developmentSteps,
                                   m_filmArea,
                                   m_sigmaConst,
                                   m_layerMixConst,
                                   m_layerTimeDivisor,
                                   m_rolloffBoundary);
    std::tuple<AbortStatus,FilmParams> tup(abort, params);
    return tup;
}

void ParameterManager::setInitialDeveloperConcentration(float initialDeveloperConcentration)
{
    QMutexLocker paramLocker(&paramMutex);
    m_initialDeveloperConcentration = initialDeveloperConcentration;
    validity = min(validity, Valid::prefilmulation);
    emit initialDeveloperConcentrationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setInitialDeveloperConcentration"));
}

void ParameterManager::setReservoirThickness(float reservoirThickness)
{
    QMutexLocker paramLocker(&paramMutex);
    m_reservoirThickness = reservoirThickness;
    validity = min(validity, Valid::prefilmulation);
    emit reservoirThicknessChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setReservoirThickness"));
}

void ParameterManager::setActiveLayerThickness(float activeLayerThickness)
{
    QMutexLocker paramLocker(&paramMutex);
    m_activeLayerThickness = activeLayerThickness;
    validity = min(validity, Valid::prefilmulation);
    emit activeLayerThicknessChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setActiveLayerThickness"));
}

void ParameterManager::setCrystalsPerPixel(float crystalsPerPixel)
{
    QMutexLocker paramLocker(&paramMutex);
    m_crystalsPerPixel = crystalsPerPixel;
    validity = min(validity, Valid::prefilmulation);
    emit crystalsPerPixelChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setCrystalsPerPixel"));
}

void ParameterManager::setInitialCrystalRadius(float initialCrystalRadius)
{
    QMutexLocker paramLocker(&paramMutex);
    m_initialCrystalRadius = initialCrystalRadius;
    validity = min(validity, Valid::prefilmulation);
    emit initialCrystalRadiusChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setInitialCrystalRadius"));
}

void ParameterManager::setInitialSilverSaltDensity(float initialSilverSaltDensity)
{
    QMutexLocker paramLocker(&paramMutex);
    m_initialSilverSaltDensity = initialSilverSaltDensity;
    validity = min(validity, Valid::prefilmulation);
    emit initialSilverSaltDensityChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setInitialSilverSaltDensity"));
}

void ParameterManager::setDeveloperConsumptionConst(float developerConsumptionConst)
{
    QMutexLocker paramLocker(&paramMutex);
    m_developerConsumptionConst = developerConsumptionConst;
    validity = min(validity, Valid::prefilmulation);
    emit developerConsumptionConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setDeveloperConsumptionConst"));
}

void ParameterManager::setCrystalGrowthConst(float crystalGrowthConst)
{
    QMutexLocker paramLocker(&paramMutex);
    m_crystalGrowthConst = crystalGrowthConst;
    validity = min(validity, Valid::prefilmulation);
    emit crystalGrowthConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setCrystalGrowthConst"));
}

void ParameterManager::setSilverSaltConsumptionConst(float silverSaltConsumptionConst)
{
    QMutexLocker paramLocker(&paramMutex);
    m_silverSaltConsumptionConst = silverSaltConsumptionConst;
    validity = min(validity, Valid::prefilmulation);
    emit silverSaltConsumptionConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setSilverSaltConsumptionConst"));
}

void ParameterManager::setTotalDevelopmentTime(float totalDevelopmentTime)
{
    QMutexLocker paramLocker(&paramMutex);
    m_totalDevelopmentTime = totalDevelopmentTime;
    validity = min(validity, Valid::prefilmulation);
    emit totalDevelopmentTimeChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setTotalDevelopmentTime"));
}

void ParameterManager::setAgitateCount(int agitateCount)
{
    QMutexLocker paramLocker(&paramMutex);
    m_agitateCount = agitateCount;
    validity = min(validity, Valid::prefilmulation);
    emit agitateCountChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setAgitateCount"));
}

void ParameterManager::setDevelopmentSteps(int developmentSteps)
{
    QMutexLocker paramLocker(&paramMutex);
    m_developmentSteps = developmentSteps;
    validity = min(validity, Valid::prefilmulation);
    emit developmentStepsChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setDevelopmentSteps"));
}

void ParameterManager::setFilmArea(float filmArea)
{
    QMutexLocker paramLocker(&paramMutex);
    m_filmArea = filmArea;
    validity = min(validity, Valid::prefilmulation);
    emit filmAreaChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setFilmArea"));
}

void ParameterManager::setSigmaConst(float sigmaConst)
{
    QMutexLocker paramLocker(&paramMutex);
    m_sigmaConst = sigmaConst;
    validity = min(validity, Valid::prefilmulation);
    emit sigmaConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setSigmaConst"));
}

void ParameterManager::setLayerMixConst(float layerMixConst)
{
    QMutexLocker paramLocker(&paramMutex);
    m_layerMixConst = layerMixConst;
    validity = min(validity, Valid::prefilmulation);
    emit layerMixConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setLayerMixConst"));
}

void ParameterManager::setLayerTimeDivisor(float layerTimeDivisor)
{
    QMutexLocker paramLocker(&paramMutex);
    m_layerTimeDivisor = layerTimeDivisor;
    validity = min(validity, Valid::prefilmulation);
    emit layerTimeDivisorChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setLayerTimeDivisor"));
}

void ParameterManager::setRolloffBoundary(float rolloffBoundary)
{
    QMutexLocker paramLocker(&paramMutex);
    m_rolloffBoundary = rolloffBoundary;
    validity = min(validity, Valid::prefilmulation);
    emit rolloffBoundaryChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setRolloffBoundary"));
}

std::tuple<AbortStatus,BlackWhiteParams> ParameterManager::claimBlackWhiteParams()
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
    BlackWhiteParams params = BlackWhiteParams(m_blackpoint,
                                               m_whitepoint);
    std::tuple<AbortStatus,BlackWhiteParams> tup(abort, params);
    return tup;
}

void ParameterManager::setBlackpoint(float blackpoint)
{
    QMutexLocker paramLocker(&paramMutex);
    m_blackpoint = blackpoint;
    validity = min(validity, Valid::filmulation);
    emit blackpointChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setBlackpoint"));
}

void ParameterManager::setWhitepoint(float whitepoint)
{
    QMutexLocker paramLocker(&paramMutex);
    m_whitepoint = whitepoint;
    validity = min(validity, Valid::filmulation);
    emit whitepointChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setWhitepoint"));
}

std::tuple<AbortStatus,CurvesParams> ParameterManager::claimCurvesParams()
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
    CurvesParams params = CurvesParams(m_shadowsX,
                                       m_shadowsY,
                                       m_highlightsX,
                                       m_highlightsY,
                                       m_vibrance,
                                       m_saturation);
    std::tuple<AbortStatus,CurvesParams> tup(abort, params);
    return tup;
}

void ParameterManager::setShadowsX(float shadowsX)
{
    QMutexLocker paramLocker(&paramMutex);
    m_shadowsX = shadowsX;
    validity = min(validity, Valid::blackwhite);
    emit shadowsXChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setShadowsX"));
}

void ParameterManager::setShadowsY(float shadowsY)
{
    QMutexLocker paramLocker(&paramMutex);
    m_shadowsY = shadowsY;
    validity = min(validity, Valid::blackwhite);
    emit shadowsYChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setShadowsY"));
}

void ParameterManager::setHighlightsX(float highlightsX)
{
    QMutexLocker paramLocker(&paramMutex);
    m_highlightsX = highlightsX;
    validity = min(validity, Valid::blackwhite);
    emit highlightsXChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setHighlightsX"));
}

void ParameterManager::setHighlightsY(float highlightsY)
{
    QMutexLocker paramLocker(&paramMutex);
    m_highlightsY = highlightsY;
    validity = min(validity, Valid::blackwhite);
    emit highlightsYChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setHighlightsY"));
}

void ParameterManager::setVibrance(float vibrance)
{
    QMutexLocker paramLocker(&paramMutex);
    m_vibrance = vibrance;
    validity = min(validity, Valid::blackwhite);
    emit vibranceChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setVibrance"));
}

void ParameterManager::setSaturation(float saturation)
{
    QMutexLocker paramLocker(&paramMutex);
    m_saturation = saturation;
    validity = min(validity, Valid::blackwhite);
    emit saturationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setSaturation"));
}

std::tuple<AbortStatus,OrientationParams> ParameterManager::claimOrientationParams()
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
    OrientationParams params = OrientationsParams(m_rotation);
    std::tuple<AbortStatus,CurvesParams> tup(abort, params);
    return tup;
}

void ParameterManager::setRotation(int rotation)
{
    QMutexLocker paramLocker(&paramMutex);
    m_rotation = rotation;
    validity = min(validity, Valid::filmlikecurve);
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
    query.bindValue(17, m_exposureComp[0]);
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
    emit updateTable("ProcessingTable", 0);//0 means edit
    emit updateTable("SearchTable", 0);//0 means edit
    emit updateTable("QueueTable", 0);//0 means edit
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

void ParameterManager::selectImage(QString imageID)
{
    QMutexLocker paramLocker(&paramMutex);//Make all the param changes happen together.
    disableParamChange();//Prevent aborting of computation.

    imageIndex = imageID;
    emit imageIndexChanged();

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
    paramLocker.unlock();
    QMutexLocker signalLocker(&signalMutex);
    enableParamChange();//Re-enable updating of the image.
    paramChangeWrapper(QString("selectImage"));

}

//This loads all of the processing params from the imageID into the param manager.
//TODO: for partial copying, make a loadParams that lets you select which ones
// you want.
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
    m_caEnabled = query.value(nameCol).toBool();

    //Next is highlights (highlight recovery)
    nameCol = rec.indexOf("ProcThighlightRecovery");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightRecovery" << endl; }
    m_highlights = query.value(nameCol).toInt();

    //Exposure compensation
    nameCol = rec.indexOf("ProcTexposureComp");
    if (-1 == nameCol) { std::cout << "paramManager ProcTexposureComp" << endl; }
    m_exposureComp = query.value(nameCol).toFloat();

    //Temperature
    nameCol = rec.indexOf("ProcTtemperature");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtemperature" << endl; }
    m_temperature = query.value(nameCol).toFloat();

    //Tint
    nameCol = rec.indexOf("ProcTtint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtint" << endl; }
    m_tint = query.value(nameCol).toFloat();

    //Initial developer concentration
    nameCol = rec.indexOf("ProcTinitialDeveloperConcentration");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialDeveloperConcentration" << endl; }
    m_initialDeveloperConcentration = query.value(nameCol).toFloat();

    //Reservoir thickness
    nameCol = rec.indexOf("ProcTreservoirThickness");
    if (-1 == nameCol) { std::cout << "paramManager ProcTreservoirThickness" << endl; }
    m_reservoirThickness = query.value(nameCol).toFloat();

    //Active layer thickness
    nameCol = rec.indexOf("ProcTactiveLayerThickness");
    if (-1 == nameCol) { std::cout << "paramManager ProcTactiveLayerThickness" << endl; }
    m_activeLayerThickness = query.value(nameCol).toFloat();

    //Crystals per pixel
    nameCol = rec.indexOf("ProcTcrystalsPerPixel");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcrystalsPerPixel" << endl; }
    m_crystalsPerPixel = query.value(nameCol).toFloat();

    //Initial crystal radius
    nameCol = rec.indexOf("ProcTinitialCrystalRadius");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialCrystalRadius" << endl; }
    m_initialCrystalRadius = query.value(nameCol).toFloat();

    //Initial silver salt area density
    nameCol = rec.indexOf("ProcTinitialSilverSaltDensity");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialSilverSaltDensity" << endl; }
    m_initialSilverSaltDensity = query.value(nameCol).toFloat();

    //Developer consumption rate constant
    nameCol = rec.indexOf("ProcTdeveloperConsumptionConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTdeveloperConsumptionConst" << endl; }
    m_developerConsumptionConst = query.value(nameCol).toFloat();

    //Crystal growth rate constant
    nameCol = rec.indexOf("ProcTcrystalGrowthConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcrystalGrowthConst" << endl; }
    m_crystalGrowthConst = query.value(nameCol).toFloat();

    //Silver halide consumption rate constant
    nameCol = rec.indexOf("ProcTsilverSaltConsumptionConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsilverSaltConsumptionConst" << endl; }
    m_silverSaltConsumptionConst = query.value(nameCol).toFloat();

    //Total development time
    nameCol = rec.indexOf("ProcTtotalDevelopmentTime");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtotalDevelopmentTime" << endl; }
    m_totalDevelopmentTime = query.value(nameCol).toFloat();

    //Number of agitations
    nameCol = rec.indexOf("ProcTagitateCount");
    if (-1 == nameCol) { std::cout << "paramManager ProcTagitateCount" << endl; }
    m_agitateCount = query.value(nameCol).toInt();

    //Number of simulation steps for development
    nameCol = rec.indexOf("ProcTdevelopmentSteps");
    if (-1 == nameCol) { std::cout << "paramManager ProcTdevelopmentSteps" << endl; }
    m_developmentSteps = query.value(nameCol).toInt();

    //Area of film for the simulation
    nameCol = rec.indexOf("ProcTfilmArea");
    if (-1 == nameCol) { std::cout << "paramManager ProcTfilmArea" << endl; }
    m_filmArea = query.value(nameCol).toFloat();

    //A constant for the size of the diffusion. It...affects the same thing as film area.
    nameCol = rec.indexOf("ProcTsigmaConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsigmaConst" << endl; }
    m_sigmaConst = query.value(nameCol).toFloat();

    //Layer mix constant: the amount of active developer that gets exchanged with the reservoir.
    nameCol = rec.indexOf("ProcTlayerMixConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTlayerMixConst" << endl; }
    m_layerMixConst = query.value(nameCol).toFloat();

    //Layer time divisor: Controls the relative intra-layer and inter-layer diffusion.
    nameCol = rec.indexOf("ProcTlayerTimeDivisor");
    if (-1 == nameCol) { std::cout << "paramManager ProcTlayerTimeDivisor" << endl; }
    m_layerTimeDivisor = query.value(nameCol).toFloat();

    //Rolloff boundary. This is where highlights start to roll off.
    nameCol = rec.indexOf("ProcTrolloffBoundary");
    if (-1 == nameCol) { std::cout << "paramManager ProcTrolloffBoundary" << endl; }
    m_rolloffBoundary = query.value(nameCol).toInt();

    //Post-filmulator black clipping point
    nameCol = rec.indexOf("ProcTblackpoint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTblackpoint" << endl; }
    m_blackpoint = query.value(nameCol).toFloat();

    //Post-filmulator white clipping point
    nameCol = rec.indexOf("ProcTwhitepoint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTwhitepoint" << endl; }
    m_whitepoint = query.value(nameCol).toFloat();

    //Shadow control point x value
    nameCol = rec.indexOf("ProcTshadowsX");
    if (-1 == nameCol) { std::cout << "paramManager ProcTshadowsX" << endl; }
    m_shadowsX = query.value(nameCol).toFloat();

    //Shadow control point y value
    nameCol = rec.indexOf("ProcTshadowsY");
    if (-1 == nameCol) { std::cout << "paramManager ProcTshadowsY" << endl; }
    m_shadowsY = query.value(nameCol).toFloat();

    //Highlight control point x value
    nameCol = rec.indexOf("ProcThighlightsX");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightsX" << endl; }
    m_highlightsX = query.value(nameCol).toFloat();

    //Highlight control point y value
    nameCol = rec.indexOf("ProcThighlightsY");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightsY" << endl; }
    m_highlightsY = query.value(nameCol).toFloat();

    //Vibrance (saturation of less-saturated things)
    nameCol = rec.indexOf("ProcTvibrance");
    if (-1 == nameCol) { std::cout << "paramManager ProcTvibrance" << endl; }
    m_vibrance = query.value(nameCol).toFloat();

    //Saturation
    nameCol = rec.indexOf("ProcTsaturation");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsaturation" << endl; }
    m_saturation = query.value(nameCol).toFloat();

    //Rotation
    nameCol = rec.indexOf("ProcTrotation");
    if (-1 == nameCol) { std::cout << "paramManager ProcTrotation" << endl; }
    m_rotation = query.value(nameCol).toInt();
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
            tempParams.writeToDB(toImageId);
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

