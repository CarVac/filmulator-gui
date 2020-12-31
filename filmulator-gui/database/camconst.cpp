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

        FILE *f = fopen(dirstr.c_str(), "wb");
        if (f)
        {
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
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

camconst_status camconst_read(const QString filePath)
{
    if (filePath.length() == 0)
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

    std::string makeModel, name, value;

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
            cJSON * makeModelList = cJSON_GetObjectItemCaseSensitive(camera, "make_model");
            if (cJSON_IsArray(makeModelList))
            {
                makeModel = std::string(cJSON_GetArrayItem(makeModelList, 0)->valuestring);
                //TODO: make list of camera name synonyms, keep track of canonical name in a table
            } else {
                makeModel = std::string(makeModelList->valuestring);
            }
            std::cout << "make and model: " << makeModel << std::endl;

            cJSON * item;
            cJSON_ArrayForEach(item, camera)
            {
                std::cout << "item string: " << item->string << std::endl;
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
                        if (QString(rangeItem->string) == "white")
                        {
                            if (cJSON_IsNumber(rangeItem))
                            {
                                std::cout << "whitepoint level only: " << rangeItem->valuedouble << std::endl;
                            }
                            if (cJSON_IsArray(rangeItem))
                            {
                                cJSON * whiteItem;
                                cJSON_ArrayForEach(whiteItem, rangeItem)
                                {
                                    if (QString(whiteItem->string) == "iso")
                                    {
                                        if (cJSON_IsNumber(whiteItem))
                                        {
                                            std::cout << "whitepoint iso: " << whiteItem->valuedouble << std::endl;
                                        }
                                        if (cJSON_IsArray(whiteItem))
                                        {
                                            cJSON * isoItem;
                                            std::cout << "whitepoint isos: ";
                                            cJSON_ArrayForEach(isoItem, whiteItem)
                                            {
                                                std::cout << isoItem->valuedouble << " ";
                                            }
                                            std::cout << std::endl;
                                        }
                                    }
                                    if (QString(whiteItem->string) == "levels")
                                    {
                                        if (cJSON_IsNumber(whiteItem))
                                        {
                                            std::cout << "whitepoint level: " << whiteItem->valuedouble << std::endl;
                                        }
                                        if (cJSON_IsArray(whiteItem))
                                        {
                                            cJSON * levelsItem;
                                            double whitepointLevel = 1e10;
                                            cJSON_ArrayForEach(levelsItem, whiteItem)
                                            {
                                                whitepointLevel = std::min(whitepointLevel, levelsItem->valuedouble);
                                            }
                                            std::cout << "whitepoint lowest level: " << whitepointLevel << std::endl;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return status;
}
