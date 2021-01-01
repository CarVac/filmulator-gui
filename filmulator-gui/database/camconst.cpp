#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include <curl/curl.h>

#include "cJSON.h"

#include <QString>
#include <QDir>
#include <QStandardPaths>

#include "camconst.h"

QString camconst_dir()
{
    std::string filename = "camconst.json";
    QString dir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    dir.append("/filmulator/");
    QDir directory(dir);
    directory.mkpath(dir);
    dir.append(filename.c_str());
    return dir;
}

camconst_status camconst_download()
{
    camconst_status result = CAMCONST_DL_OK;
    CURL *curl;
    CURLcode res;
    std::string url = "https://raw.githubusercontent.com/Beep6581/RawTherapee/dev/rtengine/camconst.json";

    QString dir = camconst_dir();
    std::string dirstr = dir.toStdString();

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        //try using native windows ca store
#ifdef __WIN32__
        curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
#endif

        FILE *f = fopen(dirstr.c_str(), "wb");
        if (f)
        {
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                std::cout << "camconst download cURL error: " << res << std::endl;
                result = CAMCONST_DL_RETRIEVEFAILED;
            }
            fclose(f);
        } else {
            result = CAMCONST_DL_FOPENFAILED;
        }
        curl_easy_cleanup(curl);
    } else {
        result = CAMCONST_DL_INITFAILED;
    }
    curl_global_cleanup();

    return result;
}

camconst_status camconst_read(const QString inputMakeModel, const float iso, const float fnumber, double &whiteLevel)
{
    QString filePath = camconst_dir();
    QFile file(filePath);
    if (!file.exists())
    {
        return CAMCONST_READ_FAILED;
    }
    camconst_status status = CAMCONST_READ_OK;

    FILE *f = NULL;
    long len = 0;
    char *data = NULL;
    size_t result;

    std::string filename = filePath.toStdString();

    f = fopen(filename.c_str(), "rb");
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    data = (char *) malloc(len+1);
    result = fread(data, 1, len, f);
    data[len] = '\0';
    fclose(f);

    cJSON_Minify(data);
    cJSON * json = cJSON_Parse(data);
    free(data);
    if (!json)
    {
        return CAMCONST_READ_FAILED;
    }

    QString makeModel;
    float apertureScaleFactor = 0;
    whiteLevel = 0;

    //Only thing we care about for now is white level based on various things
    //also black level (check panasonic g9!!!) maybe?

    //camera
    //  make model (sometimes one, sometimes an [array])
    //  dcraw_matrix IGNORE
    //  ranges
    //    black (only one value); this is an offset added to the base given by libraw, apparently?
    //    white (sometimes one value)
    //    white (sometimes an [array])
    //      iso
    //      iso (sometimes an [array])
    //      levels
    //      levels (sometimes an [array]; use the lowest)
    //    white_max
    //    aperture_scaling [array]
    //      aperture
    //      scale_factor


    cJSON * camlist = cJSON_GetObjectItemCaseSensitive(json, "camera_constants");
    if (camlist)
    {
        cJSON * camera;
        cJSON_ArrayForEach(camera,camlist)
        {
            makeModel = "";
            cJSON * makeModelList = cJSON_GetObjectItemCaseSensitive(camera, "make_model");
            if (cJSON_IsArray(makeModelList))
            {
                cJSON * makeModelItem;
                cJSON_ArrayForEach(makeModelItem, makeModelList)
                {
                    const QString tempMakeModel = QString(makeModelItem->valuestring);
                    if (tempMakeModel == inputMakeModel)
                    {
                        makeModel = tempMakeModel;
                    }
                }
                //makeModel = QString(cJSON_GetArrayItem(makeModelList, 0)->valuestring);
                //TODO: make list of camera name synonyms, keep track of canonical name in a table
            } else {
                const QString tempMakeModel = QString(makeModelList->valuestring);
                if (tempMakeModel == inputMakeModel)
                {
                    makeModel = tempMakeModel;
                }
            }
            if (makeModel == "")
            {
                continue;
            }
            std::cout << "CamConst make and model: " << makeModel.toStdString() << std::endl;

            cJSON * item;
            cJSON_ArrayForEach(item, camera)
            {
                std::cout << "CamConst item string: " << item->string << std::endl;
                if (QString(item->string) == "make_model")
                {
                    continue;
                }
                if (QString(item->string) != "ranges")
                {
                    continue; //for now we skip anything not white point
                    //there's:
                    //  dcraw_matrix
                    //  raw_crop
                    //  masked_areas
                    //  pdaf_pattern
                    //  pdaf_offset

                }
                if (QString(item->string) == "ranges")
                {
                    cJSON * rangeItem;
                    cJSON_ArrayForEach(rangeItem, item)
                    {
                        std::cout << "CamConst range item string: " << rangeItem->string << std::endl;
                        if (QString(rangeItem->string) == "white")
                        {
                            if (cJSON_IsNumber(rangeItem))
                            {
                                std::cout << "CamConst whitepoint level only: " << rangeItem->valuedouble << std::endl;
                                whiteLevel = rangeItem->valuedouble;
                            }
                            if (cJSON_IsArray(rangeItem))
                            {
                                std::cout << "CamConst whitepoint is array" << std::endl;
                                cJSON * whiteItem;
                                bool correctIso = false;
                                cJSON_ArrayForEach(whiteItem, rangeItem)
                                {
                                    correctIso = false;
                                    //We have to get the objectitems
                                    cJSON * isoItem = cJSON_GetObjectItemCaseSensitive(whiteItem, "iso");
                                    if (cJSON_IsNumber(isoItem))
                                    {
                                        std::cout << "CamConst whitepoint iso: " << isoItem->valuedouble << std::endl;
                                        if (abs(isoItem->valuedouble - iso) < 3)
                                        {
                                            correctIso = true;
                                        }
                                    }
                                    if (cJSON_IsArray(isoItem))
                                    {
                                        cJSON * isoSubItem;
                                        std::cout << "CamConst whitepoint isos: ";
                                        cJSON_ArrayForEach(isoSubItem, isoItem)
                                        {
                                            std::cout << isoSubItem->valuedouble << " ";
                                           if (abs(isoSubItem->valuedouble - iso) < 3)
                                           {
                                               correctIso = true;
                                           }
                                        }
                                        std::cout << std::endl;
                                    }

                                    if (correctIso)
                                    {
                                        cJSON * levelsItem = cJSON_GetObjectItemCaseSensitive(whiteItem, "levels");
                                        if (cJSON_IsNumber(levelsItem))
                                        {
                                            std::cout << "CamConst whitepoint level: " << levelsItem->valuedouble << std::endl;
                                            whiteLevel = levelsItem->valuedouble;
                                        }
                                        if (cJSON_IsArray(levelsItem))
                                        {
                                            cJSON * levelsSubItem;
                                            double tempWhiteLevel = 1e10;
                                            cJSON_ArrayForEach(levelsSubItem, levelsItem)
                                            {
                                                tempWhiteLevel = std::min(tempWhiteLevel, levelsSubItem->valuedouble);
                                            }
                                            std::cout << "CamConst whitepoint lowest level: " << tempWhiteLevel << std::endl;
                                            whiteLevel = tempWhiteLevel;
                                        }
                                    }
                                }
                            }
                        }
                        if (QString(rangeItem->string) == "white_max")
                        {
                            std::cout << "CamConst white max: " << rangeItem->valuedouble << std::endl;
                        }
                        if ((QString(rangeItem->string) == "aperture_scaling") && fnumber > 0.7)//we don't want to aperture scale manual lenses
                        {
                            cJSON * apertureListItem;
                            cJSON_ArrayForEach(apertureListItem, rangeItem)
                            {
                                cJSON * apertureItem = cJSON_GetObjectItemCaseSensitive(apertureListItem, "aperture");
                                if (apertureItem->valuedouble > fnumber && apertureScaleFactor == 0)
                                {
                                    cJSON * scaleItem = cJSON_GetObjectItemCaseSensitive(apertureListItem, "scale_factor");
                                    apertureScaleFactor = scaleItem->valuedouble;
                                }
                            }
                            std::cout << "CamConst aperture scale factor: " << apertureScaleFactor << std::endl;
                        }
                    }
                }
            }
        }
    }

    if (whiteLevel == 0)
    {
        status = CAMCONST_READ_NOENTRY;
    }
    return status;
}
