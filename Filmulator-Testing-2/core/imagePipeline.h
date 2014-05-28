#ifndef IMAGEPIPELINE_H
#define IMAGEPIPELINE_H
#include "filmSim.hpp"
#include "interface.h"
#include <mutex>

enum Valid {none, load, demosaic, prefilmulation, filmulation, whiteblack, colorcurve, filmlikecurve};

struct ProcessingParameters {
    //none valid

    std::vector<std::string> filenameList;
    bool tiffIn;
    bool jpegIn;
    //load is now valid

    bool caEnabled;
    int highlights;
    //demosaic is now valid

    std::vector<float> exposureComp;
    double temperature;
    double tint;
    //prefilmulation stuff is now valid

    filmulateParams filmParams;
    //filmulation is now valid

    float blackpoint;
    float whitepoint;
    //whiteblack is now valid

    //colorcurve is now valid (empty for now)

    float shadowsX;
    float shadowsY;
    float highlightsX;
    float highlightsY;
    float vibrance;
    float saturation;
    //filmlikecurve is now valid

    int rotation;
};

enum CacheAndHisto { BothCacheAndHisto, NoCacheNoHisto };
enum QuickQuality { LowQuality, HighQuality };

class ImagePipeline
{
public:
    ImagePipeline( CacheAndHisto, QuickQuality );
    matrix<unsigned short> processImage( const ProcessingParameters params, Interface* interface, bool &aborted );
    float getProgress(){ return progress; }

protected:
    matrix<unsigned short> emptyMatrix(){ matrix<unsigned short> mat; return mat;}

    CacheAndHisto cacheHisto;
    QuickQuality quality;
    Interface* interface;

    float progress;

    ProcessingParameters oldParams;

    LUT lutR, lutG, lutB;
    LUT filmLikeLUT;

    struct timeval timeRequested;
    Valid valid;

    matrix<float> input_image;
    matrix<float> pre_film_image;
    Exiv2::ExifData exifData;
    matrix<float> filmulated_image;
    matrix<unsigned short> contrast_image;
    matrix<unsigned short> color_curve_image;
    matrix<unsigned short> vibrance_saturation_image;

    //Internal functions for progress and time tracking.
    bool checkAbort( bool aborted );
    void setValid( Valid );
    void setLastValid( ProcessingParameters );

    //The core filmulation. It needs to access checkAbort, so it's here.
    bool filmulate( matrix<float> &input_image, matrix<float> &output_density,
                    filmulateParams filmParams, Interface* interface, bool &aborted );
};

#endif // IMAGEPIPELINE_H
