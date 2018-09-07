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
    //completionTimes[Valid::filmlikecurve] = 10;

}

//int ImagePipeline::libraw_callback(void *data, LibRaw_progress p, int iteration, int expected)
//but we only need data.
int ImagePipeline::libraw_callback(void *data, LibRaw_progress, int, int)
{
    AbortStatus abort;

    //Recover the param_manager from the data
    ParameterManager * pManager = static_cast<ParameterManager*>(data);
    //See whether to abort or not.
    abort = pManager->claimDemosaicAbort();
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
    histoInterface = interface_in;

    valid = paramManager->getValid();
    if (NoCache == cache || true == cacheEmpty)
    {
        valid = none;//we need to start fresh if nothing is going to be cached.
    }

    LoadParams loadParam;
    DemosaicParams demosaicParam;
    PrefilmParams prefilmParam;
    //FilmParams filmParam;
    BlackWhiteParams blackWhiteParam;
    FilmlikeCurvesParams curvesParam;

    updateProgress(valid, 0.0f);
    switch (valid)
    {
    case partload: [[fallthrough]];
    case none://Load image into buffer
    {
        AbortStatus abort;
        //See whether to abort or not, while grabbing the latest parameters.
        std::tie(valid, abort, loadParam) = paramManager->claimLoadParams();
        if (abort == AbortStatus::restart)
        {
            return emptyMatrix();
        }

        if (!loadParam.tiffIn && !loadParam.jpegIn)
        {
            LibRaw image_processor;

            //Open the file.
            const char *cstr = loadParam.fullFilename.c_str();
            if (0 != image_processor.open_file(cstr))
            {
                cout << "processImage: Could not read input file!" << endl;
                return emptyMatrix();
            }
             //Make abbreviations for brevity in accessing data.
#define RSIZE image_processor.imgdata.sizes
#define PARAM image_processor.imgdata.params
#define IMAGE image_processor.imgdata.image
#define RAW   image_processor.imgdata.rawdata.raw_image
//#define COLOR image_processor.imgdata.color

            //This makes IMAGE contains the sensel value and 3 blank values at every
            //location.
            if (0 != image_processor.unpack())
            {
                cerr << "processImage: Could not read input file, or was canceled" << endl;
                return emptyMatrix();
            }

            //get dimensions
            raw_width  = RSIZE.width;
            raw_height = RSIZE.height;

            int topmargin = RSIZE.top_margin;
            int leftmargin = RSIZE.left_margin;
            int full_width = RSIZE.raw_width;
            //int full_height = RSIZE.raw_height;

            //get color matrix
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    camToRGB[i][j] = image_processor.imgdata.color.rgb_cam[i][j];
                    cout << camToRGB[i][j] << " ";
                }
                cout << endl;
            }
            rMult = image_processor.imgdata.color.cam_mul[0]/1024.0f;
            gMult = image_processor.imgdata.color.cam_mul[1]/1024.0f;
            bMult = image_processor.imgdata.color.cam_mul[2]/1024.0f;
            cout << "rmult: " << image_processor.imgdata.color.cam_mul[0] << endl;
            cout << "gmult: " << image_processor.imgdata.color.cam_mul[1] << endl;
            cout << "bmult: " << image_processor.imgdata.color.cam_mul[2] << endl;
            cout << "rpremult: " << image_processor.imgdata.color.pre_mul[0] << endl;
            cout << "gpremult: " << image_processor.imgdata.color.pre_mul[1] << endl;
            cout << "bpremult: " << image_processor.imgdata.color.pre_mul[2] << endl;

            //get black subtraction values
            rBlack = image_processor.imgdata.color.cblack[0];
            gBlack = image_processor.imgdata.color.cblack[1];
            bBlack = image_processor.imgdata.color.cblack[2];
            float blackpoint = image_processor.imgdata.color.black;
            cout << "blackpoint: " << blackpoint << endl;

            //get color filter array
            //bayer only for now
            cfa[0][0] = unsigned(image_processor.COLOR(0 + topmargin, 1 + leftmargin));
            cfa[0][1] = unsigned(image_processor.COLOR(0 + topmargin, 0 + leftmargin));
            cfa[1][0] = unsigned(image_processor.COLOR(1 + topmargin, 1 + leftmargin));
            cfa[1][1] = unsigned(image_processor.COLOR(1 + topmargin, 0 + leftmargin));



            raw_image(raw_width, raw_height);

            matrix<float> tempraw;
            tempraw.set_size(raw_width, raw_height);
            //copy raw data
            for (int row = 0; row < raw_height; row++)
            {
                //IMAGE is an (width*height) by 4 array, not width by height by 4.
                int rowoffset = (row + topmargin)*full_width;
                for (int col = 0; col < raw_width; col++)
                {
                    raw_image[row][col] = RAW[rowoffset + col + leftmargin] - blackpoint;
                    tempraw(row, col) = RAW[rowoffset + col + leftmargin] - blackpoint;
                }
            }

            cout << "max of tempraw" << tempraw.max() << endl;
        }
        //In the future we'll actually perform loading here.
        valid = paramManager->markLoadComplete();
        updateProgress(valid, 0.0f);
        [[fallthrough]];
    }
    case partdemosaic: [[fallthrough]];
    case load://Do demosaic, or load non-raw images
    {
        AbortStatus abort;
        std::tie(valid, abort, demosaicParam) = paramManager->claimDemosaicParams();
        if (abort == AbortStatus::restart)
        {
            cout << "imagePipeline.cpp: aborted at demosaic" << endl;
            return emptyMatrix();
        }

        cout << "imagePipeline.cpp: Opening " << loadParam.fullFilename << endl;

        //Reads in the photo.
        cout << "load start:" << timeDiff (timeRequested) << endl;
        struct timeval imload_time;
        gettimeofday( &imload_time, NULL );

        if ((HighQuality == quality) && stealData)//only full pipelines may steal data
        {
            scaled_image = stealVictim->input_image;
            exifData = stealVictim->exifData;
            rMult = stealVictim->rMult;
            gMult = stealVictim->gMult;
            bMult = stealVictim->bMult;
        }
        else if (loadParam.tiffIn)
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
            array2D<float> red, green, blue;
            red(raw_width, raw_height);
            green(raw_width, raw_height);
            blue(raw_width, raw_height);

            double initialGain = 1.0;
            float inputscale = 16383.0;
            float outputscale = 65535.0;
            int border = 4;
            std::function<bool(double)> setProg = [](double) -> bool {return false;};
            librtprocess::amaze_demosaic(0, 0, raw_width-1, raw_height-1, raw_image, red, green, blue, cfa, setProg, initialGain, border, inputscale, outputscale);

            input_image.set_size(raw_height, raw_width*3);
            matrix<float> rmat, gmat, bmat;
            rmat.set_size(raw_height, raw_width);
            gmat.set_size(raw_height, raw_width);
            bmat.set_size(raw_height, raw_width);
            for (int row = 0; row < raw_height; row++)
            {
                for (int col = 0; col < raw_width; col++)
                {
                    input_image(row, col*3    ) = camToRGB[0][0] * red[row][col] + camToRGB[1][0] * green[row][col] + camToRGB[2][0] * blue[row][col];
                    input_image(row, col*3 + 1) = camToRGB[0][1] * red[row][col] + camToRGB[1][1] * green[row][col] + camToRGB[2][1] * blue[row][col];
                    input_image(row, col*3 + 2) = camToRGB[0][2] * red[row][col] + camToRGB[1][2] * green[row][col] + camToRGB[2][2] * blue[row][col];
//                    rmat(row, col) =              camToRGB[0][0] * red[row][col] + camToRGB[1][0] * green[row][col] + camToRGB[2][0] * blue[row][col];
//                    gmat(row, col) =              camToRGB[0][1] * red[row][col] + camToRGB[1][1] * green[row][col] + camToRGB[2][1] * blue[row][col];
//                    bmat(row, col) =              camToRGB[0][2] * red[row][col] + camToRGB[1][2] * green[row][col] + camToRGB[2][2] * blue[row][col];
                    rmat(row, col) = red[row][col];
                    gmat(row, col) = green[row][col];
                    bmat(row, col) = blue[row][col];
                }
            }
            cout << "mean rmat: " << rmat.mean() << endl;
            cout << "mean gmat: " << gmat.mean() << endl;
            cout << "mean bmat: " << bmat.mean() << endl;
            cout << "=========================================" << endl;
            cout << "rmult * mean rmat: " << rmat.mean()*rMult << endl;
            cout << "gmult * mean gmat: " << gmat.mean()*gMult << endl;
            cout << "bmult * mean bmat: " << bmat.mean()*bMult << endl;
            Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(loadParam.fullFilename);
            assert(image.get() != 0);
            image->readMetadata();
            exifData = image->exifData();
        }
        cout << "load time: " << timeDiff(imload_time) << endl;

        cout << "ImagePipeline::processImage: Demosaic complete." << endl;

        if (LowQuality == quality)
        {
            cout << "scale start:" << timeDiff (timeRequested) << endl;
            struct timeval downscale_time;
            gettimeofday( &downscale_time, NULL );
            downscale_and_crop(input_image,scaled_image, 0, 0, (input_image.nc()/3)-1,input_image.nr()-1, 600, 600);
            cout << "scale end: " << timeDiff( downscale_time ) << endl;
        }
        else if (PreviewQuality == quality)
        {
            cout << "scale start:" << timeDiff (timeRequested) << endl;
            struct timeval downscale_time;
            gettimeofday( &downscale_time, NULL );
            downscale_and_crop(input_image,scaled_image, 0, 0, (input_image.nc()/3)-1,input_image.nr()-1, resolution, resolution);
            cout << "scale end: " << timeDiff( downscale_time ) << endl;
        }
        else
        {
            if (!stealData) //If we had to compute the input image ourselves
            {
                scaled_image = input_image;
                input_image.set_size(0,0);
            }
        }

        valid = paramManager->markDemosaicComplete();
        updateProgress(valid, 0.0f);
        [[fallthrough]];
    }
    case partprefilmulation: [[fallthrough]];
    case demosaic://Do pre-filmulation work.
    {
        AbortStatus abort;
        std::tie(valid, abort, prefilmParam) = paramManager->claimPrefilmParams();
        if (abort == AbortStatus::restart)
        {
            return emptyMatrix();
        }

        //Here we apply the exposure compensation and white balance.
        matrix<float> exposureImage = scaled_image * pow(2, prefilmParam.exposureComp);
        whiteBalance(exposureImage,
                     pre_film_image,
                     prefilmParam.temperature,
                     prefilmParam.tint,
                     prefilmParam.fullFilename,
                     camToRGB);

        if (NoCache == cache)
        {
            scaled_image.set_size( 0, 0 );
            cacheEmpty = true;
        }
        else
        {
            cacheEmpty = false;
        }
        if (WithHisto == histo)
        {
            //Histogram work
            histoInterface->updateHistPreFilm(pre_film_image, 65535);
        }

        cout << "ImagePipeline::processImage: Prefilmulation complete." << endl;

        valid = paramManager->markPrefilmComplete();
        updateProgress(valid, 0.0f);
        [[fallthrough]];
    }
    case partfilmulation: [[fallthrough]];
    case prefilmulation://Do filmulation
    {
        //We don't need to check abort status out here, because
        //the filmulate function will do so inside its loop.
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
            histoInterface->updateHistPostFilm(filmulated_image, .0025);//TODO connect this magic number to the qml
        }

        cout << "ImagePipeline::processImage: Filmulation complete." << endl;

        valid = paramManager->markFilmComplete();
        updateProgress(valid, 0.0f);
        [[fallthrough]];
    }
    case partblackwhite: [[fallthrough]];
    case filmulation://Do whitepoint_blackpoint
    {
        AbortStatus abort;
        std::tie(valid, abort, blackWhiteParam) = paramManager->claimBlackWhiteParams();
        if (abort == AbortStatus::restart)
        {
            return emptyMatrix();
        }
        matrix<float> rotated_image;

        rotate_image(filmulated_image,
                     rotated_image,
                     blackWhiteParam.rotation);

        if (NoCache == cache)// clean up ram that's not needed anymore in order to reduce peak consumption
        {
            filmulated_image.set_size(0, 0);
            cacheEmpty = true;
        }
        else
        {
            cacheEmpty = false;
        }

        const int imWidth  = rotated_image.nc()/3;
        const int imHeight = rotated_image.nr();

        const float tempHeight = imHeight*max(min(1.0f,blackWhiteParam.cropHeight),0.0f);//restrict domain to 0:1
        const float tempAspect = max(min(10000.0f,blackWhiteParam.cropAspect),0.0001f);//restrict aspect ratio
        int width  = round(min(tempHeight*tempAspect,float(imWidth)));
        int height = round(min(tempHeight, imWidth/tempAspect));
        const float maxHoffset = (1-(float(width)  / float(imWidth) ))/2.0;
        const float maxVoffset = (1-(float(height) / float(imHeight)))/2.0;
        const float oddH = (!(round((imWidth  - width )/2.0)*2 == (imWidth  - width )))*0.5;//it's 0.5 if it's odd, 0 otherwise
        const float oddV = (!(round((imHeight - height)/2.0)*2 == (imHeight - height)))*0.5;//it's 0.5 if it's odd, 0 otherwise
        const float hoffset = (round(max(min(blackWhiteParam.cropHoffset, maxHoffset), -maxHoffset) * imWidth  + oddH) - oddH)/imWidth;
        const float voffset = (round(max(min(blackWhiteParam.cropVoffset, maxVoffset), -maxVoffset) * imHeight + oddV) - oddV)/imHeight;
        int startX = round(0.5*(imWidth  - width ) + hoffset*imWidth);
        int startY = round(0.5*(imHeight - height) + voffset*imHeight);
        int endX = startX + width  - 1;
        int endY = startY + height - 1;

        if (blackWhiteParam.cropHeight <= 0)//it shall be turned off
        {
            startX = 0;
            startY = 0;
            endX = imWidth  - 1;
            endY = imHeight - 1;
            width  = imWidth;
            height = imHeight;
        }

        matrix<float> cropped_image;

        downscale_and_crop(rotated_image,
                           cropped_image,
                           startX,
                           startY,
                           endX,
                           endY,
                           width,
                           height);

        rotated_image.set_size(0, 0);// clean up ram that's not needed anymore

        whitepoint_blackpoint(cropped_image,//filmulated_image,
                              contrast_image,
                              blackWhiteParam.whitepoint,
                              blackWhiteParam.blackpoint);

        valid = paramManager->markBlackWhiteComplete();
        updateProgress(valid, 0.0f);
        [[fallthrough]];
    }
    case partcolorcurve: [[fallthrough]];
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

        valid = paramManager->markColorCurvesComplete();
        updateProgress(valid, 0.0f);
        [[fallthrough]];
    }
    case partfilmlikecurve: [[fallthrough]];
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

        vibrance_saturation(film_curve_image,
                            vibrance_saturation_image,
                            curvesParam.vibrance,
                            curvesParam.saturation);

        updateProgress(valid, 0.0f);
        [[fallthrough]];
    }
    default://output
    {
        if (NoCache == cache)
        {
            //vibrance_saturation_image.set_size(0, 0);
            cacheEmpty = true;
        }
        else
        {
            cacheEmpty = false;
        }
        if (WithHisto == histo)
        {
            histoInterface->updateHistFinal(vibrance_saturation_image);
        }
        valid = paramManager->markFilmLikeCurvesComplete();
        updateProgress(valid, 0.0f);

        exifOutput = exifData;
        return vibrance_saturation_image;
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
    histoInterface->setProgress(totalCompletedTime/totalTime);
}

//Do not call this on something that's already been used!
void ImagePipeline::setCache(Cache cacheIn)
{
    if (false == hasStartedProcessing)
    {
        cache = cacheIn;
    }
}
