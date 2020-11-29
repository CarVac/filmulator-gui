#ifndef IMAGEPIPELINE_H
#define IMAGEPIPELINE_H
#include "filmSim.hpp"
#include "interface.h"
#include "../ui/parameterManager.h"
#include <QMutex>
#include <QMutexLocker>
#include <rtprocess/librtprocess.h>

enum Cache {WithCache, NoCache};
enum Histo {WithHisto, NoHisto};
enum QuickQuality { LowQuality, PreviewQuality, HighQuality };

class ImagePipeline
{
public:
    ImagePipeline(Cache, Histo, QuickQuality);

    //Loads and processes an image according to the 'params' structure, monitoring 'aborted' for cancellation.
    matrix<unsigned short>& processImage(ParameterManager * paramManager,
                                         Interface * histoInterface,
                                         Exiv2::ExifData &exifOutput);

    //Returns the progress of the pipeline from 0, incomplete, to 1, complete.
    float getProgress(){return progress;}

    //Returns a copy of the latest image, in a full color interleaved 16-bit per color format.
    //TODO: remove this!
    matrix<unsigned short> getLastImage();

    //Lets the consumer turn cache on and off
    void setCache(Cache cacheIn);

    //Variable relating to stealing the demosaiced data from another imagepipeline
    bool stealData = false;
    ImagePipeline * stealVictim;

    //Method to straight up copy the data from another imagepipeline
    //This is used when copying preloaded pipeline data
    void copyPipeline(ImagePipeline * copySource);
    //This should be used after the full res pipeline is done
    // so that the quick pipe gets refreshed from high res sampling
    //This is important to keep the quick pipe sharp after level/distortion
    // correction
    //Only used for pipelines that already are based on the same image.
    void copyAndDownsampleImages(ImagePipeline * copySource);

    //This is related to the above; if the image changes but the pipeline is
    // preloaded, we need to refresh the histograms
    void rerunHistograms();

    //The resolution of a quick preview
    int resolution;

protected:
    matrix<unsigned short>& emptyMatrix(){return empty;}

    Cache cache;
    bool cacheEmpty = false;
    bool hasStartedProcessing = false;
    Histo histo;
    QuickQuality quality;
    Interface * histoInterface;

    Valid valid;
    float progress;

    LUT<unsigned short> lutR, lutG, lutB;
    LUT<unsigned short> filmLikeLUT;

    struct timeval timeRequested;

    //raw stuff
    matrix<float> raw_image;
    matrix<unsigned short> empty;
    unsigned cfa[2][2];
    unsigned xtrans[6][6];
    int maxXtrans;
    int raw_width, raw_height;
    float camToRGB[3][3];
    float camToRGB4[3][4];
    float rCamMul, gCamMul, bCamMul;//wb used on the image
    float rPreMul, gPreMul, bPreMul;//"daylight" wb according to libraw
    float maxValue;
    bool isSraw;//Actually we should set this for all full-color raws (including X-Transformer)
    bool isNikonSraw;
    bool isMonochrome;
    bool isCR3;

    matrix<float> input_image;
    matrix<float> recovered_image;
    matrix<float> pre_film_image;
    Exiv2::ExifData exifData;
    Exiv2::ExifData basicExifData;//for tiff writing
    matrix<float> filmulated_image;
    matrix<unsigned short> contrast_image;
    matrix<unsigned short> color_curve_image;
    matrix<unsigned short> vibrance_saturation_image;

    //Internal functions for progress and time tracking.
    vector<double> completionTimes;
    void updateProgress(Valid valid, float CurrFractionCompleted);

    //The core filmulation. It needs to access ProcessingParameters, so it's here.
    bool filmulate(matrix<float> &scaled_image,
                   matrix<float> &output_density,
                   ParameterManager * paramManager,
                   ImagePipeline * pipeline);

    //Callback for LibRaw cancellation
    static int libraw_callback(void *data, enum LibRaw_progress p, int iteration, int expected);
};

#endif // IMAGEPIPELINE_H
