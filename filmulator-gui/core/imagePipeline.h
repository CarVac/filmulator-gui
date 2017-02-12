#ifndef IMAGEPIPELINE_H
#define IMAGEPIPELINE_H
#include "filmSim.hpp"
#include "interface.h"
#include "../ui/parameterManager.h"

enum CacheAndHisto { BothCacheAndHisto, NoCacheNoHisto };
enum QuickQuality { LowQuality, HighQuality };

class ImagePipeline
{
public:
    ImagePipeline(CacheAndHisto, QuickQuality);

    //Loads and processes an image according to the 'params' structure, monitoring 'aborted' for cancellation.
    matrix<unsigned short> processImage(ParameterManager * paramManager,
                                        Interface * interface,
                                        Exiv2::ExifData &exifOutput);

    //Returns the progress of the pipeline from 0, incomplete, to 1, complete.
    float getProgress(){return progress;}

    //Returns a copy of the latest image, in a full color interleaved 16-bit per color format.
    matrix<unsigned short> getLastImage();

protected:
    matrix<unsigned short> emptyMatrix(){matrix<unsigned short> mat; return mat;}

    CacheAndHisto cacheHisto;
    QuickQuality quality;
    Interface * interface;

    Valid valid;
    float progress;

    LUT<unsigned short> lutR, lutG, lutB;
    LUT<unsigned short> filmLikeLUT;

    struct timeval timeRequested;

    matrix<float> cropped_image;
    matrix<float> pre_film_image;
    Exiv2::ExifData exifData;
    matrix<float> filmulated_image;
    matrix<unsigned short> contrast_image;
    matrix<unsigned short> color_curve_image;
    matrix<unsigned short> vibrance_saturation_image;
    matrix<unsigned short> rotated_image;

    //Internal functions for progress and time tracking.
    vector<double> completionTimes;
    void updateProgress(Valid valid, float CurrFractionCompleted);

    //The core filmulation. It needs to access ProcessingParameters, so it's here.
    bool filmulate(matrix<float> &cropped_image,
                   matrix<float> &output_density,
                   ParameterManager * paramManager,
                   ImagePipeline * pipeline);

    //Callback for LibRaw cancellation
    static int libraw_callback(void *data, enum LibRaw_progress p, int iteration, int expected);
};

#endif // IMAGEPIPELINE_H
