#ifndef IMAGEPIPELINE_H
#define IMAGEPIPELINE_H
#include "filmSim.hpp"
#include "interface.h"

enum Valid {none, load, demosaic, prefilmulation, filmulation, whiteblack, colorcurve, filmlikecurve, count};

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
    ImagePipeline(CacheAndHisto, QuickQuality);

    //Loads and processes an image according to the 'params' structure, monitoring 'aborted' for cancellation.
    matrix<unsigned short> processImage(const ProcessingParameters params, Interface* interface, bool &aborted,
                                         Exiv2::ExifData &exifOutput);

    //Returns the progress of the pipeline from 0, incomplete, to 1, complete.
    float getProgress(){ return progress; }

    //Returns a copy of the latest image, in a full color interleaved 16-bit per color format.
    matrix<unsigned short> getLastImage();

protected:
    matrix<unsigned short> emptyMatrix(){ matrix<unsigned short> mat; return mat;}


    CacheAndHisto cacheHisto;
    QuickQuality quality;
    Interface* interface;

    float progress;

    ProcessingParameters oldParams;

    LUT<unsigned short> lutR, lutG, lutB;
    LUT<unsigned short> filmLikeLUT;

    struct timeval timeRequested;
    Valid valid;

    matrix<float> cropped_image;
    matrix<float> pre_film_image;
    Exiv2::ExifData exifData;
    matrix<float> filmulated_image;
    matrix<unsigned short> contrast_image;
    matrix<unsigned short> color_curve_image;
    matrix<unsigned short> vibrance_saturation_image;
    matrix<unsigned short> rotated_image;

    //Internal functions for progress and time tracking.
    bool checkAbort(bool aborted);
    void setValid(Valid);
    void setLastValid(ProcessingParameters);
    vector<double> completionTimes;
    void updateProgress(float CurrFractionCompleted);

    //The core filmulation. It needs to access checkAbort, so it's here.
    bool filmulate( matrix<float> &cropped_image, matrix<float> &output_density,
                    filmulateParams filmParams, ImagePipeline* pipeline, bool &aborted );
};

#endif // IMAGEPIPELINE_H
