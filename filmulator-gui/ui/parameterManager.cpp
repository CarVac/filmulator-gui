#include "parameterManager.h"

using std::cout;
using std::endl;

ParameterManager::ParameterManager() : QObject(0)
{
    std::vector<std::string> inputFilenameList;
    inputFilenameList.push_back("");
    param.filenameList = inputFilenameList;
    param.tiffIn = false;
    param.jpegIn = false;
    param.caEnabled = false;
    param.highlights = 0;
    std::vector<float> exposureCompList;
    exposureCompList.push_back(0.0f);
    param.exposureComp = exposureCompList;
    param.temperature = 5200.0f;
    param.tint = 1.0f;
    param.filmParams.initialDeveloperConcentration = 1.0f;
    param.filmParams.reservoirThickness = 1000.0f;
    param.filmParams.activeLayerThickness = 0.1f;
    param.filmParams.crystalsPerPixel = 500.0f;
    param.filmParams.initialCrystalRadius = 0.00001f;
    param.filmParams.initialSilverSaltDensity = 1.0f;
    param.filmParams.initialSilverSaltDensity = 2000000.0f;
    param.filmParams.crystalGrowthConst =  0.00001f;
    param.filmParams.silverSaltConsumptionConst = 2000000.0f;
    param.filmParams.totalDevelTime = 100.0f;
    param.filmParams.agitateCount = 1;
    param.filmParams.developmentSteps = 12;
    param.filmParams.filmArea = 864.0f;
    param.filmParams.sigmaConst = 0.2f;
    param.filmParams.layerMixConst = 0.2f;
    param.filmParams.layerTimeDivisor = 20.0f;
    param.filmParams.rolloffBoundary = 51275;
    param.blackpoint = 0.0f;
    param.whitepoint = 0.002f;
    param.shadowsX = 0.25f;
    param.shadowsY = 0.25f;
    param.highlightsX = 0.75f;
    param.highlightsY = 0.75f;
    param.vibrance = 0.0f;
    param.saturation = 0.0f;
    param.rotation = 0;
}

ProcessingParameters ParameterManager::getParams()
{
    QMutexLocker locker(&mutex);
    return param;
}

void ParameterManager::setTiffIn(bool tiffIn)
{
    QMutexLocker locker(&mutex);
    m_tiffIn = tiffIn;
    param.tiffIn = tiffIn;
    emit tiffInChanged();
    emit paramChanged();
}

void ParameterManager::setJpegIn(bool jpegIn)
{
    QMutexLocker locker(&mutex);
    m_jpegIn = jpegIn;
    param.jpegIn = jpegIn;
    emit jpegInChanged();
    emit paramChanged();
}

void ParameterManager::setCaEnabled(bool caEnabled)
{
    QMutexLocker locker(&mutex);
    m_caEnabled = caEnabled;
    param.caEnabled = caEnabled;
    writeback(QString("ProcTcaEnabled"), QVariant(int(caEnabled)));
    emit caEnabledChanged();
    emit paramChanged();
}

void ParameterManager::setHighlights(int highlights)
{
    QMutexLocker locker(&mutex);
    m_highlights = highlights;
    param.highlights = highlights;
    writeback(QString("ProcThighlightRecovery"), QVariant(highlights));
    emit highlightsChanged();
    emit paramChanged();
}

void ParameterManager::setExposureComp(float exposureComp)
{
    QMutexLocker locker(&mutex);
    m_exposureComp = exposureComp;
    std::vector<float> exposureCompList;
    exposureCompList.push_back(exposureComp);
    param.exposureComp = exposureCompList;
    writeback(QString("ProcTexposureComp"), QVariant(exposureComp));
    emit exposureCompChanged();
    emit paramChanged();
}

void ParameterManager::setTemperature(double temperature)
{
    QMutexLocker locker(&mutex);
    m_temperature = temperature;
    param.temperature = temperature;
    writeback(QString("ProcTtemperature"), QVariant(temperature));
    emit temperatureChanged();
    emit paramChanged();
}

void ParameterManager::setTint(double tint)
{
    QMutexLocker locker(&mutex);
    m_tint = tint;
    param.tint = tint;
    writeback(QString("ProcTtint"), QVariant(tint));
    emit tintChanged();
    emit paramChanged();
}

void ParameterManager::setInitialDeveloperConcentration(float initialDeveloperConcentration)
{
    QMutexLocker locker(&mutex);
    m_initialDeveloperConcentration = initialDeveloperConcentration;
    param.filmParams.initialDeveloperConcentration = initialDeveloperConcentration;
    writeback(QString("ProcTinitialDeveloperConcentration"), QVariant(initialDeveloperConcentration));
    emit initialDeveloperConcentrationChanged();
    emit paramChanged();
}

void ParameterManager::setReservoirThickness(float reservoirThickness)
{
    QMutexLocker locker(&mutex);
    m_reservoirThickness = reservoirThickness;
    param.filmParams.reservoirThickness = reservoirThickness;
    writeback(QString("ProcTreservoirThickness"), QVariant(reservoirThickness));
    emit reservoirThicknessChanged();
    emit paramChanged();
}

void ParameterManager::setActiveLayerThickness(float activeLayerThickness)
{
    QMutexLocker locker(&mutex);
    m_activeLayerThickness = activeLayerThickness;
    param.filmParams.activeLayerThickness = activeLayerThickness;
    writeback(QString("ProcTactiveLayerThickness"), QVariant(activeLayerThickness));
    emit activeLayerThicknessChanged();
    emit paramChanged();
}

void ParameterManager::setCrystalsPerPixel(float crystalsPerPixel)
{
    QMutexLocker locker(&mutex);
    m_crystalsPerPixel = crystalsPerPixel;
    param.filmParams.crystalsPerPixel = crystalsPerPixel;
    writeback(QString("ProcTcrystalsPerPixel"), QVariant(crystalsPerPixel));
    emit crystalsPerPixelChanged();
    emit paramChanged();
}

void ParameterManager::setInitialCrystalRadius(float initialCrystalRadius)
{
    QMutexLocker locker(&mutex);
    m_initialCrystalRadius = initialCrystalRadius;
    param.filmParams.initialCrystalRadius = initialCrystalRadius;
    writeback(QString("ProcTinitialCrystalRadius"), QVariant(initialCrystalRadius));
    emit initialCrystalRadiusChanged();
    emit paramChanged();
}

void ParameterManager::setInitialSilverSaltDensity(float initialSilverSaltDensity)
{
    QMutexLocker locker(&mutex);
    m_initialSilverSaltDensity = initialSilverSaltDensity;
    param.filmParams.initialSilverSaltDensity = initialSilverSaltDensity;
    writeback(QString("ProcTinitialSilverSaltDensity"), QVariant(initialSilverSaltDensity));
    emit initialSilverSaltDensityChanged();
    emit paramChanged();
}

void ParameterManager::setDeveloperConsumptionConst(float developerConsumptionConst)
{
    QMutexLocker locker(&mutex);
    m_developerConsumptionConst = developerConsumptionConst;
    param.filmParams.developerConsumptionConst = developerConsumptionConst;
    writeback(QString("ProcTdeveloperConsumptionConst"), QVariant(developerConsumptionConst));
    emit developerConsumptionConstChanged();
    emit paramChanged();
}

void ParameterManager::setCrystalGrowthConst(float crystalGrowthConst)
{
    QMutexLocker locker(&mutex);
    m_crystalGrowthConst = crystalGrowthConst;
    param.filmParams.crystalGrowthConst = crystalGrowthConst;
    writeback(QString("ProcTcrystalGrowthConst"), QVariant(crystalGrowthConst));
    emit crystalGrowthConstChanged();
    emit paramChanged();
}

void ParameterManager::setSilverSaltConsumptionConst(float silverSaltConsumptionConst)
{
    QMutexLocker locker(&mutex);
    m_silverSaltConsumptionConst = silverSaltConsumptionConst;
    param.filmParams.silverSaltConsumptionConst = silverSaltConsumptionConst;
    writeback(QString("ProcTsilverSaltConsumptionConst"), QVariant(silverSaltConsumptionConst));
    emit silverSaltConsumptionConstChanged();
    emit paramChanged();
}

void ParameterManager::setTotalDevelopmentTime(float totalDevelopmentTime)
{
    QMutexLocker locker(&mutex);
    m_totalDevelopmentTime = totalDevelopmentTime;
    param.filmParams.totalDevelTime = totalDevelopmentTime;
    writeback(QString("ProcTtotalDevelopmentTime"), QVariant(totalDevelopmentTime));
    emit totalDevelopmentTimeChanged();
    emit paramChanged();
}

void ParameterManager::setAgitateCount(int agitateCount)
{
    QMutexLocker locker(&mutex);
    m_agitateCount = agitateCount;
    param.filmParams.agitateCount = agitateCount;
    writeback(QString("ProcTagitateCount"), QVariant(agitateCount));
    emit agitateCountChanged();
    emit paramChanged();
}

void ParameterManager::setDevelopmentSteps(int developmentSteps)
{
    QMutexLocker locker(&mutex);
    m_developmentSteps = developmentSteps;
    param.filmParams.developmentSteps = developmentSteps;
    writeback(QString("ProcTdevelopmentSteps"), QVariant(developmentSteps));
    emit developmentStepsChanged();
    emit paramChanged();
}

void ParameterManager::setFilmArea(float filmArea)
{
    QMutexLocker locker(&mutex);
    m_filmArea = filmArea;
    param.filmParams.filmArea = filmArea;
    writeback(QString("ProcTfilmArea"), QVariant(filmArea));
    emit filmAreaChanged();
    emit paramChanged();
}

void ParameterManager::setSigmaConst(float sigmaConst)
{
    QMutexLocker locker(&mutex);
    m_sigmaConst = sigmaConst;
    param.filmParams.sigmaConst = sigmaConst;
    writeback(QString("ProcTsigmaConst"), QVariant(sigmaConst));
    emit sigmaConstChanged();
    emit paramChanged();
}

void ParameterManager::setLayerMixConst(float layerMixConst)
{
    QMutexLocker locker(&mutex);
    m_layerMixConst = layerMixConst;
    param.filmParams.layerMixConst = layerMixConst;
    writeback(QString("ProcTlayerMixConst"), QVariant(layerMixConst));
    emit layerMixConstChanged();
    emit paramChanged();
}

void ParameterManager::setLayerTimeDivisor(float layerTimeDivisor)
{
    QMutexLocker locker(&mutex);
    m_layerTimeDivisor = layerTimeDivisor;
    param.filmParams.layerTimeDivisor = layerTimeDivisor;
    writeback(QString("ProcTlayerTimeDivisor"), QVariant(layerTimeDivisor));
    emit layerTimeDivisorChanged();
    emit paramChanged();
}

void ParameterManager::setRolloffBoundary(int rolloffBoundary)
{
    QMutexLocker locker(&mutex);
    m_rolloffBoundary = rolloffBoundary;
    param.filmParams.rolloffBoundary = rolloffBoundary;
    writeback(QString("ProcTrolloffBoundary"), QVariant(rolloffBoundary));
    emit rolloffBoundaryChanged();
    emit paramChanged();
}

void ParameterManager::setBlackpoint(float blackpoint)
{
    QMutexLocker locker(&mutex);
    m_blackpoint = blackpoint;
    param.blackpoint = blackpoint;
    writeback(QString("ProcTblackpoint"), QVariant(blackpoint));
    emit blackpointChanged();
    emit paramChanged();
}

void ParameterManager::setWhitepoint(float whitepoint)
{
    QMutexLocker locker(&mutex);
    m_whitepoint = whitepoint;
    param.whitepoint = whitepoint;
    writeback(QString("ProcTwhitepoint"), QVariant(whitepoint));
    emit whitepointChanged();
    emit paramChanged();
}

void ParameterManager::setShadowsX(float shadowsX)
{
    QMutexLocker locker(&mutex);
    m_shadowsX = shadowsX;
    param.shadowsX = shadowsX;
    writeback(QString("ProcTshadowsX"), QVariant(shadowsX));
    emit shadowsXChanged();
    emit paramChanged();
}

void ParameterManager::setShadowsY(float shadowsY)
{
    QMutexLocker locker(&mutex);
    m_shadowsY = shadowsY;
    param.shadowsY = shadowsY;
    writeback(QString("ProcTshadowsY"), QVariant(shadowsY));
    emit shadowsYChanged();
    emit paramChanged();
}

void ParameterManager::setHighlightsX(float highlightsX)
{
    QMutexLocker locker(&mutex);
    m_highlightsX = highlightsX;
    param.highlightsX = highlightsX;
    writeback(QString("ProcThighlightsX"), QVariant(highlightsX));
    emit highlightsXChanged();
    emit paramChanged();
}

void ParameterManager::setHighlightsY(float highlightsY)
{
    QMutexLocker locker(&mutex);
    m_highlightsY = highlightsY;
    param.highlightsY = highlightsY;
    writeback(QString("ProcThighlightsY"), QVariant(highlightsY));
    emit highlightsYChanged();
    emit paramChanged();
}

void ParameterManager::setVibrance(float vibrance)
{
    QMutexLocker locker(&mutex);
    m_vibrance = vibrance;
    param.vibrance = vibrance;
    writeback(QString("ProcTvibrance"), QVariant(vibrance));
    emit vibranceChanged();
    emit paramChanged();
}

void ParameterManager::setSaturation(float saturation)
{
    QMutexLocker locker(&mutex);
    m_saturation = saturation;
    param.saturation = saturation;
    writeback(QString("ProcTsaturation"), QVariant(saturation));
    emit saturationChanged();
    emit paramChanged();
}

void ParameterManager::setRotation(int rotation)
{
    QMutexLocker locker(&mutex);
    m_rotation = rotation;
    param.rotation = rotation;
    writeback(QString("ProcTrotation"), QVariant(rotation));
    emit rotationChanged();
    emit paramChanged();
}

void ParameterManager::rotateRight()
{
    QMutexLocker locker(&mutex);
    m_rotation--;
    if (m_rotation < 0)
    {
        m_rotation += 4;
    }
    param.rotation = m_rotation;
    writeback(QString("ProcTrotation"), QVariant(m_rotation));
    emit rotationChanged();
    emit paramChanged();
}

void ParameterManager::rotateLeft()
{
    QMutexLocker locker(&mutex);
    m_rotation++;
    if (m_rotation > 3)
    {
        m_rotation -= 4;
    }
    param.rotation = m_rotation;
    writeback(QString("ProcTrotation"), QVariant(m_rotation));
    emit rotationChanged();
    emit paramChanged();
}

//This should be protected by the in its calling function.
//It writes back a value valueIn to the database column colName for the currrent
// image specified by imageIndex.
void ParameterManager::writeback(QString colName, QVariant valueIn)
{
    //Write back the slider to the database.
    QSqlQuery query;
    query.prepare(QString("UPDATE ProcessingTable SET %1 = ? WHERE ProcTprocID = ?;").arg(colName));
    query.bindValue(0, valueIn);
    query.bindValue(1, imageIndex);
    query.exec();
    //Write that it's been edited to the SearchTable (actually writing the edit time)
    QDateTime now = QDateTime::currentDateTime();
    query.prepare("UPDATE SearchTable SET STlastProcessedTime = ? WHERE STSearchID = ?;");
    query.bindValue(0, QVariant(now.toTime_t()));
    query.bindValue(1, imageIndex);
    query.exec();
    //Write that it's been edited to the QueueTable
    query.prepare("UPDATE QueueTable SET QTprocessed = ? WHERE QTsearchID = ?;");
    query.bindValue(0, QVariant(true));
    query.bindValue(1, imageIndex);
    query.exec();
}

void ParameterManager::selectImage(QString imageID)
{
    QMutexLocker locker(&mutex);
    imageIndex = imageID;
    emit imageIndexChanged();

    QString tempString = imageID;
    tempString.truncate(32);//length of md5
    QSqlQuery query;
    query.prepare("SELECT FTfilePath FROM FileTable WHERE FTfileID = ?;");
    query.bindValue(0, QVariant(tempString));
    query.exec();
    query.first();
    filename = query.value(0).toString();
    std::vector<string> tempFilename;
    tempFilename.push_back(filename.toStdString());
    param.filenameList = tempFilename;
    //You _cannot_ emit filenameChanged() here because it needs to
    //happen after all the sliders have been set.
    //It's called at the end.

    //tiffIn should be false.
    m_tiffIn = false;
    param.tiffIn = false;
    emit tiffInChanged();

    //So should jpegIn.
    m_jpegIn = false;
    param.jpegIn = false;
    emit jpegInChanged();


    //Everything else can be pulled from sql.
    //This query will be shared.
    query.prepare("SELECT * FROM ProcessingTable WHERE ProcTprocID = ?;");
    query.bindValue(0, imageID);
    query.exec();

    //This will help us get the column index of the desired column name.
    QSqlRecord rec = query.record();
    int nameCol;

    query.first();

    //First is caEnabled.
    nameCol = rec.indexOf("ProcTcaEnabled");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcaEnabled" << endl; }
    m_caEnabled = query.value(nameCol).toBool();
    param.caEnabled = m_caEnabled;
    emit caEnabledChanged();

    //Next is highlights (highlight recovery)
    nameCol = rec.indexOf("ProcThighlightRecovery");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightRecovery" << endl; }
    m_highlights = query.value(nameCol).toInt();
    param.highlights = m_highlights;
    emit highlightsChanged();

    //Exposure compensation
    nameCol = rec.indexOf("ProcTexposureComp");
    if (-1 == nameCol) { std::cout << "paramManager ProcTexposureComp" << endl; }
    m_exposureComp = query.value(nameCol).toFloat();
    std::vector<float> exposureCompList;
    exposureCompList.push_back(m_exposureComp);
    param.exposureComp = exposureCompList;
    emit exposureCompChanged();

    //Temperature
    nameCol = rec.indexOf("ProcTtemperature");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtemperature" << endl; }
    m_temperature = query.value(nameCol).toDouble();
    param.temperature = m_temperature;
    emit temperatureChanged();

    //Tint
    nameCol = rec.indexOf("ProcTtint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtint" << endl; }
    m_tint = query.value(nameCol).toDouble();
    param.tint = m_tint;
    emit tintChanged();

    //Initial developer concentration
    nameCol = rec.indexOf("ProcTinitialDeveloperConcentration");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialDeveloperConcentration" << endl; }
    m_initialDeveloperConcentration = query.value(nameCol).toFloat();
    param.filmParams.initialDeveloperConcentration = m_initialDeveloperConcentration;
    emit initialDeveloperConcentrationChanged();

    //Reservoir thickness
    nameCol = rec.indexOf("ProcTreservoirThickness");
    if (-1 == nameCol) { std::cout << "paramManager ProcTreservoirThickness" << endl; }
    m_reservoirThickness = query.value(nameCol).toFloat();
    param.filmParams.reservoirThickness = m_reservoirThickness;
    emit reservoirThicknessChanged();

    //Active layer thickness
    nameCol = rec.indexOf("ProcTactiveLayerThickness");
    if (-1 == nameCol) { std::cout << "paramManager ProcTactiveLayerThickness" << endl; }
    m_activeLayerThickness = query.value(nameCol).toFloat();
    param.filmParams.activeLayerThickness = m_activeLayerThickness;
    emit activeLayerThicknessChanged();

    //Crystals per pixel
    nameCol = rec.indexOf("ProcTcrystalsPerPixel");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcrystalsPerPixel" << endl; }
    m_crystalsPerPixel = query.value(nameCol).toFloat();
    param.filmParams.crystalsPerPixel = m_crystalsPerPixel;
    emit crystalsPerPixelChanged();

    //Initial crystal radius
    nameCol = rec.indexOf("ProcTinitialCrystalRadius");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialCrystalRadius" << endl; }
    m_initialCrystalRadius = query.value(nameCol).toFloat();
    param.filmParams.initialCrystalRadius = m_initialCrystalRadius;
    emit initialCrystalRadiusChanged();

    //Initial silver salt area density
    nameCol = rec.indexOf("ProcTinitialSilverSaltDensity");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialSilverSaltDensity" << endl; }
    m_initialSilverSaltDensity = query.value(nameCol).toFloat();
    param.filmParams.initialSilverSaltDensity = m_initialSilverSaltDensity;
    emit initialSilverSaltDensityChanged();

    //Developer consumption rate constant
    nameCol = rec.indexOf("ProcTdeveloperConsumptionConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTdeveloperConsumptionConst" << endl; }
    m_developerConsumptionConst = query.value(nameCol).toFloat();
    param.filmParams.developerConsumptionConst = m_developerConsumptionConst;
    emit developerConsumptionConstChanged();

    //Crystal growth rate constant
    nameCol = rec.indexOf("ProcTcrystalGrowthConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcrystalGrowthConst" << endl; }
    m_crystalGrowthConst = query.value(nameCol).toFloat();
    param.filmParams.crystalGrowthConst = m_crystalGrowthConst;
    emit crystalGrowthConstChanged();

    //Silver halide consumption rate constant
    nameCol = rec.indexOf("ProcTsilverSaltConsumptionConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsilverSaltConsumptionConst" << endl; }
    m_silverSaltConsumptionConst = query.value(nameCol).toFloat();
    param.filmParams.silverSaltConsumptionConst = m_silverSaltConsumptionConst;
    emit crystalGrowthConstChanged();

    //Total development time
    nameCol = rec.indexOf("ProcTtotalDevelopmentTime");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtotalDevelopmentTime" << endl; }
    m_totalDevelopmentTime = query.value(nameCol).toFloat();
    param.filmParams.totalDevelTime = m_totalDevelopmentTime;
    emit totalDevelopmentTimeChanged();

    //Number of agitations
    nameCol = rec.indexOf("ProcTagitateCount");
    if (-1 == nameCol) { std::cout << "paramManager ProcTagitateCount" << endl; }
    m_agitateCount = query.value(nameCol).toInt();
    param.filmParams.agitateCount = m_agitateCount;
    emit agitateCountChanged();

    //Number of simulation steps for development
    nameCol = rec.indexOf("ProcTdevelopmentSteps");
    if (-1 == nameCol) { std::cout << "paramManager ProcTdevelopmentSteps" << endl; }
    m_developmentSteps = query.value(nameCol).toInt();
    param.filmParams.developmentSteps = m_developmentSteps;
    emit developmentStepsChanged();

    //Area of film for the simulation
    nameCol = rec.indexOf("ProcTfilmArea");
    if (-1 == nameCol) { std::cout << "paramManager ProcTfilmArea" << endl; }
    m_filmArea = query.value(nameCol).toFloat();
    param.filmParams.filmArea = m_filmArea;
    emit filmAreaChanged();

    //A constant for the size of the diffusion. It...affects the same thing as film area.
    nameCol = rec.indexOf("ProcTsigmaConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsigmaConst" << endl; }
    m_sigmaConst = query.value(nameCol).toFloat();
    param.filmParams.sigmaConst = m_sigmaConst;
    emit sigmaConstChanged();

    //Layer mix constant: the amount of active developer that gets exchanged with the reservoir.
    nameCol = rec.indexOf("ProcTlayerMixConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTlayerMixConst" << endl; }
    m_layerMixConst = query.value(nameCol).toFloat();
    param.filmParams.layerMixConst = m_layerMixConst;
    emit layerMixConstChanged();

    //Layer time divisor: Controls the relative intra-layer and inter-layer diffusion.
    nameCol = rec.indexOf("ProcTlayerTimeDivisor");
    if (-1 == nameCol) { std::cout << "paramManager ProcTlayerTimeDivisor" << endl; }
    m_layerTimeDivisor = query.value(nameCol).toFloat();
    param.filmParams.layerTimeDivisor = m_layerTimeDivisor;
    emit layerTimeDivisorChanged();

    //Rolloff boundary. This is where highlights start to roll off.
    nameCol = rec.indexOf("ProcTrolloffBoundary");
    if (-1 == nameCol) { std::cout << "paramManager ProcTrolloffBoundary" << endl; }
    m_rolloffBoundary = query.value(nameCol).toInt();
    param.filmParams.rolloffBoundary = m_rolloffBoundary;
    emit rolloffBoundaryChanged();

    //Post-filmulator black clipping point
    nameCol = rec.indexOf("ProcTblackpoint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTblackpoint" << endl; }
    m_blackpoint = query.value(nameCol).toFloat();
    param.blackpoint = m_blackpoint;
    emit blackpointChanged();

    //Post-filmulator white clipping point
    nameCol = rec.indexOf("ProcTwhitepoint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTwhitepoint" << endl; }
    m_whitepoint = query.value(nameCol).toFloat();
    param.whitepoint = m_whitepoint;
    emit whitepointChanged();

    //Shadow control point x value
    nameCol = rec.indexOf("ProcTshadowsX");
    if (-1 == nameCol) { std::cout << "paramManager ProcTshadowsX" << endl; }
    m_shadowsX = query.value(nameCol).toFloat();
    param.shadowsX = m_shadowsX;
    emit shadowsXChanged();

    //Shadow control point y value
    nameCol = rec.indexOf("ProcTshadowsY");
    if (-1 == nameCol) { std::cout << "paramManager ProcTshadowsY" << endl; }
    m_shadowsY = query.value(nameCol).toFloat();
    param.shadowsY = m_shadowsY;
    emit shadowsYChanged();

    //Highlight control point x value
    nameCol = rec.indexOf("ProcThighlightsX");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightsX" << endl; }
    m_highlightsX = query.value(nameCol).toFloat();
    param.highlightsX = m_highlightsX;
    emit highlightsXChanged();

    //Highlight control point y value
    nameCol = rec.indexOf("ProcThighlightsY");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightsY" << endl; }
    m_highlightsY = query.value(nameCol).toFloat();
    param.highlightsY = m_highlightsY;
    emit highlightsYChanged();

    //Vibrance (saturation of less-saturated things)
    nameCol = rec.indexOf("ProcTvibrance");
    if (-1 == nameCol) { std::cout << "paramManager ProcTvibrance" << endl; }
    m_vibrance = query.value(nameCol).toFloat();
    param.vibrance = m_vibrance;
    emit vibranceChanged();

    //Saturation
    nameCol = rec.indexOf("ProcTsaturation");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsaturation" << endl; }
    m_saturation = query.value(nameCol).toFloat();
    param.saturation = m_saturation;
    emit saturationChanged();

    //Rotation
    nameCol = rec.indexOf("ProcTrotation");
    if (-1 == nameCol) { std::cout << "paramManager ProcTrotation" << endl; }
    m_rotation = query.value(nameCol).toInt();
    param.rotation = m_rotation;
    emit rotationChanged();

    locker.unlock();
    emit filenameChanged();
    emit paramChanged();
}
