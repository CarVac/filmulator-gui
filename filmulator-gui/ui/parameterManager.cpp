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
    QMutexLocker paramLocker(&paramMutex);
    return param;
}

void ParameterManager::setTiffIn(bool tiffIn)
{
    m_tiffIn = tiffIn;
    param.tiffIn = tiffIn;
    emit tiffInChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setTiff"));
}

void ParameterManager::setJpegIn(bool jpegIn)
{
    m_jpegIn = jpegIn;
    param.jpegIn = jpegIn;
    emit jpegInChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setJpeg"));
}

void ParameterManager::setCaEnabled(bool caEnabled)
{
    m_caEnabled = caEnabled;
    param.caEnabled = caEnabled;
    emit caEnabledChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setCaEnabled"));
}

void ParameterManager::setHighlights(int highlights)
{
    m_highlights = highlights;
    param.highlights = highlights;
    emit highlightsChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setHighlights"));
}

void ParameterManager::setExposureComp(float exposureComp)
{
    m_exposureComp = exposureComp;
    std::vector<float> exposureCompList;
    exposureCompList.push_back(exposureComp);
    param.exposureComp = exposureCompList;
    emit exposureCompChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setExposureComp"));
}

void ParameterManager::setTemperature(double temperature)
{
    m_temperature = temperature;
    param.temperature = temperature;
    emit temperatureChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setTemperature"));
}

void ParameterManager::setTint(double tint)
{
    m_tint = tint;
    param.tint = tint;
    emit tintChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setTint"));
}

void ParameterManager::setInitialDeveloperConcentration(float initialDeveloperConcentration)
{
    m_initialDeveloperConcentration = initialDeveloperConcentration;
    param.filmParams.initialDeveloperConcentration = initialDeveloperConcentration;
    emit initialDeveloperConcentrationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setInitialDeveloperConcentration"));
}

void ParameterManager::setReservoirThickness(float reservoirThickness)
{
    m_reservoirThickness = reservoirThickness;
    param.filmParams.reservoirThickness = reservoirThickness;
    emit reservoirThicknessChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setReservoirThickness"));
}

void ParameterManager::setActiveLayerThickness(float activeLayerThickness)
{
    m_activeLayerThickness = activeLayerThickness;
    param.filmParams.activeLayerThickness = activeLayerThickness;
    emit activeLayerThicknessChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setActiveLayerThickness"));
}

void ParameterManager::setCrystalsPerPixel(float crystalsPerPixel)
{
    m_crystalsPerPixel = crystalsPerPixel;
    param.filmParams.crystalsPerPixel = crystalsPerPixel;
    emit crystalsPerPixelChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setCrystalsPerPixel"));
}

void ParameterManager::setInitialCrystalRadius(float initialCrystalRadius)
{
    m_initialCrystalRadius = initialCrystalRadius;
    param.filmParams.initialCrystalRadius = initialCrystalRadius;
    emit initialCrystalRadiusChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setInitialCrystalRadius"));
}

void ParameterManager::setInitialSilverSaltDensity(float initialSilverSaltDensity)
{
    m_initialSilverSaltDensity = initialSilverSaltDensity;
    param.filmParams.initialSilverSaltDensity = initialSilverSaltDensity;
    emit initialSilverSaltDensityChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setInitialSilverSaltDensity"));
}

void ParameterManager::setDeveloperConsumptionConst(float developerConsumptionConst)
{
    m_developerConsumptionConst = developerConsumptionConst;
    param.filmParams.developerConsumptionConst = developerConsumptionConst;
    emit developerConsumptionConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setDeveloperConsumptionConst"));
}

void ParameterManager::setCrystalGrowthConst(float crystalGrowthConst)
{
    m_crystalGrowthConst = crystalGrowthConst;
    param.filmParams.crystalGrowthConst = crystalGrowthConst;
    emit crystalGrowthConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setCrystalGrowthConst"));
}

void ParameterManager::setSilverSaltConsumptionConst(float silverSaltConsumptionConst)
{
    m_silverSaltConsumptionConst = silverSaltConsumptionConst;
    param.filmParams.silverSaltConsumptionConst = silverSaltConsumptionConst;
    emit silverSaltConsumptionConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setSilverSaltConsumptionConst"));
}

void ParameterManager::setTotalDevelopmentTime(float totalDevelopmentTime)
{
    m_totalDevelopmentTime = totalDevelopmentTime;
    param.filmParams.totalDevelTime = totalDevelopmentTime;
    emit totalDevelopmentTimeChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setTotalDevelopmentTime"));
}

void ParameterManager::setAgitateCount(int agitateCount)
{
    m_agitateCount = agitateCount;
    param.filmParams.agitateCount = agitateCount;
    emit agitateCountChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setAgitateCount"));
}

void ParameterManager::setDevelopmentSteps(int developmentSteps)
{
    m_developmentSteps = developmentSteps;
    param.filmParams.developmentSteps = developmentSteps;
    emit developmentStepsChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setDevelopmentSteps"));
}

void ParameterManager::setFilmArea(float filmArea)
{
    m_filmArea = filmArea;
    param.filmParams.filmArea = filmArea;
    emit filmAreaChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setFilmArea"));
}

void ParameterManager::setSigmaConst(float sigmaConst)
{
    m_sigmaConst = sigmaConst;
    param.filmParams.sigmaConst = sigmaConst;
    emit sigmaConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setSigmaConst"));
}

void ParameterManager::setLayerMixConst(float layerMixConst)
{
    m_layerMixConst = layerMixConst;
    param.filmParams.layerMixConst = layerMixConst;
    emit layerMixConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setLayerMixConst"));
}

void ParameterManager::setLayerTimeDivisor(float layerTimeDivisor)
{
    m_layerTimeDivisor = layerTimeDivisor;
    param.filmParams.layerTimeDivisor = layerTimeDivisor;
    emit layerTimeDivisorChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setLayerTimeDivisor"));
}

void ParameterManager::setRolloffBoundary(int rolloffBoundary)
{
    m_rolloffBoundary = rolloffBoundary;
    param.filmParams.rolloffBoundary = rolloffBoundary;
    emit rolloffBoundaryChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setRolloffBoundary"));
}

void ParameterManager::setBlackpoint(float blackpoint)
{
    m_blackpoint = blackpoint;
    param.blackpoint = blackpoint;
    emit blackpointChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setBlackpoint"));
}

void ParameterManager::setWhitepoint(float whitepoint)
{
    m_whitepoint = whitepoint;
    param.whitepoint = whitepoint;
    emit whitepointChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setWhitepoint"));
}

void ParameterManager::setShadowsX(float shadowsX)
{
    m_shadowsX = shadowsX;
    param.shadowsX = shadowsX;
    emit shadowsXChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setShadowsX"));
}

void ParameterManager::setShadowsY(float shadowsY)
{
    m_shadowsY = shadowsY;
    param.shadowsY = shadowsY;
    emit shadowsYChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setShadowsY"));
}

void ParameterManager::setHighlightsX(float highlightsX)
{
    m_highlightsX = highlightsX;
    param.highlightsX = highlightsX;
    emit highlightsXChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setHighlightsX"));
}

void ParameterManager::setHighlightsY(float highlightsY)
{
    m_highlightsY = highlightsY;
    param.highlightsY = highlightsY;
    emit highlightsYChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setHighlightsY"));
}

void ParameterManager::setVibrance(float vibrance)
{
    m_vibrance = vibrance;
    param.vibrance = vibrance;
    emit vibranceChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setVibrance"));
}

void ParameterManager::setSaturation(float saturation)
{
    m_saturation = saturation;
    param.saturation = saturation;
    emit saturationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setSaturation"));
}

void ParameterManager::setRotation(int rotation)
{
    m_rotation = rotation;
    param.rotation = rotation;
    emit rotationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setRotation"));
}

void ParameterManager::rotateRight()
{
    m_rotation--;
    if (m_rotation < 0)
    {
        m_rotation += 4;
    }
    param.rotation = m_rotation;
    emit rotationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("rotateRight"));
    writeback();//Normally the slider has to call this when released, but this isn't a slider.
}

void ParameterManager::rotateLeft()
{
    m_rotation++;
    if (m_rotation > 3)
    {
        m_rotation -= 4;
    }
    param.rotation = m_rotation;
    emit rotationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("rotateLeft"));
    writeback();//Normally the slider has to call this when released, but this isn't a slider.
}

//This gets called by a slider when it is released.
//It syncs the setting with the database.
void ParameterManager::writeback()
{
    //It prevents writing to the DB while loading an image, however unlikely.
    if (paramChangeEnabled)
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
        query.bindValue(31, imageIndex);
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
        query.exec("COMMIT;");//Apply all the changes together.
        emit updateTable("ProcessingTable", 0);//0 means edit
        emit updateTable("SearchTable", 0);//0 means edit
        emit updateTable("QueueTable", 0);//0 means edit
    }
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
    std::vector<string> tempFilename;
    tempFilename.push_back(name.toStdString());
    param.filenameList = tempFilename;
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
    query.first();
    rec = query.record();

    //First is caEnabled.
    nameCol = rec.indexOf("ProcTcaEnabled");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcaEnabled" << endl; }
    setCaEnabled(query.value(nameCol).toBool());

    //Next is highlights (highlight recovery)
    nameCol = rec.indexOf("ProcThighlightRecovery");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightRecovery" << endl; }
    setHighlights(query.value(nameCol).toInt());

    //Exposure compensation
    nameCol = rec.indexOf("ProcTexposureComp");
    if (-1 == nameCol) { std::cout << "paramManager ProcTexposureComp" << endl; }
    setExposureComp(query.value(nameCol).toFloat());

    //Temperature
    nameCol = rec.indexOf("ProcTtemperature");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtemperature" << endl; }
    setTemperature(query.value(nameCol).toDouble());

    //Tint
    nameCol = rec.indexOf("ProcTtint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtint" << endl; }
    setTint(query.value(nameCol).toDouble());

    //Initial developer concentration
    nameCol = rec.indexOf("ProcTinitialDeveloperConcentration");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialDeveloperConcentration" << endl; }
    setInitialDeveloperConcentration(query.value(nameCol).toFloat());

    //Reservoir thickness
    nameCol = rec.indexOf("ProcTreservoirThickness");
    if (-1 == nameCol) { std::cout << "paramManager ProcTreservoirThickness" << endl; }
    setReservoirThickness(query.value(nameCol).toFloat());

    //Active layer thickness
    nameCol = rec.indexOf("ProcTactiveLayerThickness");
    if (-1 == nameCol) { std::cout << "paramManager ProcTactiveLayerThickness" << endl; }
    setActiveLayerThickness(query.value(nameCol).toFloat());

    //Crystals per pixel
    nameCol = rec.indexOf("ProcTcrystalsPerPixel");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcrystalsPerPixel" << endl; }
    setCrystalsPerPixel(query.value(nameCol).toFloat());

    //Initial crystal radius
    nameCol = rec.indexOf("ProcTinitialCrystalRadius");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialCrystalRadius" << endl; }
    setInitialCrystalRadius(query.value(nameCol).toFloat());

    //Initial silver salt area density
    nameCol = rec.indexOf("ProcTinitialSilverSaltDensity");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialSilverSaltDensity" << endl; }
    setInitialSilverSaltDensity(query.value(nameCol).toFloat());

    //Developer consumption rate constant
    nameCol = rec.indexOf("ProcTdeveloperConsumptionConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTdeveloperConsumptionConst" << endl; }
    setDeveloperConsumptionConst(query.value(nameCol).toFloat());

    //Crystal growth rate constant
    nameCol = rec.indexOf("ProcTcrystalGrowthConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcrystalGrowthConst" << endl; }
    setCrystalGrowthConst(query.value(nameCol).toFloat());

    //Silver halide consumption rate constant
    nameCol = rec.indexOf("ProcTsilverSaltConsumptionConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsilverSaltConsumptionConst" << endl; }
    setSilverSaltConsumptionConst(query.value(nameCol).toFloat());

    //Total development time
    nameCol = rec.indexOf("ProcTtotalDevelopmentTime");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtotalDevelopmentTime" << endl; }
    setTotalDevelopmentTime(query.value(nameCol).toFloat());

    //Number of agitations
    nameCol = rec.indexOf("ProcTagitateCount");
    if (-1 == nameCol) { std::cout << "paramManager ProcTagitateCount" << endl; }
    setAgitateCount(query.value(nameCol).toInt());

    //Number of simulation steps for development
    nameCol = rec.indexOf("ProcTdevelopmentSteps");
    if (-1 == nameCol) { std::cout << "paramManager ProcTdevelopmentSteps" << endl; }
    setDevelopmentSteps(query.value(nameCol).toInt());

    //Area of film for the simulation
    nameCol = rec.indexOf("ProcTfilmArea");
    if (-1 == nameCol) { std::cout << "paramManager ProcTfilmArea" << endl; }
    setFilmArea(query.value(nameCol).toFloat());

    //A constant for the size of the diffusion. It...affects the same thing as film area.
    nameCol = rec.indexOf("ProcTsigmaConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsigmaConst" << endl; }
    setSigmaConst(query.value(nameCol).toFloat());

    //Layer mix constant: the amount of active developer that gets exchanged with the reservoir.
    nameCol = rec.indexOf("ProcTlayerMixConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTlayerMixConst" << endl; }
    setLayerMixConst(query.value(nameCol).toFloat());

    //Layer time divisor: Controls the relative intra-layer and inter-layer diffusion.
    nameCol = rec.indexOf("ProcTlayerTimeDivisor");
    if (-1 == nameCol) { std::cout << "paramManager ProcTlayerTimeDivisor" << endl; }
    setLayerTimeDivisor(query.value(nameCol).toFloat());

    //Rolloff boundary. This is where highlights start to roll off.
    nameCol = rec.indexOf("ProcTrolloffBoundary");
    if (-1 == nameCol) { std::cout << "paramManager ProcTrolloffBoundary" << endl; }
    setRolloffBoundary(query.value(nameCol).toInt());

    //Post-filmulator black clipping point
    nameCol = rec.indexOf("ProcTblackpoint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTblackpoint" << endl; }
    setBlackpoint(query.value(nameCol).toFloat());

    //Post-filmulator white clipping point
    nameCol = rec.indexOf("ProcTwhitepoint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTwhitepoint" << endl; }
    setWhitepoint(query.value(nameCol).toFloat());

    //Shadow control point x value
    nameCol = rec.indexOf("ProcTshadowsX");
    if (-1 == nameCol) { std::cout << "paramManager ProcTshadowsX" << endl; }
    setShadowsX(query.value(nameCol).toFloat());

    //Shadow control point y value
    nameCol = rec.indexOf("ProcTshadowsY");
    if (-1 == nameCol) { std::cout << "paramManager ProcTshadowsY" << endl; }
    setShadowsY(query.value(nameCol).toFloat());

    //Highlight control point x value
    nameCol = rec.indexOf("ProcThighlightsX");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightsX" << endl; }
    setHighlightsX(query.value(nameCol).toFloat());

    //Highlight control point y value
    nameCol = rec.indexOf("ProcThighlightsY");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightsY" << endl; }
    setHighlightsY(query.value(nameCol).toFloat());

    //Vibrance (saturation of less-saturated things)
    nameCol = rec.indexOf("ProcTvibrance");
    if (-1 == nameCol) { std::cout << "paramManager ProcTvibrance" << endl; }
    setVibrance(query.value(nameCol).toFloat());

    //Saturation
    nameCol = rec.indexOf("ProcTsaturation");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsaturation" << endl; }
    setSaturation(query.value(nameCol).toFloat());

    //Rotation
    nameCol = rec.indexOf("ProcTrotation");
    if (-1 == nameCol) { std::cout << "paramManager ProcTrotation" << endl; }
    setRotation(query.value(nameCol).toInt());

    paramLocker.unlock();
    QMutexLocker signalLocker(&signalMutex);
    enableParamChange();//Re-enable updating of the image.
    paramChangeWrapper(QString("selectImage"));
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
}
