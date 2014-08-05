#include "parameterManager.h"

using std::cout;
using std::endl;

ParameterManager::ParameterManager() : QObject(0)
{
    paramChangeEnabled = true;
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
    paramChangeWrapper(QString("setTiff"));
}

void ParameterManager::setJpegIn(bool jpegIn)
{
    QMutexLocker locker(&mutex);
    m_jpegIn = jpegIn;
    param.jpegIn = jpegIn;
    emit jpegInChanged();
    paramChangeWrapper(QString("setJpeg"));
}

void ParameterManager::setCaEnabled(bool caEnabled)
{
    QMutexLocker locker(&mutex);
    m_caEnabled = caEnabled;
    param.caEnabled = caEnabled;
    writeback(QString("ProcTcaEnabled"), QVariant(int(caEnabled)));
    emit caEnabledChanged();
    paramChangeWrapper(QString("setCaEnabled"));
}

void ParameterManager::setHighlights(int highlights)
{
    QMutexLocker locker(&mutex);
    m_highlights = highlights;
    param.highlights = highlights;
    writeback(QString("ProcThighlightRecovery"), QVariant(highlights));
    emit highlightsChanged();
    paramChangeWrapper(QString("setHighlights"));
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
    paramChangeWrapper(QString("setExposureComp"));
}

void ParameterManager::setTemperature(double temperature)
{
    QMutexLocker locker(&mutex);
    m_temperature = temperature;
    param.temperature = temperature;
    writeback(QString("ProcTtemperature"), QVariant(temperature));
    emit temperatureChanged();
    paramChangeWrapper(QString("setTemperature"));
}

void ParameterManager::setTint(double tint)
{
    QMutexLocker locker(&mutex);
    m_tint = tint;
    param.tint = tint;
    writeback(QString("ProcTtint"), QVariant(tint));
    emit tintChanged();
    paramChangeWrapper(QString("setTint"));
}

void ParameterManager::setInitialDeveloperConcentration(float initialDeveloperConcentration)
{
    QMutexLocker locker(&mutex);
    m_initialDeveloperConcentration = initialDeveloperConcentration;
    param.filmParams.initialDeveloperConcentration = initialDeveloperConcentration;
    writeback(QString("ProcTinitialDeveloperConcentration"), QVariant(initialDeveloperConcentration));
    emit initialDeveloperConcentrationChanged();
    paramChangeWrapper(QString("setInitialDeveloperConcentration"));
}

void ParameterManager::setReservoirThickness(float reservoirThickness)
{
    QMutexLocker locker(&mutex);
    m_reservoirThickness = reservoirThickness;
    param.filmParams.reservoirThickness = reservoirThickness;
    writeback(QString("ProcTreservoirThickness"), QVariant(reservoirThickness));
    emit reservoirThicknessChanged();
    paramChangeWrapper(QString("setReservoirThickness"));
}

void ParameterManager::setActiveLayerThickness(float activeLayerThickness)
{
    QMutexLocker locker(&mutex);
    m_activeLayerThickness = activeLayerThickness;
    param.filmParams.activeLayerThickness = activeLayerThickness;
    writeback(QString("ProcTactiveLayerThickness"), QVariant(activeLayerThickness));
    emit activeLayerThicknessChanged();
    paramChangeWrapper(QString("setActiveLayerThickness"));
}

void ParameterManager::setCrystalsPerPixel(float crystalsPerPixel)
{
    QMutexLocker locker(&mutex);
    m_crystalsPerPixel = crystalsPerPixel;
    param.filmParams.crystalsPerPixel = crystalsPerPixel;
    writeback(QString("ProcTcrystalsPerPixel"), QVariant(crystalsPerPixel));
    emit crystalsPerPixelChanged();
    paramChangeWrapper(QString("setCrystalsPerPixel"));
}

void ParameterManager::setInitialCrystalRadius(float initialCrystalRadius)
{
    QMutexLocker locker(&mutex);
    m_initialCrystalRadius = initialCrystalRadius;
    param.filmParams.initialCrystalRadius = initialCrystalRadius;
    writeback(QString("ProcTinitialCrystalRadius"), QVariant(initialCrystalRadius));
    emit initialCrystalRadiusChanged();
    paramChangeWrapper(QString("setInitialCrystalRadius"));
}

void ParameterManager::setInitialSilverSaltDensity(float initialSilverSaltDensity)
{
    QMutexLocker locker(&mutex);
    m_initialSilverSaltDensity = initialSilverSaltDensity;
    param.filmParams.initialSilverSaltDensity = initialSilverSaltDensity;
    writeback(QString("ProcTinitialSilverSaltDensity"), QVariant(initialSilverSaltDensity));
    emit initialSilverSaltDensityChanged();
    paramChangeWrapper(QString("setInitialSilverSaltDensity"));
}

void ParameterManager::setDeveloperConsumptionConst(float developerConsumptionConst)
{
    QMutexLocker locker(&mutex);
    m_developerConsumptionConst = developerConsumptionConst;
    param.filmParams.developerConsumptionConst = developerConsumptionConst;
    writeback(QString("ProcTdeveloperConsumptionConst"), QVariant(developerConsumptionConst));
    emit developerConsumptionConstChanged();
    paramChangeWrapper(QString("setDeveloperConsumptionConst"));
}

void ParameterManager::setCrystalGrowthConst(float crystalGrowthConst)
{
    QMutexLocker locker(&mutex);
    m_crystalGrowthConst = crystalGrowthConst;
    param.filmParams.crystalGrowthConst = crystalGrowthConst;
    writeback(QString("ProcTcrystalGrowthConst"), QVariant(crystalGrowthConst));
    emit crystalGrowthConstChanged();
    paramChangeWrapper(QString("setCrystalGrowthConst"));
}

void ParameterManager::setSilverSaltConsumptionConst(float silverSaltConsumptionConst)
{
    QMutexLocker locker(&mutex);
    m_silverSaltConsumptionConst = silverSaltConsumptionConst;
    param.filmParams.silverSaltConsumptionConst = silverSaltConsumptionConst;
    writeback(QString("ProcTsilverSaltConsumptionConst"), QVariant(silverSaltConsumptionConst));
    emit silverSaltConsumptionConstChanged();
    paramChangeWrapper(QString("setSilverSaltConsumptionConst"));
}

void ParameterManager::setTotalDevelopmentTime(float totalDevelopmentTime)
{
    QMutexLocker locker(&mutex);
    m_totalDevelopmentTime = totalDevelopmentTime;
    param.filmParams.totalDevelTime = totalDevelopmentTime;
    writeback(QString("ProcTtotalDevelopmentTime"), QVariant(totalDevelopmentTime));
    emit totalDevelopmentTimeChanged();
    paramChangeWrapper(QString("setTotalDevelopmentTime"));
}

void ParameterManager::setAgitateCount(int agitateCount)
{
    QMutexLocker locker(&mutex);
    m_agitateCount = agitateCount;
    param.filmParams.agitateCount = agitateCount;
    writeback(QString("ProcTagitateCount"), QVariant(agitateCount));
    emit agitateCountChanged();
    paramChangeWrapper(QString("setAgitateCount"));
}

void ParameterManager::setDevelopmentSteps(int developmentSteps)
{
    QMutexLocker locker(&mutex);
    m_developmentSteps = developmentSteps;
    param.filmParams.developmentSteps = developmentSteps;
    writeback(QString("ProcTdevelopmentSteps"), QVariant(developmentSteps));
    emit developmentStepsChanged();
    paramChangeWrapper(QString("setDevelopmentSteps"));
}

void ParameterManager::setFilmArea(float filmArea)
{
    QMutexLocker locker(&mutex);
    m_filmArea = filmArea;
    param.filmParams.filmArea = filmArea;
    writeback(QString("ProcTfilmArea"), QVariant(filmArea));
    emit filmAreaChanged();
    paramChangeWrapper(QString("setFilmArea"));
}

void ParameterManager::setSigmaConst(float sigmaConst)
{
    QMutexLocker locker(&mutex);
    m_sigmaConst = sigmaConst;
    param.filmParams.sigmaConst = sigmaConst;
    writeback(QString("ProcTsigmaConst"), QVariant(sigmaConst));
    emit sigmaConstChanged();
    paramChangeWrapper(QString("setSigmaConst"));
}

void ParameterManager::setLayerMixConst(float layerMixConst)
{
    QMutexLocker locker(&mutex);
    m_layerMixConst = layerMixConst;
    param.filmParams.layerMixConst = layerMixConst;
    writeback(QString("ProcTlayerMixConst"), QVariant(layerMixConst));
    emit layerMixConstChanged();
    paramChangeWrapper(QString("setLayerMixConst"));
}

void ParameterManager::setLayerTimeDivisor(float layerTimeDivisor)
{
    QMutexLocker locker(&mutex);
    m_layerTimeDivisor = layerTimeDivisor;
    param.filmParams.layerTimeDivisor = layerTimeDivisor;
    writeback(QString("ProcTlayerTimeDivisor"), QVariant(layerTimeDivisor));
    emit layerTimeDivisorChanged();
    paramChangeWrapper(QString("setLayerTimeDivisor"));
}

void ParameterManager::setRolloffBoundary(int rolloffBoundary)
{
    QMutexLocker locker(&mutex);
    m_rolloffBoundary = rolloffBoundary;
    param.filmParams.rolloffBoundary = rolloffBoundary;
    writeback(QString("ProcTrolloffBoundary"), QVariant(rolloffBoundary));
    emit rolloffBoundaryChanged();
    paramChangeWrapper(QString("setRolloffBoundary"));
}

void ParameterManager::setBlackpoint(float blackpoint)
{
    QMutexLocker locker(&mutex);
    m_blackpoint = blackpoint;
    param.blackpoint = blackpoint;
    writeback(QString("ProcTblackpoint"), QVariant(blackpoint));
    emit blackpointChanged();
    paramChangeWrapper(QString("setBlackpoint"));
}

void ParameterManager::setWhitepoint(float whitepoint)
{
    QMutexLocker locker(&mutex);
    m_whitepoint = whitepoint;
    param.whitepoint = whitepoint;
    writeback(QString("ProcTwhitepoint"), QVariant(whitepoint));
    emit whitepointChanged();
    paramChangeWrapper(QString("setWhitepoint"));
}

void ParameterManager::setShadowsX(float shadowsX)
{
    QMutexLocker locker(&mutex);
    m_shadowsX = shadowsX;
    param.shadowsX = shadowsX;
    writeback(QString("ProcTshadowsX"), QVariant(shadowsX));
    emit shadowsXChanged();
    paramChangeWrapper(QString("setShadowsX"));
}

void ParameterManager::setShadowsY(float shadowsY)
{
    QMutexLocker locker(&mutex);
    m_shadowsY = shadowsY;
    param.shadowsY = shadowsY;
    writeback(QString("ProcTshadowsY"), QVariant(shadowsY));
    emit shadowsYChanged();
    paramChangeWrapper(QString("setShadowsY"));
}

void ParameterManager::setHighlightsX(float highlightsX)
{
    QMutexLocker locker(&mutex);
    m_highlightsX = highlightsX;
    param.highlightsX = highlightsX;
    writeback(QString("ProcThighlightsX"), QVariant(highlightsX));
    emit highlightsXChanged();
    paramChangeWrapper(QString("setHighlightsX"));
}

void ParameterManager::setHighlightsY(float highlightsY)
{
    QMutexLocker locker(&mutex);
    m_highlightsY = highlightsY;
    param.highlightsY = highlightsY;
    writeback(QString("ProcThighlightsY"), QVariant(highlightsY));
    emit highlightsYChanged();
    paramChangeWrapper(QString("setHighlightsY"));
}

void ParameterManager::setVibrance(float vibrance)
{
    QMutexLocker locker(&mutex);
    m_vibrance = vibrance;
    param.vibrance = vibrance;
    writeback(QString("ProcTvibrance"), QVariant(vibrance));
    emit vibranceChanged();
    paramChangeWrapper(QString("setVibrance"));
}

void ParameterManager::setSaturation(float saturation)
{
    QMutexLocker locker(&mutex);
    m_saturation = saturation;
    param.saturation = saturation;
    writeback(QString("ProcTsaturation"), QVariant(saturation));
    emit saturationChanged();
    paramChangeWrapper(QString("setSaturation"));
}

void ParameterManager::setRotation(int rotation)
{
    QMutexLocker locker(&mutex);
    m_rotation = rotation;
    param.rotation = rotation;
    writeback(QString("ProcTrotation"), QVariant(rotation));
    emit rotationChanged();
    paramChangeWrapper(QString("setRotation"));
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
    paramChangeWrapper(QString("rotateRight"));
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
    paramChangeWrapper(QString("rotateLeft"));
}

//This should be protected by the in its calling function.
//It writes back a value valueIn to the database column colName for the currrent
// image specified by imageIndex.
void ParameterManager::writeback(QString colName, QVariant valueIn)
{
    //This only needs to happen when actually done by a slider.
    //It prevents this from occurring when loading an image.
    if (paramChangeEnabled)
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
}

void ParameterManager::selectImage(QString imageID)
{
    QMutexLocker locker(&mutex);//Make all the param changes happen together.
    disableParamChange();//Prevent aborting of computation.
    qDebug() << paramChangeEnabled;

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

    //tiffIn should be false.
    m_tiffIn = false;
    param.tiffIn = false;

    //So should jpegIn.
    m_jpegIn = false;
    param.jpegIn = false;


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

    //Next is highlights (highlight recovery)
    nameCol = rec.indexOf("ProcThighlightRecovery");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightRecovery" << endl; }
    m_highlights = query.value(nameCol).toInt();
    param.highlights = m_highlights;

    //Exposure compensation
    nameCol = rec.indexOf("ProcTexposureComp");
    if (-1 == nameCol) { std::cout << "paramManager ProcTexposureComp" << endl; }
    m_exposureComp = query.value(nameCol).toFloat();
    std::vector<float> exposureCompList;
    exposureCompList.push_back(m_exposureComp);
    param.exposureComp = exposureCompList;

    //Temperature
    nameCol = rec.indexOf("ProcTtemperature");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtemperature" << endl; }
    m_temperature = query.value(nameCol).toDouble();
    param.temperature = m_temperature;

    //Tint
    nameCol = rec.indexOf("ProcTtint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtint" << endl; }
    m_tint = query.value(nameCol).toDouble();
    param.tint = m_tint;

    //Initial developer concentration
    nameCol = rec.indexOf("ProcTinitialDeveloperConcentration");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialDeveloperConcentration" << endl; }
    m_initialDeveloperConcentration = query.value(nameCol).toFloat();
    param.filmParams.initialDeveloperConcentration = m_initialDeveloperConcentration;

    //Reservoir thickness
    nameCol = rec.indexOf("ProcTreservoirThickness");
    if (-1 == nameCol) { std::cout << "paramManager ProcTreservoirThickness" << endl; }
    m_reservoirThickness = query.value(nameCol).toFloat();
    param.filmParams.reservoirThickness = m_reservoirThickness;

    //Active layer thickness
    nameCol = rec.indexOf("ProcTactiveLayerThickness");
    if (-1 == nameCol) { std::cout << "paramManager ProcTactiveLayerThickness" << endl; }
    m_activeLayerThickness = query.value(nameCol).toFloat();
    param.filmParams.activeLayerThickness = m_activeLayerThickness;

    //Crystals per pixel
    nameCol = rec.indexOf("ProcTcrystalsPerPixel");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcrystalsPerPixel" << endl; }
    m_crystalsPerPixel = query.value(nameCol).toFloat();
    param.filmParams.crystalsPerPixel = m_crystalsPerPixel;

    //Initial crystal radius
    nameCol = rec.indexOf("ProcTinitialCrystalRadius");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialCrystalRadius" << endl; }
    m_initialCrystalRadius = query.value(nameCol).toFloat();
    param.filmParams.initialCrystalRadius = m_initialCrystalRadius;

    //Initial silver salt area density
    nameCol = rec.indexOf("ProcTinitialSilverSaltDensity");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialSilverSaltDensity" << endl; }
    m_initialSilverSaltDensity = query.value(nameCol).toFloat();
    param.filmParams.initialSilverSaltDensity = m_initialSilverSaltDensity;

    //Developer consumption rate constant
    nameCol = rec.indexOf("ProcTdeveloperConsumptionConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTdeveloperConsumptionConst" << endl; }
    m_developerConsumptionConst = query.value(nameCol).toFloat();
    param.filmParams.developerConsumptionConst = m_developerConsumptionConst;

    //Crystal growth rate constant
    nameCol = rec.indexOf("ProcTcrystalGrowthConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcrystalGrowthConst" << endl; }
    m_crystalGrowthConst = query.value(nameCol).toFloat();
    param.filmParams.crystalGrowthConst = m_crystalGrowthConst;

    //Silver halide consumption rate constant
    nameCol = rec.indexOf("ProcTsilverSaltConsumptionConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsilverSaltConsumptionConst" << endl; }
    m_silverSaltConsumptionConst = query.value(nameCol).toFloat();
    param.filmParams.silverSaltConsumptionConst = m_silverSaltConsumptionConst;

    //Total development time
    nameCol = rec.indexOf("ProcTtotalDevelopmentTime");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtotalDevelopmentTime" << endl; }
    m_totalDevelopmentTime = query.value(nameCol).toFloat();
    param.filmParams.totalDevelTime = m_totalDevelopmentTime;

    //Number of agitations
    nameCol = rec.indexOf("ProcTagitateCount");
    if (-1 == nameCol) { std::cout << "paramManager ProcTagitateCount" << endl; }
    m_agitateCount = query.value(nameCol).toInt();
    param.filmParams.agitateCount = m_agitateCount;

    //Number of simulation steps for development
    nameCol = rec.indexOf("ProcTdevelopmentSteps");
    if (-1 == nameCol) { std::cout << "paramManager ProcTdevelopmentSteps" << endl; }
    m_developmentSteps = query.value(nameCol).toInt();
    param.filmParams.developmentSteps = m_developmentSteps;

    //Area of film for the simulation
    nameCol = rec.indexOf("ProcTfilmArea");
    if (-1 == nameCol) { std::cout << "paramManager ProcTfilmArea" << endl; }
    m_filmArea = query.value(nameCol).toFloat();
    param.filmParams.filmArea = m_filmArea;

    //A constant for the size of the diffusion. It...affects the same thing as film area.
    nameCol = rec.indexOf("ProcTsigmaConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsigmaConst" << endl; }
    m_sigmaConst = query.value(nameCol).toFloat();
    param.filmParams.sigmaConst = m_sigmaConst;

    //Layer mix constant: the amount of active developer that gets exchanged with the reservoir.
    nameCol = rec.indexOf("ProcTlayerMixConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTlayerMixConst" << endl; }
    m_layerMixConst = query.value(nameCol).toFloat();
    param.filmParams.layerMixConst = m_layerMixConst;

    //Layer time divisor: Controls the relative intra-layer and inter-layer diffusion.
    nameCol = rec.indexOf("ProcTlayerTimeDivisor");
    if (-1 == nameCol) { std::cout << "paramManager ProcTlayerTimeDivisor" << endl; }
    m_layerTimeDivisor = query.value(nameCol).toFloat();
    param.filmParams.layerTimeDivisor = m_layerTimeDivisor;

    //Rolloff boundary. This is where highlights start to roll off.
    nameCol = rec.indexOf("ProcTrolloffBoundary");
    if (-1 == nameCol) { std::cout << "paramManager ProcTrolloffBoundary" << endl; }
    m_rolloffBoundary = query.value(nameCol).toInt();
    param.filmParams.rolloffBoundary = m_rolloffBoundary;

    //Post-filmulator black clipping point
    nameCol = rec.indexOf("ProcTblackpoint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTblackpoint" << endl; }
    m_blackpoint = query.value(nameCol).toFloat();
    param.blackpoint = m_blackpoint;

    //Post-filmulator white clipping point
    nameCol = rec.indexOf("ProcTwhitepoint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTwhitepoint" << endl; }
    m_whitepoint = query.value(nameCol).toFloat();
    param.whitepoint = m_whitepoint;

    //Shadow control point x value
    nameCol = rec.indexOf("ProcTshadowsX");
    if (-1 == nameCol) { std::cout << "paramManager ProcTshadowsX" << endl; }
    m_shadowsX = query.value(nameCol).toFloat();
    param.shadowsX = m_shadowsX;

    //Shadow control point y value
    nameCol = rec.indexOf("ProcTshadowsY");
    if (-1 == nameCol) { std::cout << "paramManager ProcTshadowsY" << endl; }
    m_shadowsY = query.value(nameCol).toFloat();
    param.shadowsY = m_shadowsY;

    //Highlight control point x value
    nameCol = rec.indexOf("ProcThighlightsX");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightsX" << endl; }
    m_highlightsX = query.value(nameCol).toFloat();
    param.highlightsX = m_highlightsX;

    //Highlight control point y value
    nameCol = rec.indexOf("ProcThighlightsY");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightsY" << endl; }
    m_highlightsY = query.value(nameCol).toFloat();
    param.highlightsY = m_highlightsY;

    //Vibrance (saturation of less-saturated things)
    nameCol = rec.indexOf("ProcTvibrance");
    if (-1 == nameCol) { std::cout << "paramManager ProcTvibrance" << endl; }
    m_vibrance = query.value(nameCol).toFloat();
    param.vibrance = m_vibrance;

    //Saturation
    nameCol = rec.indexOf("ProcTsaturation");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsaturation" << endl; }
    m_saturation = query.value(nameCol).toFloat();
    param.saturation = m_saturation;

    //Rotation
    nameCol = rec.indexOf("ProcTrotation");
    if (-1 == nameCol) { std::cout << "paramManager ProcTrotation" << endl; }
    m_rotation = query.value(nameCol).toInt();
    param.rotation = m_rotation;

    locker.unlock();
    enableParamChange();//Re-enable updating of the image.
    paramChangeWrapper(QString("selectImage"));
}

//This prevents the back-and-forth between this object and QML from aborting
// computation, and also prevents the sliders' moving from marking the photo
// as edited.
void ParameterManager::disableParamChange()
{
    paramChangeEnabled = false;
    qDebug() << "Disabling param changes: enabled? " << paramChangeEnabled;
}
void ParameterManager::enableParamChange()
{
    paramChangeEnabled = true;
}
void ParameterManager::paramChangeWrapper(QString source)
{
    qDebug() << "paramChangeWrapper: changes enabled? " << paramChangeEnabled;
    if (paramChangeEnabled)
    {
        emit paramChanged(source);
    }
    else
    {
        qDebug() << "Param change prevented";
    }
}
