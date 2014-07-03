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
    inputFilenameList.push_back(filename);
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

void ParameterManager::setjpegIn(bool jpegIn)
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

void ParameterManager::setTotalDevelTime(float totalDevelTime)
{
    QMutexLocker locker(&mutex);
    m_totalDevelTime = totalDevelTime;
    param.filmParams.totalDevelTime = totalDevelTime;
    emit totalDevelTimeChanged();
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
    m_filename = tempString;
    param.filename = m_filename;
    emit filenameChanged();

    //tiffIn should be false.
    m_tiffIn = false;
    emit tiffInChanged();

    //So should jpegIn.
    m_jpegIn = false;
    emit jpegInChanged();


    //Everything else can be pulled from sql.
    //This query will be shared.
    QSqlQuery query;
    query.prepare("SELECT * FROM ProcessingTable WHERE ProcTprocID = ?;");
    query.bindValue(0, imageID);
    query.exec;

    //This will help us get the column index of the desired column name.
    QSqlRecord rec = query.record();
    int nameCol;

    query.first();

    //First is caEnabled.
    nameCol = rec.indexOf("ProcTcaEnabled");
    m_caEnabled = query.value(nameCol).toBool;
    param.caEnabled = m_caEnabled;
    emit caEnabledChanged();
}
