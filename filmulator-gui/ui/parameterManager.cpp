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

    pasteable = false;
    pasteSome = false;
}

ProcessingParameters ParameterManager::getParams()
{
    QMutexLocker paramLocker(&paramMutex);
    return param;
}

void ParameterManager::setTiffIn(bool tiffIn)
{
    param.tiffIn = tiffIn;
    emit tiffInChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setTiff"));
}

void ParameterManager::setJpegIn(bool jpegIn)
{
    param.jpegIn = jpegIn;
    emit jpegInChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setJpeg"));
}

void ParameterManager::setCaEnabled(bool caEnabled)
{
    param.caEnabled = caEnabled;
    emit caEnabledChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setCaEnabled"));
}

void ParameterManager::setHighlights(int highlights)
{
    param.highlights = highlights;
    emit highlightsChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setHighlights"));
}

void ParameterManager::setExposureComp(float exposureComp)
{
    //The struct uses a vector, so we can't directly access it so neatly, I think.
    m_exposureComp = exposureComp;
    std::vector<float> exposureCompList;
    exposureCompList.push_back(exposureComp);
    param.exposureComp = exposureCompList;
    emit exposureCompChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setExposureComp"));
}

void ParameterManager::setTemperature(float temperature)
{
    param.temperature = temperature;
    emit temperatureChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setTemperature"));
}

void ParameterManager::setTint(float tint)
{
    param.tint = tint;
    emit tintChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setTint"));
}

void ParameterManager::setInitialDeveloperConcentration(float initialDeveloperConcentration)
{
    param.filmParams.initialDeveloperConcentration = initialDeveloperConcentration;
    emit initialDeveloperConcentrationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setInitialDeveloperConcentration"));
}

void ParameterManager::setReservoirThickness(float reservoirThickness)
{
    param.filmParams.reservoirThickness = reservoirThickness;
    emit reservoirThicknessChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setReservoirThickness"));
}

void ParameterManager::setActiveLayerThickness(float activeLayerThickness)
{
    param.filmParams.activeLayerThickness = activeLayerThickness;
    emit activeLayerThicknessChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setActiveLayerThickness"));
}

void ParameterManager::setCrystalsPerPixel(float crystalsPerPixel)
{
    param.filmParams.crystalsPerPixel = crystalsPerPixel;
    emit crystalsPerPixelChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setCrystalsPerPixel"));
}

void ParameterManager::setInitialCrystalRadius(float initialCrystalRadius)
{
    param.filmParams.initialCrystalRadius = initialCrystalRadius;
    emit initialCrystalRadiusChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setInitialCrystalRadius"));
}

void ParameterManager::setInitialSilverSaltDensity(float initialSilverSaltDensity)
{
    param.filmParams.initialSilverSaltDensity = initialSilverSaltDensity;
    emit initialSilverSaltDensityChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setInitialSilverSaltDensity"));
}

void ParameterManager::setDeveloperConsumptionConst(float developerConsumptionConst)
{
    param.filmParams.developerConsumptionConst = developerConsumptionConst;
    emit developerConsumptionConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setDeveloperConsumptionConst"));
}

void ParameterManager::setCrystalGrowthConst(float crystalGrowthConst)
{
    param.filmParams.crystalGrowthConst = crystalGrowthConst;
    emit crystalGrowthConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setCrystalGrowthConst"));
}

void ParameterManager::setSilverSaltConsumptionConst(float silverSaltConsumptionConst)
{
    param.filmParams.silverSaltConsumptionConst = silverSaltConsumptionConst;
    emit silverSaltConsumptionConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setSilverSaltConsumptionConst"));
}

void ParameterManager::setTotalDevelopmentTime(float totalDevelopmentTime)
{
    param.filmParams.totalDevelTime = totalDevelopmentTime;
    emit totalDevelopmentTimeChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setTotalDevelopmentTime"));
}

void ParameterManager::setAgitateCount(int agitateCount)
{
    param.filmParams.agitateCount = agitateCount;
    emit agitateCountChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setAgitateCount"));
}

void ParameterManager::setDevelopmentSteps(int developmentSteps)
{
    param.filmParams.developmentSteps = developmentSteps;
    emit developmentStepsChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setDevelopmentSteps"));
}

void ParameterManager::setFilmArea(float filmArea)
{
    param.filmParams.filmArea = filmArea;
    emit filmAreaChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setFilmArea"));
}

void ParameterManager::setSigmaConst(float sigmaConst)
{
    param.filmParams.sigmaConst = sigmaConst;
    emit sigmaConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setSigmaConst"));
}

void ParameterManager::setLayerMixConst(float layerMixConst)
{
    param.filmParams.layerMixConst = layerMixConst;
    emit layerMixConstChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setLayerMixConst"));
}

void ParameterManager::setLayerTimeDivisor(float layerTimeDivisor)
{
    param.filmParams.layerTimeDivisor = layerTimeDivisor;
    emit layerTimeDivisorChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setLayerTimeDivisor"));
}

void ParameterManager::setRolloffBoundary(int rolloffBoundary)
{
    param.filmParams.rolloffBoundary = rolloffBoundary;
    emit rolloffBoundaryChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setRolloffBoundary"));
}

void ParameterManager::setBlackpoint(float blackpoint)
{
    param.blackpoint = blackpoint;
    emit blackpointChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setBlackpoint"));
}

void ParameterManager::setWhitepoint(float whitepoint)
{
    param.whitepoint = whitepoint;
    emit whitepointChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setWhitepoint"));
}

void ParameterManager::setShadowsX(float shadowsX)
{
    param.shadowsX = shadowsX;
    emit shadowsXChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setShadowsX"));
}

void ParameterManager::setShadowsY(float shadowsY)
{
    param.shadowsY = shadowsY;
    emit shadowsYChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setShadowsY"));
}

void ParameterManager::setHighlightsX(float highlightsX)
{
    param.highlightsX = highlightsX;
    emit highlightsXChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setHighlightsX"));
}

void ParameterManager::setHighlightsY(float highlightsY)
{
    param.highlightsY = highlightsY;
    emit highlightsYChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setHighlightsY"));
}

void ParameterManager::setVibrance(float vibrance)
{
    param.vibrance = vibrance;
    emit vibranceChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setVibrance"));
}

void ParameterManager::setSaturation(float saturation)
{
    param.saturation = saturation;
    emit saturationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setSaturation"));
}

void ParameterManager::setRotation(int rotation)
{
    param.rotation = rotation;
    emit rotationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("setRotation"));
}

void ParameterManager::rotateRight()
{
    int rotation = param.rotation - 1;
    if (rotation < 0)
    {
        rotation += 4;
    }
    param.rotation = rotation;
    emit rotationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("rotateRight"));
    writeback();//Normally the slider has to call this when released, but this isn't a slider.
}

void ParameterManager::rotateLeft()
{
    int rotation = param.rotation + 1;
    if (rotation > 3)
    {
        rotation -= 4;
    }
    param.rotation = rotation;
    emit rotationChanged();
    QMutexLocker signalLocker(&signalMutex);
    paramChangeWrapper(QString("rotateLeft"));
    writeback();//Normally the slider has to call this when released, but this isn't a slider.
}

//This gets called by a slider (from qml) when it is released.
//It syncs the main (slider-interfaced) settings with the database.
void ParameterManager::writeback()
{
    //Writeback gets called by setters, which themselves are called by
    if (paramChangeEnabled)
    {
        writeToDB(param, imageIndex);
    }
}

//This syncs the passed-in parameters to the database.
void ParameterManager::writeToDB(ProcessingParameters params, QString imageID)
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
    query.bindValue( 0, params.filmParams.initialDeveloperConcentration);
    query.bindValue( 1, params.filmParams.reservoirThickness);
    query.bindValue( 2, params.filmParams.activeLayerThickness);
    query.bindValue( 3, params.filmParams.crystalsPerPixel);
    query.bindValue( 4, params.filmParams.initialCrystalRadius);
    query.bindValue( 5, params.filmParams.initialSilverSaltDensity);
    query.bindValue( 6, params.filmParams.developerConsumptionConst);
    query.bindValue( 7, params.filmParams.crystalGrowthConst);
    query.bindValue( 8, params.filmParams.silverSaltConsumptionConst);
    query.bindValue( 9, params.filmParams.totalDevelTime);
    query.bindValue(10, params.filmParams.agitateCount);
    query.bindValue(11, params.filmParams.developmentSteps);
    query.bindValue(12, params.filmParams.filmArea);
    query.bindValue(13, params.filmParams.sigmaConst);
    query.bindValue(14, params.filmParams.layerMixConst);
    query.bindValue(15, params.filmParams.layerTimeDivisor);
    query.bindValue(16, params.filmParams.rolloffBoundary);
    query.bindValue(17, params.exposureComp[0]);
    query.bindValue(18, params.whitepoint);
    query.bindValue(19, params.blackpoint);
    query.bindValue(20, params.shadowsX);
    query.bindValue(21, params.shadowsY);
    query.bindValue(22, params.highlightsX);
    query.bindValue(23, params.highlightsY);
    query.bindValue(24, params.highlights);
    query.bindValue(25, params.caEnabled);
    query.bindValue(26, params.temperature);
    query.bindValue(27, params.tint);
    query.bindValue(28, params.vibrance);
    query.bindValue(29, params.saturation);
    query.bindValue(30, params.rotation);
    query.bindValue(31, imageID);
    query.exec();
    //Write that it's been edited to the SearchTable (actually writing the edit time)
    QDateTime now = QDateTime::currentDateTime();
    query.prepare("UPDATE SearchTable SET STlastProcessedTime = ? WHERE STSearchID = ?;");
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
    std::vector<string> tempFilename;
    tempFilename.push_back(name.toStdString());
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

    //Copy all of the processing parameters from the db into this object.
    param = loadParams(imageID);

    //The filename was retrieved separately earlier, so now we need to set the param.
    param.filenameList = tempFilename;

    //Emit that the things have changed.
    emit caEnabledChanged();
    emit highlightsChanged();
    m_exposureComp = param.exposureComp[0];//this is an odd one out
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

//This loads the processing params from imageID into a struct.
ProcessingParameters ParameterManager::loadParams(QString imageID)
{
    ProcessingParameters outParam;
    QSqlRecord rec;
    int nameCol;
    QSqlQuery query;

    //tiffIn should be false.
    outParam.tiffIn = false;

    //So should jpegIn.
    outParam.jpegIn = false;


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
    outParam.caEnabled = query.value(nameCol).toBool();

    //Next is highlights (highlight recovery)
    nameCol = rec.indexOf("ProcThighlightRecovery");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightRecovery" << endl; }
    outParam.highlights = query.value(nameCol).toInt();

    //Exposure compensation
    nameCol = rec.indexOf("ProcTexposureComp");
    if (-1 == nameCol) { std::cout << "paramManager ProcTexposureComp" << endl; }
    std::vector<float> exposureCompList;
    exposureCompList.push_back(query.value(nameCol).toFloat());
    outParam.exposureComp = exposureCompList;

    //Temperature
    nameCol = rec.indexOf("ProcTtemperature");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtemperature" << endl; }
    outParam.temperature = query.value(nameCol).toFloat();

    //Tint
    nameCol = rec.indexOf("ProcTtint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtint" << endl; }
    outParam.tint = query.value(nameCol).toFloat();

    //Initial developer concentration
    nameCol = rec.indexOf("ProcTinitialDeveloperConcentration");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialDeveloperConcentration" << endl; }
    outParam.filmParams.initialDeveloperConcentration = query.value(nameCol).toFloat();

    //Reservoir thickness
    nameCol = rec.indexOf("ProcTreservoirThickness");
    if (-1 == nameCol) { std::cout << "paramManager ProcTreservoirThickness" << endl; }
    outParam.filmParams.reservoirThickness = query.value(nameCol).toFloat();

    //Active layer thickness
    nameCol = rec.indexOf("ProcTactiveLayerThickness");
    if (-1 == nameCol) { std::cout << "paramManager ProcTactiveLayerThickness" << endl; }
    outParam.filmParams.activeLayerThickness = query.value(nameCol).toFloat();

    //Crystals per pixel
    nameCol = rec.indexOf("ProcTcrystalsPerPixel");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcrystalsPerPixel" << endl; }
    outParam.filmParams.crystalsPerPixel = query.value(nameCol).toFloat();

    //Initial crystal radius
    nameCol = rec.indexOf("ProcTinitialCrystalRadius");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialCrystalRadius" << endl; }
    outParam.filmParams.initialCrystalRadius = query.value(nameCol).toFloat();

    //Initial silver salt area density
    nameCol = rec.indexOf("ProcTinitialSilverSaltDensity");
    if (-1 == nameCol) { std::cout << "paramManager ProcTinitialSilverSaltDensity" << endl; }
    outParam.filmParams.initialSilverSaltDensity = query.value(nameCol).toFloat();

    //Developer consumption rate constant
    nameCol = rec.indexOf("ProcTdeveloperConsumptionConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTdeveloperConsumptionConst" << endl; }
    outParam.filmParams.developerConsumptionConst = query.value(nameCol).toFloat();

    //Crystal growth rate constant
    nameCol = rec.indexOf("ProcTcrystalGrowthConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTcrystalGrowthConst" << endl; }
    outParam.filmParams.crystalGrowthConst = query.value(nameCol).toFloat();

    //Silver halide consumption rate constant
    nameCol = rec.indexOf("ProcTsilverSaltConsumptionConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsilverSaltConsumptionConst" << endl; }
    outParam.filmParams.silverSaltConsumptionConst = query.value(nameCol).toFloat();

    //Total development time
    nameCol = rec.indexOf("ProcTtotalDevelopmentTime");
    if (-1 == nameCol) { std::cout << "paramManager ProcTtotalDevelopmentTime" << endl; }
    outParam.filmParams.totalDevelTime = query.value(nameCol).toFloat();

    //Number of agitations
    nameCol = rec.indexOf("ProcTagitateCount");
    if (-1 == nameCol) { std::cout << "paramManager ProcTagitateCount" << endl; }
    outParam.filmParams.agitateCount = query.value(nameCol).toInt();

    //Number of simulation steps for development
    nameCol = rec.indexOf("ProcTdevelopmentSteps");
    if (-1 == nameCol) { std::cout << "paramManager ProcTdevelopmentSteps" << endl; }
    outParam.filmParams.developmentSteps = query.value(nameCol).toInt();

    //Area of film for the simulation
    nameCol = rec.indexOf("ProcTfilmArea");
    if (-1 == nameCol) { std::cout << "paramManager ProcTfilmArea" << endl; }
    outParam.filmParams.filmArea = query.value(nameCol).toFloat();

    //A constant for the size of the diffusion. It...affects the same thing as film area.
    nameCol = rec.indexOf("ProcTsigmaConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsigmaConst" << endl; }
    outParam.filmParams.sigmaConst = query.value(nameCol).toFloat();

    //Layer mix constant: the amount of active developer that gets exchanged with the reservoir.
    nameCol = rec.indexOf("ProcTlayerMixConst");
    if (-1 == nameCol) { std::cout << "paramManager ProcTlayerMixConst" << endl; }
    outParam.filmParams.layerMixConst = query.value(nameCol).toFloat();

    //Layer time divisor: Controls the relative intra-layer and inter-layer diffusion.
    nameCol = rec.indexOf("ProcTlayerTimeDivisor");
    if (-1 == nameCol) { std::cout << "paramManager ProcTlayerTimeDivisor" << endl; }
    outParam.filmParams.layerTimeDivisor = query.value(nameCol).toFloat();

    //Rolloff boundary. This is where highlights start to roll off.
    nameCol = rec.indexOf("ProcTrolloffBoundary");
    if (-1 == nameCol) { std::cout << "paramManager ProcTrolloffBoundary" << endl; }
    outParam.filmParams.rolloffBoundary = query.value(nameCol).toInt();

    //Post-filmulator black clipping point
    nameCol = rec.indexOf("ProcTblackpoint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTblackpoint" << endl; }
    outParam.blackpoint = query.value(nameCol).toFloat();

    //Post-filmulator white clipping point
    nameCol = rec.indexOf("ProcTwhitepoint");
    if (-1 == nameCol) { std::cout << "paramManager ProcTwhitepoint" << endl; }
    outParam.whitepoint = query.value(nameCol).toFloat();

    //Shadow control point x value
    nameCol = rec.indexOf("ProcTshadowsX");
    if (-1 == nameCol) { std::cout << "paramManager ProcTshadowsX" << endl; }
    outParam.shadowsX = query.value(nameCol).toFloat();

    //Shadow control point y value
    nameCol = rec.indexOf("ProcTshadowsY");
    if (-1 == nameCol) { std::cout << "paramManager ProcTshadowsY" << endl; }
    outParam.shadowsY = query.value(nameCol).toFloat();

    //Highlight control point x value
    nameCol = rec.indexOf("ProcThighlightsX");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightsX" << endl; }
    outParam.highlightsX = query.value(nameCol).toFloat();

    //Highlight control point y value
    nameCol = rec.indexOf("ProcThighlightsY");
    if (-1 == nameCol) { std::cout << "paramManager ProcThighlightsY" << endl; }
    outParam.highlightsY = query.value(nameCol).toFloat();

    //Vibrance (saturation of less-saturated things)
    nameCol = rec.indexOf("ProcTvibrance");
    if (-1 == nameCol) { std::cout << "paramManager ProcTvibrance" << endl; }
    outParam.vibrance = query.value(nameCol).toFloat();

    //Saturation
    nameCol = rec.indexOf("ProcTsaturation");
    if (-1 == nameCol) { std::cout << "paramManager ProcTsaturation" << endl; }
    outParam.saturation = query.value(nameCol).toFloat();

    //Rotation
    nameCol = rec.indexOf("ProcTrotation");
    if (-1 == nameCol) { std::cout << "paramManager ProcTrotation" << endl; }
    outParam.rotation = query.value(nameCol).toInt();

    return outParam;
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
            QMutexLocker paramLocker(&paramMutex);
            writeToDB(loadParams(copyFromImageIndex), toImageID);
        }
    }
}
