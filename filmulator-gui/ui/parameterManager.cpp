#include "parameterManager.h"

ParameterManager::ParameterManager() : QObject(0)
{
}

ProcessingParameters ParameterManager::getParams()
{
    QMutexLocker locker(&mutex);
    return param;
}

void ParameterManager::setFilename(QString filename)
{
    //This isn't going to be used.
    QMutexLocker locker(&mutex);
    m_filename = filename;
    std::vector<std::string> inputFilenameList;
    inputFilenameList.push_back(filename.toStdString());
    param.filenameList = inputFilenameList;
    emit filenameChanged();
    emit paramChanged();
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
    emit caEnabledChanged();
    emit paramChanged();
}

void ParameterManager::setHighlights(int highlights)
{
    QMutexLocker locker(&mutex);
    m_highlights = highlights;
    param.highlights = highlights;
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
    emit exposureCompChanged();
    emit paramChanged();
}

void ParameterManager::setTemperature(double temperature)
{
    QMutexLocker locker(&mutex);
    m_temperature = temperature;
    param.temperature = temperature;
    emit temperatureChanged();
    emit paramChanged();
}

void ParameterManager::setTint(double tint)
{
    QMutexLocker locker(&mutex);
    m_tint = tint;
    param.tint = tint;
    emit tintChanged();
    emit paramChanged();
}

void ParameterManager::setInitialDeveloperConcentration(float initialDeveloperConcentration)
{
    QMutexLocker locker(&mutex);
    m_initialDeveloperConcentration = initialDeveloperConcentration;
    param.filmParams.initialDeveloperConcentration = initialDeveloperConcentration;
    emit initialDeveloperConcentrationChanged();
    emit paramChanged();
}

void ParameterManager::setReservoirThickness(float reservoirThickness)
{
    QMutexLocker locker(&mutex);
    m_reservoirThickness = reservoirThickness;
    param.filmParams.reservoirThickness = reservoirThickness;
    emit reservoirThicknessChanged();
    emit paramChanged();
}

void ParameterManager::setActiveLayerThickness(float activeLayerThickness)
{
    QMutexLocker locker(&mutex);
    m_activeLayerThickness = activeLayerThickness;
    param.filmParams.activeLayerThickness = activeLayerThickness;
    emit activeLayerThicknessChanged();
    emit paramChanged();
}

void ParameterManager::setCrystalsPerPixel(float crystalsPerPixel)
{
    QMutexLocker locker(&mutex);
    m_crystalsPerPixel = crystalsPerPixel;
    param.filmParams.crystalsPerPixel = crystalsPerPixel;
    emit crystalsPerPixelChanged();
    emit paramChanged();
}

void ParameterManager::setInitialCrystalRadius(float initialCrystalRadius)
{
    QMutexLocker locker(&mutex);
    m_initialCrystalRadius = initialCrystalRadius;
    param.filmParams.initialCrystalRadius = initialCrystalRadius;
    emit initialCrystalRadiusChanged();
    emit paramChanged();
}

void ParameterManager::setInitialSilverSaltDensity(float initialSilverSaltDensity)
{
    QMutexLocker locker(&mutex);
    m_initialSilverSaltDensity = initialSilverSaltDensity;
    param.filmParams.initialSilverSaltDensity = initialSilverSaltDensity;
    emit initialSilverSaltDensityChanged();
    emit paramChanged();
}

void ParameterManager::setDeveloperConsumptionConst(float developerConsumptionConst)
{
    QMutexLocker locker(&mutex);
    m_developerConsumptionConst = developerConsumptionConst;
    param.filmParams.developerConsumptionConst = developerConsumptionConst;
    emit developerConsumptionConstChanged();
    emit paramChanged();
}

void ParameterManager::setCrystalGrowthConst(float crystalGrowthConst)
{
    QMutexLocker locker(&mutex);
    m_crystalGrowthConst = crystalGrowthConst;
    param.filmParams.crystalGrowthConst = crystalGrowthConst;
    emit crystalGrowthConstChanged();
    emit paramChanged();
}

void ParameterManager::setSilverSaltConsumptionConst(float silverSaltConsumptionConst)
{
    QMutexLocker locker(&mutex);
    m_silverSaltConsumptionConst = silverSaltConsumptionConst;
    param.filmParams.silverSaltConsumptionConst = silverSaltConsumptionConst;
    emit silverSaltConsumptionConstChanged();
    emit paramChanged();
}

void ParameterManager::setTotalDevelopmentTime(float totalDevelopmentTime)
{
    QMutexLocker locker(&mutex);
    m_totalDevelopmentTime = totalDevelopmentTime;
    param.filmParams.totalDevelTime = totalDevelopmentTime;
    emit totalDevelopmentTimeChanged();
    emit paramChanged();
}

void ParameterManager::setAgitateCount(int agitateCount)
{
    QMutexLocker locker(&mutex);
    m_agitateCount = agitateCount;
    param.filmParams.agitateCount = agitateCount;
    emit agitateCountChanged();
    emit paramChanged();
}

void ParameterManager::setDevelopmentSteps(int developmentSteps)
{
    QMutexLocker locker(&mutex);
    m_developmentSteps = developmentSteps;
    param.filmParams.developmentSteps = developmentSteps;
    emit developmentStepsChanged();
    emit paramChanged();
}

void ParameterManager::setFilmArea(float filmArea)
{
    QMutexLocker locker(&mutex);
    m_filmArea = filmArea;
    param.filmParams.filmArea = filmArea;
    emit filmAreaChanged();
    emit paramChanged();
}

void ParameterManager::setSigmaConst(float sigmaConst)
{
    QMutexLocker locker(&mutex);
    m_sigmaConst = sigmaConst;
    param.filmParams.sigmaConst = sigmaConst;
    emit sigmaConstChanged();
    emit paramChanged();
}

void ParameterManager::setLayerMixConst(float layerMixConst)
{
    QMutexLocker locker(&mutex);
    m_layerMixConst = layerMixConst;
    param.filmParams.layerMixConst = layerMixConst;
    emit layerMixConstChanged();
    emit paramChanged();
}

void ParameterManager::setLayerTimeDivisor(float layerTimeDivisor)
{
    QMutexLocker locker(&mutex);
    m_layerTimeDivisor = layerTimeDivisor;
    param.filmParams.layerTimeDivisor = layerTimeDivisor;
    emit layerTimeDivisorChanged();
    emit paramChanged();
}

void ParameterManager::setRolloffBoundary(int rolloffBoundary)
{
    QMutexLocker locker(&mutex);
    m_rolloffBoundary = rolloffBoundary;
    param.filmParams.rolloffBoundary = rolloffBoundary;
    emit rolloffBoundaryChanged();
    emit paramChanged();
}

void ParameterManager::setBlackpoint(float blackpoint)
{
    QMutexLocker locker(&mutex);
    m_blackpoint = blackpoint;
    param.blackpoint = blackpoint;
    emit blackpointChanged();
    emit paramChanged();
}

void ParameterManager::setWhitepoint(float whitepoint)
{
    QMutexLocker locker(&mutex);
    m_whitepoint = whitepoint;
    param.whitepoint = whitepoint;
    emit whitepointChanged();
    emit paramChanged();
}

void ParameterManager::setShadowsX(float shadowsX)
{
    QMutexLocker locker(&mutex);
    m_shadowsX = shadowsX;
    param.shadowsX = shadowsX;
    emit shadowsXChanged();
    emit paramChanged();
}

void ParameterManager::setShadowsY(float shadowsY)
{
    QMutexLocker locker(&mutex);
    m_shadowsY = shadowsY;
    param.shadowsY = shadowsY;
    emit shadowsYChanged();
    emit paramChanged();
}

void ParameterManager::setHighlightsX(float highlightsX)
{
    QMutexLocker locker(&mutex);
    m_highlightsX = highlightsX;
    param.highlightsX = highlightsX;
    emit highlightsXChanged();
    emit paramChanged();
}

void ParameterManager::setHighlightsY(float highlightsY)
{
    QMutexLocker locker(&mutex);
    m_highlightsY = highlightsY;
    param.highlightsY = highlightsY;
    emit highlightsYChanged();
    emit paramChanged();
}

void ParameterManager::setVibrance(float vibrance)
{
    QMutexLocker locker(&mutex);
    m_vibrance = vibrance;
    param.vibrance = vibrance;
    emit vibranceChanged();
    emit paramChanged();
}

void ParameterManager::setSaturation(float saturation)
{
    QMutexLocker locker(&mutex);
    m_saturation = saturation;
    param.saturation = saturation;
    emit saturationChanged();
    emit paramChanged();
}

void ParameterManager::setRotation(int rotation)
{
    QMutexLocker locker(&mutex);
    m_rotation = rotation;
    param.rotation = rotation;
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
    emit rotationChanged();
    emit paramChanged();
}

QString ParameterManager::queryHelper(QString imageID, QString columnName)
{
    QString query = "SELECT ";
    query.append(columnName);
    query.append(" FROM ProcessingTable ");
    query.append("WHERE ProcT.procID = \"");
    query.append(imageID);
    query.append("\";");
    return query;
}

void ParameterManager::selectImage(QString imageID)
{
    QMutexLocker locker(&mutex);
    imageIndex = imageID;

    QString tempString = imageID;
    tempString.truncate(32);//length of md5
    QSqlQuery query;
    query.prepare("SELECT FTfilePath FROM FileTable WHERE FTfileID = ?;");
    query.bindValue(0, QVariant(tempString));
    query.exec();
    query.first();
    m_filename = query.value(0).toString();
    std::vector<string> tempFilename;
    tempFilename.push_back(m_filename.toStdString());
    param.filenameList = tempFilename;
    emit filenameChanged();

    //tiffIn should be false.
    m_tiffIn = false;
    emit tiffInChanged();

    //So should jpegIn.
    m_jpegIn = false;
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
    m_caEnabled = query.value(nameCol).toBool();
    param.caEnabled = m_caEnabled;
    emit caEnabledChanged();

    //Next is highlights (highlight recovery)
    nameCol = rec.indexOf("ProcThighlights");
    m_highlights = query.value(nameCol).toInt();
    param.highlights = m_highlights;
    emit highlightsChanged();

    //Exposure compensation
    nameCol = rec.indexOf("ProcTexposureComp");
    m_exposureComp = query.value(nameCol).toFloat();
    std::vector<float> exposureCompList;
    exposureCompList.push_back(m_exposureComp);
    param.exposureComp = exposureCompList;
    emit exposureCompChanged();

    //Temperature
    nameCol = rec.indexOf("ProcTtemperature");
    m_temperature = query.value(nameCol).toDouble();
    param.temperature = m_temperature;
    emit temperatureChanged();

    //Tint
    nameCol = rec.indexOf("ProcTtint");
    m_tint = query.value(nameCol).toDouble();
    param.tint = m_tint;
    emit tintChanged();

    //Initial developer concentration
    nameCol = rec.indexOf("ProcTinitialDeveloperConcentration");
    m_initialDeveloperConcentration = query.value(nameCol).toFloat();
    param.filmParams.initialDeveloperConcentration = m_initialDeveloperConcentration;
    emit initialDeveloperConcentrationChanged();

    //Reservoir thickness
    nameCol = rec.indexOf("ProcTreservoirThickness");
    m_reservoirThickness = query.value(nameCol).toFloat();
    param.filmParams.reservoirThickness = m_reservoirThickness;
    emit reservoirThicknessChanged();

    //Active layer thickness
    nameCol = rec.indexOf("ProcTactiveLayerThickness");
    m_activeLayerThickness = query.value(nameCol).toFloat();
    param.filmParams.activeLayerThickness = m_activeLayerThickness;
    emit activeLayerThicknessChanged();

    //Crystals per pixel
    nameCol = rec.indexOf("ProcTcrystalsPerPixel");
    m_crystalsPerPixel = query.value(nameCol).toFloat();
    param.filmParams.crystalsPerPixel = m_crystalsPerPixel;
    emit crystalsPerPixelChanged();

    //Initial crystal radius
    nameCol = rec.indexOf("ProcTinitialCrystalRadius");
    m_initialCrystalRadius = query.value(nameCol).toFloat();
    param.filmParams.initialCrystalRadius = m_initialCrystalRadius;
    emit initialCrystalRadiusChanged();

    //Initial silver salt area density
    nameCol = rec.indexOf("ProcTinitialSilverSaltDensity");
    m_initialSilverSaltDensity = query.value(nameCol).toFloat();
    param.filmParams.initialSilverSaltDensity = m_initialSilverSaltDensity;
    emit initialSilverSaltDensityChanged();

    //Developer consumption rate constant
    nameCol = rec.indexOf("ProcTdeveloperConsumptionConst");
    m_developerConsumptionConst = query.value(nameCol).toFloat();
    param.filmParams.developerConsumptionConst = m_developerConsumptionConst;
    emit developerConsumptionConstChanged();

    //Crystal growth rate constant
    nameCol = rec.indexOf("ProcTcrystalGrowthConst");
    m_crystalGrowthConst = query.value(nameCol).toFloat();
    param.filmParams.crystalGrowthConst = m_crystalGrowthConst;
    emit crystalGrowthConstChanged();

    //Silver halide consumption rate constant
    nameCol = rec.indexOf("ProcTsilverSaltConsumptionConst");
    m_silverSaltConsumptionConst = query.value(nameCol).toFloat();
    param.filmParams.silverSaltConsumptionConst = m_silverSaltConsumptionConst;
    emit crystalGrowthConstChanged();

    //Total development time
    nameCol = rec.indexOf("ProcTtotalDevelopmentTime");
    m_totalDevelopmentTime = query.value(nameCol).toFloat();
    param.filmParams.totalDevelTime = m_totalDevelopmentTime;
    emit totalDevelopmentTimeChanged();

    //Number of agitations
    nameCol = rec.indexOf("ProcTagitationCount");
    m_agitateCount = query.value(nameCol).toInt();
    param.filmParams.agitateCount = m_agitateCount;
    emit agitateCountChanged();

    //Number of simulation steps for development
    nameCol = rec.indexOf("ProcTdevelopmentResolution");
    m_developmentSteps = query.value(nameCol).toInt();
    param.filmParams.developmentSteps = m_developmentSteps;
    emit developmentStepsChanged();

    //Area of film for the simulation
    nameCol = rec.indexOf("ProcTfilmArea");
    m_filmArea = query.value(nameCol).toFloat();
    param.filmParams.filmArea = m_filmArea;
    emit filmAreaChanged();

    //A constant for the size of the diffusion. It...affects the same thing as film area.
    nameCol = rec.indexOf("ProcTsigmaConst");
    m_sigmaConst = query.value(nameCol).toFloat();
    param.filmParams.sigmaConst = m_sigmaConst;
    emit sigmaConstChanged();

    //Layer mix constant: the amount of active developer that gets exchanged with the reservoir.
    nameCol = rec.indexOf("ProcTlayerMixConst");
    m_layerMixConst = query.value(nameCol).toFloat();
    param.filmParams.layerMixConst = m_layerMixConst;
    emit layerMixConstChanged();

    //Layer time divisor: Controls the relative intra-layer and inter-layer diffusion.
    nameCol = rec.indexOf("ProcTlayerTimeDivisor");
    m_layerTimeDivisor = query.value(nameCol).toFloat();
    param.filmParams.layerTimeDivisor = m_layerTimeDivisor;
    emit layerTimeDivisorChanged();

    //Rolloff boundary. This is where highlights start to roll off.
    nameCol = rec.indexOf("ProcTrolloffBoundary");
    m_rolloffBoundary = query.value(nameCol).toInt();
    param.filmParams.rolloffBoundary = m_rolloffBoundary;
    emit rolloffBoundaryChanged();

    //Post-filmulator black clipping point
    nameCol = rec.indexOf("ProcTblackpoint");
    m_blackpoint = query.value(nameCol).toFloat();
    param.blackpoint = m_blackpoint;
    emit blackpointChanged();

    //Post-filmulator white clipping point
    nameCol = rec.indexOf("ProcTwhitepoint");
    m_whitepoint = query.value(nameCol).toFloat();
    param.whitepoint = m_whitepoint;
    emit whitepointChanged();

    //Shadow control point x value
    nameCol = rec.indexOf("ProcTshadowsX");
    m_shadowsX = query.value(nameCol).toFloat();
    param.shadowsX = m_shadowsX;
    emit shadowsXChanged();

    //Shadow control point y value
    nameCol = rec.indexOf("ProcTshadowsY");
    m_shadowsY = query.value(nameCol).toFloat();
    param.shadowsY = m_shadowsY;
    emit shadowsYChanged();

    //Highlight control point x value
    nameCol = rec.indexOf("ProcThighlightsX");
    m_highlightsX = query.value(nameCol).toFloat();
    param.highlightsX = m_highlightsX;
    emit highlightsXChanged();

    //Highlight control point y value
    nameCol = rec.indexOf("ProcThighlightsY");
    m_highlightsY = query.value(nameCol).toFloat();
    param.highlightsY = m_highlightsY;
    emit highlightsYChanged();

    //Vibrance (saturation of less-saturated things)
    nameCol = rec.indexOf("ProcTvibrance");
    m_vibrance = query.value(nameCol).toFloat();
    param.vibrance = m_vibrance;
    emit vibranceChanged();

    //Saturation
    nameCol = rec.indexOf("ProcTsaturation");
    m_saturation = query.value(nameCol).toFloat();
    param.saturation = m_saturation;
    emit saturationChanged();

    //Rotation
    nameCol = rec.indexOf("ProcTrotation");
    m_rotation = query.value(nameCol).toInt();
    param.rotation = m_rotation;
    emit rotationChanged();

    emit paramChanged();
}
