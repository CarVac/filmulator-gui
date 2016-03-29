#include "imagePipeline.h"

ImagePipeline::ImagePipeline(CacheAndHisto cacheAndHistoIn, QuickQuality qualityIn)
{
    cacheHisto = cacheAndHistoIn;
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

matrix<unsigned short> ImagePipeline::processImage(ParameterManager * paramManager,
                                                   Interface * interface_in,
                                                   Exiv2::ExifData &exifOutput)
{
    //Record when the function was requested. This is so that the function will not give up
    // until a given short time has elapsed.
    gettimeofday(&timeRequested, NULL);
    interface = interface_in;

    valid = paramManager->getValid();
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

        if (NoCacheNoHisto == cacheHisto)
        {
            cropped_image.set_size( 0, 0 );
        }
        else//(BothCacheAndHisto == cacheHisto)
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

        if (NoCacheNoHisto == cacheHisto)
        {
            pre_film_image.set_size(0, 0);
        }
        else
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

        if (NoCacheNoHisto == cacheHisto)
        {
            filmulated_image.set_size(0, 0);
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

        if (NoCacheNoHisto == cacheHisto)
        {
            contrast_image.set_size(0, 0);
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

        if (NoCacheNoHisto == cacheHisto)
        {
            color_curve_image.set_size(0, 0);
            //film_curve_image is going out of scope anyway.
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

        if (NoCacheNoHisto == cacheHisto)
        {
            vibrance_saturation_image.set_size(0, 0);
        }
        else
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
