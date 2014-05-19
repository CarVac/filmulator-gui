#ifndef IMAGEPIPELINE_H
#define IMAGEPIPELINE_H
#include "filmsim.hpp"
#include "interface.h"

struct ProcessingParameters {
    std::vector<std::string> filenameList;
    bool tiffIn;
    bool jpegIn;

    int rotation;

    filmulateParams filmParams;
    bool caEnabled;
    int highlights;

    std::vector<float> exposureComp;
    double temperature;
    double tint;

    float blackpoint;
    float whitepoint;
    float shadowsX;
    float shadowsY;
    float highlightsX;
    float highlightsY;

    float vibrance;
    float saturation;
};

enum CacheAndHisto { BothCacheAndHisto, NoCacheNoHisto };

class ImagePipeline
{
public:
    ImagePipeline( CacheAndHisto, Interface* );
    matrix<unsigned short> processImage( ProcessingParameters );
protected:
    matrix<unsigned short> emptyMatrix(){ matrix<unsigned short> mat; return mat;}

    CacheAndHisto cacheAndHistograms;
    Interface* interface;

    float progress;

    LUT lutR, lutG, lutB;
    LUT filmLikeLUT;

    Valid valid;

    matrix<float> input_image;
    matrix<float> pre_film_image;
    Exiv2::ExifData exifData;
    matrix<float> filmulated_image;
    matrix<unsigned short> contrast_image;
    matrix<unsigned short> color_curve_image;
    matrix<unsigned short> vibrance_saturation_image;
};

#endif // IMAGEPIPELINE_H
