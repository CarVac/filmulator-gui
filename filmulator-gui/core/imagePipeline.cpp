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
    gettimeofday(&timeRequested, nullptr);
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

        if (!loadParam.tiffIn && !loadParam.jpegIn && !((HighQuality == quality) && stealData))
        {
            std::unique_ptr<LibRaw> image_processor = std::make_unique<LibRaw>();

            //Open the file.
            const char *cstr = loadParam.fullFilename.c_str();
            if (0 != image_processor->open_file(cstr))
            {
                cout << "processImage: Could not read input file!" << endl;
                return emptyMatrix();
            }
             //Make abbreviations for brevity in accessing data.
#define RSIZE image_processor->imgdata.sizes
#define PARAM image_processor->imgdata.params
#define IMAGE image_processor->imgdata.image
#define RAW   image_processor->imgdata.rawdata.raw_image
//#define COLOR image_processor.imgdata.color

            //This makes IMAGE contains the sensel value and 3 blank values at every
            //location.
            if (0 != image_processor->unpack())
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
                cout << "camToRGB: ";
                for (int j = 0; j < 3; j++)
                {
                    camToRGB[i][j] = image_processor->imgdata.color.rgb_cam[i][j];
                    cout << camToRGB[i][j] << " ";
                }
                cout << endl;
            }
            for (int i = 0; i < 3; i++)
            {
                cout << "camToRGB4: ";
                for (int j = 0; j < 4; j++)
                {
                    camToRGB4[i][j] = image_processor->imgdata.color.rgb_cam[i][j];
                    if (i==j)
                    {
                        camToRGB4[i][j] = 1;
                    } else {
                        camToRGB4[i][j] = 0;
                    }
                    if (j==3)
                    {
                        camToRGB4[i][j] = camToRGB4[i][1];
                    }
                    cout << camToRGB4[i][j] << " ";
                }
                cout << endl;
            }
            rCamMul = image_processor->imgdata.color.cam_mul[0];
            gCamMul = image_processor->imgdata.color.cam_mul[1];
            bCamMul = image_processor->imgdata.color.cam_mul[2];
            float minMult = min(min(rCamMul, gCamMul), bCamMul);
            rCamMul /= minMult;
            gCamMul /= minMult;
            bCamMul /= minMult;
            rPreMul = image_processor->imgdata.color.pre_mul[0];
            gPreMul = image_processor->imgdata.color.pre_mul[1];
            bPreMul = image_processor->imgdata.color.pre_mul[2];
            minMult = min(min(rPreMul, gPreMul), bPreMul);
            rPreMul /= minMult;
            gPreMul /= minMult;
            bPreMul /= minMult;

            //get black subtraction values
            //for everything
            float blackpoint = image_processor->imgdata.color.black;
            //some cameras have individual color channel subtraction. This hasn't been implemented yet.
            float rBlack = image_processor->imgdata.color.cblack[0];
            float gBlack = image_processor->imgdata.color.cblack[1];
            float bBlack = image_processor->imgdata.color.cblack[2];
            float g2Black = image_processor->imgdata.color.cblack[3];
            //Still others have a matrix to subtract.
            int blackRow = image_processor->imgdata.color.cblack[4];
            int blackCol = image_processor->imgdata.color.cblack[5];

            cout << "BLACKPOINT ======================================================" << endl;
            cout << blackpoint << endl;
            cout << "color channel blackpoints" << endl;
            cout << rBlack << endl;
            cout << gBlack << endl;
            cout << bBlack << endl;
            cout << g2Black << endl;
            cout << "block-based blackpoint dimensions:" << endl;
            cout << image_processor->imgdata.color.cblack[4] << endl;
            cout << image_processor->imgdata.color.cblack[5] << endl;
            cout << "block-based blackpoint: " << endl;
            if (blackRow > 0 && blackCol > 0)
            {
                for (int i = 0; i < blackRow; i++)
                {
                    for (int j = 0; j < blackCol; j++)
                    {
                        cout << image_processor->imgdata.color.cblack[6 + i*blackCol + j] << "  ";
                    }
                    cout << endl;
                }
            }

            //get white saturation values
            cout << "data_maximum: " << image_processor->imgdata.color.data_maximum << endl;
            cout << "maximum: " << image_processor->imgdata.color.maximum << endl;
            maxValue = image_processor->imgdata.color.maximum;
            cout << "fmaximum: " << image_processor->imgdata.color.fmaximum << endl;
            cout << "fnorm: " << image_processor->imgdata.color.fnorm << endl;

            //get color filter array
            //if all the image_processor.imgdata.idata.xtrans values are 0, it's bayer.
            //bayer only for now
            for (unsigned int i=0; i<2; i++)
            {
                for (unsigned int j=0; j<2; j++)
                {
                    cfa[i][j] = unsigned(image_processor->COLOR(int(i), int(j)));
                    if (cfa[i][j] == 3) //Auto CA correct doesn't like 0123 for RGBG; we change it to 0121.
                    {
                        cfa[i][j] = 1;
                    }
                }
            }

            //get xtrans color filter array
            maxXtrans = 0;
            for (int i=0; i<6; i++)
            {
                cout << "xtrans: ";
                for (int j=0; j<6; j++)
                {
                    xtrans[i][j] = image_processor->imgdata.idata.xtrans[i][j];
                    maxXtrans = max(maxXtrans,int(image_processor->imgdata.idata.xtrans[i][j]));
                    cout << xtrans[i][j];
                }
                cout << endl;
            }

            auto image = Exiv2::ImageFactory::open(loadParam.fullFilename);
            assert(image.get() != 0);
            image->readMetadata();
            exifData = image->exifData();


            raw_image.set_size(raw_height, raw_width);

            //copy raw data
            for (int row = 0; row < raw_height; row++)
            {
                //IMAGE is an (width*height) by 4 array, not width by height by 4.
                int rowoffset = (row + topmargin)*full_width;
                for (int col = 0; col < raw_width; col++)
                {
                    float tempBlackpoint = blackpoint;
                    if (blackRow > 0 && blackCol > 0)
                    {
                        tempBlackpoint = tempBlackpoint + image_processor->imgdata.color.cblack[6 + (row%blackRow)*blackCol + col%blackCol];
                    }
                    raw_image[row][col] = RAW[rowoffset + col + leftmargin] - tempBlackpoint;
                }
            }

//            cout << "max of raw_image" << raw_image.max() << endl;
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
        std::tie(valid, abort, loadParam, demosaicParam) = paramManager->claimDemosaicParams();
        if (abort == AbortStatus::restart)
        {
            cout << "imagePipeline.cpp: aborted at demosaic" << endl;
            return emptyMatrix();
        }

        cout << "imagePipeline.cpp: Opening " << loadParam.fullFilename << endl;

        //Reads in the photo.
        cout << "load start:" << timeDiff (timeRequested) << endl;
        struct timeval imload_time;
        gettimeofday( &imload_time, nullptr );

        if ((HighQuality == quality) && stealData)//only full pipelines may steal data
        {
            scaled_image = stealVictim->input_image;
            exifData = stealVictim->exifData;
            rCamMul = stealVictim->rCamMul;
            gCamMul = stealVictim->gCamMul;
            bCamMul = stealVictim->bCamMul;
            rPreMul = stealVictim->rPreMul;
            gPreMul = stealVictim->gPreMul;
            bPreMul = stealVictim->bPreMul;
            maxValue = stealVictim->maxValue;
            raw_width = stealVictim->raw_width;
            raw_height = stealVictim->raw_height;
            exifData = stealVictim->exifData;
            //copy color matrix
            //get color matrix
            for (int i = 0; i < 3; i++)
            {
                cout << "camToRGB: ";
                for (int j = 0; j < 3; j++)
                {
                    camToRGB[i][j] = stealVictim->camToRGB[i][j];
                }
                cout << endl;
            }
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
            matrix<float>   red(raw_height, raw_width);
            matrix<float> green(raw_height, raw_width);
            matrix<float>  blue(raw_height, raw_width);

            double initialGain = 1.0;
            float inputscale = maxValue;
            float outputscale = 65535.0;
            int border = 4;
            std::function<bool(double)> setProg = [](double) -> bool {return false;};

            cout << "raw width:  " << raw_width << endl;
            cout << "raw height: " << raw_height << endl;

            if (maxXtrans > 0)
            {
                librtprocess::markesteijn_demosaic(raw_width, raw_height, raw_image, red, green, blue, xtrans, camToRGB4, setProg, 3, true);
            }
            else
            {
                if (demosaicParam.caEnabled)
                {
                    matrix<float> raw_fixed(raw_height, raw_width);
                    CaFitParams fitparams;
                    librtprocess::CA_correct(0, 0, raw_width, raw_height, true, 1, 0.0, 0.0, true, raw_image, raw_fixed, cfa, setProg, fitparams, false);
                    librtprocess::amaze_demosaic(raw_width, raw_height, 0, 0, raw_width, raw_height, raw_fixed, red, green, blue, cfa, setProg, initialGain, border, inputscale, outputscale);
                } else {
                    librtprocess::amaze_demosaic(raw_width, raw_height, 0, 0, raw_width, raw_height, raw_image, red, green, blue, cfa, setProg, initialGain, border, inputscale, outputscale);
                }
            }

            input_image.set_size(raw_height, raw_width*3);
            for (int row = 0; row < raw_height; row++)
            {
                for (int col = 0; col < raw_width; col++)
                {
                    input_image(row, col*3    ) =   red(row, col);
                    input_image(row, col*3 + 1) = green(row, col);
                    input_image(row, col*3 + 2) =  blue(row, col);
                }
            }
        }
        cout << "load time: " << timeDiff(imload_time) << endl;

        cout << "ImagePipeline::processImage: Demosaic complete." << endl;

        if (LowQuality == quality)
        {
            cout << "scale start:" << timeDiff (timeRequested) << endl;
            struct timeval downscale_time;
            gettimeofday( &downscale_time, nullptr );
            downscale_and_crop(input_image,scaled_image, 0, 0, (input_image.nc()/3)-1,input_image.nr()-1, 600, 600);
            cout << "scale end: " << timeDiff( downscale_time ) << endl;
        }
        else if (PreviewQuality == quality)
        {
            cout << "scale start:" << timeDiff (timeRequested) << endl;
            struct timeval downscale_time;
            gettimeofday( &downscale_time, nullptr );
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

        //Here we apply the exposure compensation and white balance and color conversion matrix.
        matrix<float> exposureImage = scaled_image * pow(2, prefilmParam.exposureComp);
        whiteBalance(exposureImage,
                     pre_film_image,
                     prefilmParam.temperature,
                     prefilmParam.tint,
                     camToRGB,
                     rCamMul, gCamMul, bCamMul,
                     rPreMul, gPreMul, bPreMul);

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
        cout << "before rotation" << endl;

        rotate_image(filmulated_image,
                     rotated_image,
                     blackWhiteParam.rotation);
        cout << "after rotation" << endl;

        if (NoCache == cache)// clean up ram that's not needed anymore in order to reduce peak consumption
        {
            filmulated_image.set_size(0, 0);
            cacheEmpty = true;
        }
        else
        {
            cacheEmpty = false;
        }

        cout << "before crop calculations" << endl;

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

        cout << "before crop" << endl;

        matrix<float> cropped_image;

        downscale_and_crop(rotated_image,
                           cropped_image,
                           startX,
                           startY,
                           endX,
                           endY,
                           width,
                           height);

        cout << "after crop" << endl;

        rotated_image.set_size(0, 0);// clean up ram that's not needed anymore

        cout << "before whitepoint blackpoint" << endl;

        whitepoint_blackpoint(cropped_image,//filmulated_image,
                              contrast_image,
                              blackWhiteParam.whitepoint,
                              blackWhiteParam.blackpoint);

        cout << "after whitepoint blackpoint" << endl;

        valid = paramManager->markBlackWhiteComplete();
        updateProgress(valid, 0.0f);
        [[fallthrough]];
    }
    case partcolorcurve: [[fallthrough]];
    case blackwhite: // Do color_curve
    {
        cout << "before colorcurve" << endl;
        //It's not gonna abort because we have no color curves yet..
        //Prepare LUT's for individual color processin.g
        lutR.setUnity();
        lutG.setUnity();
        lutB.setUnity();
        cout << "after colorcurve luts before colorcurve" << endl;
        colorCurves(contrast_image,
                    color_curve_image,
                    lutR,
                    lutG,
                    lutB);

        cout << "after colorcurve" << endl;

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

        cout << "before curve LUT" << endl;

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
        cout << "before filmlikecurve" << endl;
        matrix<unsigned short> film_curve_image;
        film_like_curve(color_curve_image,
                        film_curve_image,
                        filmLikeLUT);
        cout << "after filmlikecurve" << endl;

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

        cout << "before vibrancesaturation" << endl;
        vibrance_saturation(film_curve_image,
                            vibrance_saturation_image,
                            curvesParam.vibrance,
                            curvesParam.saturation);
        cout << "after vibrancesaturation" << endl;

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
