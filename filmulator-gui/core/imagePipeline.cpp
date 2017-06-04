#include "imagePipeline.h"

ImagePipeline::ImagePipeline(Cache cacheIn, Histo histoIn, QuickQuality qualityIn)
{
    cache = cacheIn;
    histo = histoIn;
    quality = qualityIn;
    valid = Valid::none;

    completionTimes.resize(Valid::count);
    completionTimes[Valid::none] = 0;
    completionTimes[Valid::load] = 5;
    completionTimes[Valid::demosaic] = 50;
    completionTimes[Valid::prefilmulation] = 5;
    completionTimes[Valid::filmulation] = 50;
    completionTimes[Valid::blackwhite] = 10;
    completionTimes[Valid::colorcurve] = 10;
    completionTimes[Valid::filmlikecurve] = 10;

}

//int ImagePipeline::libraw_callback(void *data, LibRaw_progress p, int iteration, int expected)
//but we only need data.
int ImagePipeline::libraw_callback(void *data, LibRaw_progress, int, int)
{
    AbortStatus abort;
    Valid validity;

    //Recover the param_manager from the data
    ParameterManager * pManager = static_cast<ParameterManager*>(data);
    //See whether to abort or not.
    //Because LibRaw does the demosaicing, we need to use the check that's performed afterwards
    //That's prefilmulation.
    //If we ever use LibRaw only for decoding, then change this to do the check for demosaicing.
    std::tie(validity, abort, std::ignore) = pManager->claimPrefilmParams();
    if (abort == AbortStatus::restart)
    {
        return 1;//cancel processing
    }
    else
    {
        return 0;
    }
}

matrix<unsigned short> ImagePipeline::processImage(ParameterManager * paramManager,
                                                   Interface * interface_in,
                                                   Exiv2::ExifData &exifOutput)
{
    //Say that we've started processing to prevent cache status from changing..
    hasStartedProcessing = true;
    //Record when the function was requested. This is so that the function will not give up
    // until a given short time has elapsed.
    gettimeofday(&timeRequested, NULL);
    interface = interface_in;

    valid = paramManager->getValid();
    if (NoCache == cache || true == cacheEmpty)
    {
        valid = none;//we need to start fresh if nothing is going to be cached.
    }

    LoadParams loadParam;
    DemosaicParams demosaicParam;
    PrefilmParams prefilmParam;
    FilmParams filmParam;
    BlackWhiteParams blackWhiteParam;
    FilmlikeCurvesParams curvesParam;
    OrientationParams orientationParam;

    updateProgress(valid, 0.0f);
    switch (valid)
    {
    case none://Load image into buffer
    {
        AbortStatus abort;
        //See whether to abort or not, while grabbing the latest parameters.
        std::tie(valid, abort, loadParam) = paramManager->claimLoadParams();
        if (abort == AbortStatus::restart)
        {
            return emptyMatrix();
        }
        //In the future we'll actually perform loading here.
        updateProgress(valid, 0.0f);
    }
    case load://Do demosaic
    {
        AbortStatus abort;
        //Because the load params are used here
        std::tie(valid, abort, loadParam) = paramManager->claimLoadParams();
        std::tie(valid, abort, demosaicParam) = paramManager->claimDemosaicParams();
        if (abort == AbortStatus::restart)
        {
            cout << "imagePipeline.cpp: aborted at demosaic" << endl;
            return emptyMatrix();
        }

        cout << "imagePipeline.cpp: Opening " << loadParam.fullFilename << endl;

        matrix<float> input_image;
        //Reads in the photo.
        cout << "load start:" << timeDiff (timeRequested) << endl;
        struct timeval imload_time;
        gettimeofday( &imload_time, NULL );
        /*
        if (imload(loadParam.fullFilename,
                  input_image,
                  loadParam.tiffIn,
                  loadParam.jpegIn,
                  exifData,
                  demosaicParam.highlights,
                  demosaicParam.caEnabled,
                  (LowQuality == quality)))
        {
            //Tell the param manager to abort back for some reason? maybe?
            //It was setting valid back to none.
            return emptyMatrix();
        }
        */
        if (loadParam.tiffIn)
        {
            if (imread_tiff(loadParam.fullFilename, input_image, exifData))
            {
                cerr << "Could not open image " << loadParam.fullFilename << "; Exiting..." << endl;
                return emptyMatrix();
            }
        }
        else if (loadParam.jpegIn)
        {
            if (imread_jpeg(loadParam.fullFilename, input_image, exifData))
            {
                cerr << "Could not open image " << loadParam.fullFilename << "; Exiting..." << endl;
                return emptyMatrix();
            }
        }
        else //raw
        {
            //Create image processor for reading raws.
            LibRaw image_processor;

            //Connect image processor with callback for cancellation
            image_processor.set_progress_handler(ImagePipeline::libraw_callback, paramManager);

            //Open the file.
            const char *cstr = loadParam.fullFilename.c_str();
            if (0 != image_processor.open_file(cstr))
            {
                cerr << "processImage: Could not read input file!" << endl;
                return emptyMatrix();
            }
             //Make abbreviations for brevity in accessing data.
#define SIZES image_processor.imgdata.sizes
#define PARAM image_processor.imgdata.params
#define IMAGE image_processor.imgdata.image
#define COLOR image_processor.imgdata.color

            //Now we'll set demosaic and other processing settings.
            PARAM.user_qual = 10;//9;//10 is AMaZE; -q[#] in dcraw
            PARAM.no_auto_bright = 1;//Don't autoadjust brightness (-W)
            PARAM.output_bps = 16;//16 bits per channel (-6)
            PARAM.gamm[0] = 1;
            PARAM.gamm[1] = 1;//Linear gamma (-g 1 1)
            PARAM.ca_correc = demosaicParam.caEnabled;//Turn on CA auto correction
            PARAM.cared = 0;
            PARAM.cablue = 0;
            PARAM.output_color = 1;//1: Use sRGB regardless.
            PARAM.use_camera_wb = 1;//1: Use camera WB setting (-w)
            PARAM.highlight = demosaicParam.highlights;//Set highlight recovery (-H #)
            PARAM.med_passes = 1;//median filter

            if (LowQuality == quality)
            {
                //PARAM.half_size = 1;//half-size output, should dummy down demosaic.
                /* The above sometimes read out a dng thumbnail instead of the image itself. */
                PARAM.user_qual = 0;//nearest-neighbor demosaic
                PARAM.ca_correc = 0;//turn off auto CA correction.
            }

            AbortStatus abort;
            std::tie(valid, abort, prefilmParam) = paramManager->claimPrefilmParams();
            if (abort == AbortStatus::restart)
            {
                return emptyMatrix();
            }

            //This makes IMAGE contains the sensel value and 3 blank values at every
            //location.
            if (0 != image_processor.unpack())
            {
                cerr << "processImage: Could not read input file, or was canceled" << endl;
                return emptyMatrix();
            }

            std::tie(valid, abort, prefilmParam) = paramManager->claimPrefilmParams();
            if (abort == AbortStatus::restart)
            {
                return emptyMatrix();
            }

            //This calls the dcraw processing on the raw sensel data.
            //Now, it contains 3 color values and one blank value at every location.
            //We will ignore the last blank value.
            if (0 != image_processor.dcraw_process())
            {
                cerr << "processImage: Processing was canceled during dcraw_process" << endl;
                return emptyMatrix();
            }

            long rSum = 0, gSum = 0, bSum = 0;
            input_image.set_size(SIZES.iheight, SIZES.iwidth*3);
            for (int row = 0; row < SIZES.iheight; row++)
            {
                //IMAGE is an (width*height) by 4 array, not width by height by 4.
                int rowoffset = row*SIZES.iwidth;
                for (int col = 0; col < SIZES.iwidth; col++)
                {
                    input_image(row, col*3    ) = IMAGE[rowoffset + col][0];//R
                    input_image(row, col*3 + 1) = IMAGE[rowoffset + col][1];//G
                    input_image(row, col*3 + 2) = IMAGE[rowoffset + col][2];//B
                    rSum += IMAGE[rowoffset + col][0];
                    gSum += IMAGE[rowoffset + col][1];
                    bSum += IMAGE[rowoffset + col][2];
                }
            }
            image_processor.recycle();
            Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(loadParam.fullFilename);
            assert(image.get() != 0);
            image->readMetadata();
            exifData = image->exifData();
        }
        cout << "load time: " << timeDiff(imload_time);

        cout << "ImagePipeline::processImage: Demosaic complete." << endl;

        if (LowQuality == quality)
        {
            cout << "scale start:" << timeDiff (timeRequested) << endl;
            struct timeval downscale_time;
            gettimeofday( &downscale_time, NULL );
            downscale_and_crop(input_image,cropped_image, 0, 0, (input_image.nc()/3)-1,input_image.nr()-1, 600, 600);
            //cropped_image = input_image;
            cout << "scale end: " << timeDiff( downscale_time ) << endl;
        }
        else
        {
            cropped_image = input_image;
        }
        updateProgress(valid, 0.0f);
    }
    case demosaic://Do pre-filmulation work.
    {
        AbortStatus abort;
        std::tie(valid, abort, prefilmParam) = paramManager->claimPrefilmParams();
        if (abort == AbortStatus::restart)
        {
            return emptyMatrix();
        }

        //Here we apply the exposure compensation and white balance.
        matrix<float> exposureImage = cropped_image * pow(2, prefilmParam.exposureComp);
        whiteBalance(exposureImage,
                     pre_film_image,
                     prefilmParam.temperature,
                     prefilmParam.tint,
                     prefilmParam.fullFilename);

        if (NoCache == cache)
        {
            cropped_image.set_size( 0, 0 );
            cacheEmpty = true;
        }
        else
        {
            cacheEmpty = false;
        }
        if (WithHisto == histo)
        {
            //Histogram work
            interface->updateHistPreFilm(pre_film_image, 65535);
        }

        cout << "ImagePipeline::processImage: Prefilmulation complete." << endl;

        updateProgress(valid, 0.0f);
    }
    case prefilmulation://Do filmulation
    {
        //We don't need to check abort status out here, because
        //the filmulate function will do so inside its loop multiple times.
        //We just check for it returning an empty matrix.

        //Here we do the film simulation on the image...
        //If filmulate detects an abort, it returns true.
        if (filmulate(pre_film_image,
                      filmulated_image,
                      paramManager,
                      this))
        {
            return emptyMatrix();
        }

        if (NoCache == cache)
        {
            pre_film_image.set_size(0, 0);
            cacheEmpty = true;
        }
        else
        {
            cacheEmpty = false;
        }
        if (WithHisto == histo)
        {
            //Histogram work
            interface->updateHistPostFilm(filmulated_image, .0025);//TODO connect this magic number to the qml
        }

        cout << "ImagePipeline::processImage: Filmulation complete." << endl;

        //Now, since we didn't check abort status out here, we do have to at least
        // increment the validity.
        AbortStatus abort;
        std::tie(valid, abort, filmParam) = paramManager->claimFilmParams(FilmFetch::subsequent);
        updateProgress(valid, 0.0f);
    }
    case filmulation://Do whitepoint_blackpoint
    {
        AbortStatus abort;
        std::tie(valid, abort, blackWhiteParam) = paramManager->claimBlackWhiteParams();
        if (abort == AbortStatus::restart)
        {
            return emptyMatrix();
        }

        whitepoint_blackpoint(filmulated_image,
                              contrast_image,
                              blackWhiteParam.whitepoint,
                              blackWhiteParam.blackpoint);

        if (NoCache == cache)
        {
            filmulated_image.set_size(0, 0);
            cacheEmpty = true;
        }
        else
        {
            cacheEmpty = false;
        }
        updateProgress(valid, 0.0f);
    }
    case blackwhite: // Do color_curve
    {
        //It's not gonna abort because we have no color curves yet..
        //Prepare LUT's for individual color processin.g
        lutR.setUnity();
        lutG.setUnity();
        lutB.setUnity();
        colorCurves(contrast_image,
                    color_curve_image,
                    lutR,
                    lutG,
                    lutB);

        if (NoCache == cache)
        {
            contrast_image.set_size(0, 0);
        }
        else
        {
            cacheEmpty = false;
        }
        updateProgress(valid, 0.0f);
    }
    case colorcurve://Do film-like curve
    {
        AbortStatus abort;
        std::tie(valid, abort, curvesParam) = paramManager->claimFilmlikeCurvesParams();
        if (abort == AbortStatus::restart)
        {
            return emptyMatrix();
        }

        filmLikeLUT.fill( [=](unsigned short in) -> unsigned short
            {
                float shResult = shadows_highlights(float(in)/65535.0,
                                                     curvesParam.shadowsX,
                                                     curvesParam.shadowsY,
                                                     curvesParam.highlightsX,
                                                     curvesParam.highlightsY);
                return 65535*default_tonecurve(shResult);
            }
        );
        matrix<unsigned short> film_curve_image;
        film_like_curve(color_curve_image,
                        film_curve_image,
                        filmLikeLUT);
        vibrance_saturation(film_curve_image,
                            vibrance_saturation_image,
                            curvesParam.vibrance,
                            curvesParam.saturation);

        if (NoCache == cache)
        {
            color_curve_image.set_size(0, 0);
            cacheEmpty = true;
            //film_curve_image is going out of scope anyway.
        }
        else
        {
            cacheEmpty = false;
        }

        updateProgress(valid, 0.0f);
    }
    default://output
    {
        AbortStatus abort;
        std::tie(valid, abort, orientationParam) = paramManager->claimOrientationParams();
        //We won't abort now,
        if (abort == AbortStatus::restart)
        {
            cout << "why are we aborting here?" << endl;
            return emptyMatrix();
        }

        rotate_image(vibrance_saturation_image,
                     rotated_image,
                     orientationParam.rotation);

        if (NoCache == cache)
        {
            vibrance_saturation_image.set_size(0, 0);
            cacheEmpty = true;
        }
        else
        {
            cacheEmpty = false;
        }
        if (WithHisto == histo)
        {
            interface->updateHistFinal(rotated_image);
        }
        updateProgress(valid, 0.0f);

        exifOutput = exifData;
        return rotated_image;
    }
    }//End task switch

    return emptyMatrix();
}

//Saved for posterity: we may want to re-implement this 0.1 second continuation
// inside of parameterManager.
//bool ImagePipeline::checkAbort(bool aborted)
//{
//    if (aborted && timeDiff(timeRequested) > 0.1)
//    {
//        cout << "ImagePipeline::aborted. valid = " << valid << endl;
//        return true;
//    }
//    else
//    {
//        return false;
//    }
//}

void ImagePipeline::updateProgress(Valid valid, float stepProgress)
{
    double totalTime = numeric_limits<double>::epsilon();
    double totalCompletedTime = 0;
    for (int i = 0; i < (int) completionTimes.size(); i++)
    {
        totalTime += completionTimes[i];
        float fractionCompleted = 0;
        if (i <= valid)
            fractionCompleted = 1;
        if (i == valid + 1)
            fractionCompleted = stepProgress;
        //if greater -> 0
        totalCompletedTime += completionTimes[i]*fractionCompleted;
    }
    interface->setProgress(totalCompletedTime/totalTime);
}

//Do not call this on something that's already been used!
void ImagePipeline::setCache(Cache cacheIn)
{
    if (false == hasStartedProcessing)
    {
        cache = cacheIn;
    }
}
