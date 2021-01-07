#include "imagePipeline.h"
#include "../database/exifFunctions.h"
#include "../database/camconst.h"
#include <QDir>
#include <QStandardPaths>

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

matrix<unsigned short>& ImagePipeline::processImage(ParameterManager * paramManager,
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

    isCR3 = false;

    cout << "ImagePipeline::processImage valid: " << valid << endl;

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
            cout << "ImagePipeline::processImage: aborted at the start" << endl;
            return emptyMatrix();
        }

        isCR3 = QString::fromStdString(loadParam.fullFilename).endsWith(".cr3", Qt::CaseInsensitive);
        const bool isDNG = QString::fromStdString(loadParam.fullFilename).endsWith(".dng", Qt::CaseInsensitive);
        if (isCR3)
        {
            cout << "processImage this is a CR3!" << endl;
        }

        if (!loadParam.tiffIn && !loadParam.jpegIn && !((HighQuality == quality) && stealData))
        {
            std::unique_ptr<LibRaw> libraw = unique_ptr<LibRaw>(new LibRaw());

            //Open the file.
            int libraw_error;
#if (defined(_WIN32) || defined(__WIN32__))
            const QString tempFilename = QString::fromStdString(loadParam.fullFilename);
            std::wstring wstr = tempFilename.toStdWString();
            libraw_error = libraw->open_file(wstr.c_str());
#else
            const char *cstr = loadParam.fullFilename.c_str();
            libraw_error = libraw->open_file(cstr);
#endif
            if (libraw_error)
            {
                cout << "processImage: Could not read input file!" << endl;
                cout << "libraw error text: " << libraw_strerror(libraw_error) << endl;
                return emptyMatrix();
            }

             //Make abbreviations for brevity in accessing data.
#define RSIZE libraw->imgdata.sizes
#define PARAM libraw->imgdata.params
#define IMAGE libraw->imgdata.image
#define RAW   libraw->imgdata.rawdata.raw_image
#define RAW3  libraw->imgdata.rawdata.color3_image
#define RAW4  libraw->imgdata.rawdata.color4_image
#define RAWF  libraw->imgdata.rawdata.float_image
#define IDATA libraw->imgdata.idata
#define LENS  libraw->imgdata.lens
#define MAKER libraw->imgdata.lens.makernotes
#define OTHER libraw->imgdata.other
#define SIZES libraw->imgdata.sizes

            if (libraw->is_floating_point())
            {
                cout << "processImage: libraw cannot open a floating point raw" << endl;
                //LibRaw cannot process floating point images unless compiled with the DNG SDK.
                return emptyMatrix();
            }
            //This makes IMAGE contains the sensel value and 3 blank values at every
            //location.
            libraw_error = libraw->unpack();
            if (libraw_error)
            {
                cout << "processImage: Could not read input file, or was canceled" << endl;
                cout << "libraw error text: " << libraw_strerror(libraw_error) << endl;
                return emptyMatrix();
            }

            //get dimensions
            raw_width  = RSIZE.width;
            raw_height = RSIZE.height;
            cout << "raw width:  " << raw_width << endl;
            cout << "raw height: " << raw_height << endl;

            int topmargin = RSIZE.top_margin;
            int leftmargin = RSIZE.left_margin;
            int full_width = RSIZE.raw_width;
            //int full_height = RSIZE.raw_height;

            //get color matrix
            for (int i = 0; i < 3; i++)
            {
                //cout << "camToRGB: ";
                for (int j = 0; j < 3; j++)
                {
                    camToRGB[i][j] = libraw->imgdata.color.rgb_cam[i][j];
                    //cout << camToRGB[i][j] << " ";
                }
                //cout << endl;
            }
            for (int i = 0; i < 3; i++)
            {
                //cout << "camToRGB4: ";
                for (int j = 0; j < 4; j++)
                {
                    camToRGB4[i][j] = libraw->imgdata.color.rgb_cam[i][j];
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
                    //cout << camToRGB4[i][j] << " ";
                }
                //cout << endl;
            }
            rCamMul = libraw->imgdata.color.cam_mul[0];
            gCamMul = libraw->imgdata.color.cam_mul[1];
            bCamMul = libraw->imgdata.color.cam_mul[2];
            float minMult = min(min(rCamMul, gCamMul), bCamMul);
            rCamMul /= minMult;
            gCamMul /= minMult;
            bCamMul /= minMult;
            rPreMul = libraw->imgdata.color.pre_mul[0];
            gPreMul = libraw->imgdata.color.pre_mul[1];
            bPreMul = libraw->imgdata.color.pre_mul[2];
            minMult = min(min(rPreMul, gPreMul), bPreMul);
            rPreMul /= minMult;
            gPreMul /= minMult;
            bPreMul /= minMult;

            //get black subtraction values
            //for everything
            float blackpoint = libraw->imgdata.color.black;
            //some cameras have individual color channel subtraction. This hasn't been implemented yet.
            float rBlack = libraw->imgdata.color.cblack[0];
            float gBlack = libraw->imgdata.color.cblack[1];
            float bBlack = libraw->imgdata.color.cblack[2];
            float g2Black = libraw->imgdata.color.cblack[3];
            //Still others have a matrix to subtract.
            int blackRow = int(libraw->imgdata.color.cblack[4]);
            int blackCol = int(libraw->imgdata.color.cblack[5]);

            //cout << "BLACKPOINT" << endl;
            //cout << blackpoint << endl;
            //cout << "color channel blackpoints" << endl;
            //cout << rBlack << endl;
            //cout << gBlack << endl;
            //cout << bBlack << endl;
            //cout << g2Black << endl;
            //cout << "block-based blackpoint dimensions:" << endl;
            //cout << libraw->imgdata.color.cblack[4] << endl;
            //cout << libraw->imgdata.color.cblack[5] << endl;
            //cout << "block-based blackpoint: " << endl;
            uint maxBlockBlackpoint = 0;
            if (blackRow > 0 && blackCol > 0)
            {
                for (int i = 0; i < blackRow; i++)
                {
                    for (int j = 0; j < blackCol; j++)
                    {
                        maxBlockBlackpoint = max(maxBlockBlackpoint, libraw->imgdata.color.cblack[6 + i*blackCol + j]);
                        //cout << libraw->imgdata.color.cblack[6 + i*blackCol + j] << "  ";
                    }
                    //cout << endl;
                }
            }
            //cout << "Max of block-based blackpoint: " << maxBlockBlackpoint << endl;

            //get white saturation values
            cout << "WHITE SATURATION ===================================" << endl;
            cout << "data_maximum: " << libraw->imgdata.color.data_maximum << endl;
            cout << "maximum: " << libraw->imgdata.color.maximum << endl;

            //Calculate the white point based on the camera settings.
            //This needs the black point subtracted, and a fudge factor to ensure clipping is hard and fast.
            double whiteClippingPoint;
            QString makeModel = IDATA.make;
            makeModel.append(" ");
            makeModel.append(IDATA.model);
            camconst_status camconstStatus = camconst_read(makeModel, OTHER.iso_speed, OTHER.aperture, whiteClippingPoint);

            //Modern Nikons have camconst.json white levels specified as if they were 14-bit
            // even if the raw files are 12-bit-only, like the entry level cams
            //So we need to detect if it's 12-bit and if the camconst specifies as 14-bit.
            if ((QString(IDATA.make) == "Nikon") && (libraw->imgdata.color.maximum < 4096) && (whiteClippingPoint >= 4096))
            {
                whiteClippingPoint = whiteClippingPoint*4095/16383;
                cout << "Nikon 12-bit camconst white clipping point: " << whiteClippingPoint << endl;
            }
            cout << "is the file dng?: " << isDNG << endl;

            if (camconstStatus == CAMCONST_READ_OK && !isDNG) //dngs provide their own correct whitepoint
            {
                maxValue = whiteClippingPoint - blackpoint - maxBlockBlackpoint;
            } else {
                maxValue = libraw->imgdata.color.maximum - blackpoint - maxBlockBlackpoint;
            }
            cout << "black-subtracted maximum: " << maxValue << endl;
            cout << "fmaximum: " << libraw->imgdata.color.fmaximum << endl;
            cout << "fnorm: " << libraw->imgdata.color.fnorm << endl;

            //get color filter array
            //if all the libraw.imgdata.idata.xtrans values are 0, it's bayer.
            //bayer only for now
            for (unsigned int i=0; i<2; i++)
            {
                //cout << "bayer: ";
                for (unsigned int j=0; j<2; j++)
                {
                    cfa[i][j] = unsigned(libraw->COLOR(int(i), int(j)));
                    if (cfa[i][j] == 3) //Auto CA correct doesn't like 0123 for RGBG; we change it to 0121.
                    {
                        cfa[i][j] = 1;
                    }
                    //cout << cfa[i][j];
                }
                //cout << endl;
            }

            //get xtrans color filter array
            maxXtrans = 0;
            for (int i=0; i<6; i++)
            {
                //cout << "xtrans: ";
                for (int j=0; j<6; j++)
                {
                    xtrans[i][j] = uint(libraw->imgdata.idata.xtrans[i][j]);
                    maxXtrans = max(maxXtrans,int(libraw->imgdata.idata.xtrans[i][j]));
                    //cout << xtrans[i][j];
                }
                //cout << endl;
            }

            if (!isCR3)//we can't use exiv2 on CR3 yet
            {
                cout << "processImage exiv filename: " << loadParam.fullFilename << endl;
                auto image = Exiv2::ImageFactory::open(loadParam.fullFilename);
                assert(image.get() != 0);
                image->readMetadata();
                exifData = image->exifData();
            } else {
                //We need to fabricate fresh exif data from what libraw gives us
                Exiv2::ExifData basicExifData;

                basicExifData["Exif.Image.Orientation"] = uint16_t(1);
                basicExifData["Exif.Image.ImageWidth"] = vibrance_saturation_image.nc()/3;
                basicExifData["Exif.Image.ImageLength"] = vibrance_saturation_image.nr();
                basicExifData["Exif.Image.Make"] = IDATA.make;
                basicExifData["Exif.Image.Model"] = IDATA.model;
                basicExifData["Exif.Image.DateTime"] = exifDateTimeString(OTHER.timestamp);
                basicExifData["Exif.Photo.DateTimeOriginal"] = exifDateTimeString(OTHER.timestamp);
                basicExifData["Exif.Photo.DateTimeDigitized"] = exifDateTimeString(OTHER.timestamp);
                basicExifData["Exif.Photo.ExposureTime"] = rationalTv(OTHER.shutter);
                basicExifData["Exif.Photo.FNumber"] = rationalAvFL(OTHER.aperture);
                basicExifData["Exif.Photo.ISOSpeed"] = int(round(OTHER.iso_speed));
                basicExifData["Exif.Photo.FocalLength"] = rationalAvFL(OTHER.focal_len);

                exifData = basicExifData;
            }

            raw_image.set_size(raw_height, raw_width);

            //copy raw data
            float rawMin = std::numeric_limits<float>::max();
            float rawMax = std::numeric_limits<float>::min();

            isSraw = libraw->is_sraw();

            //Iridient X-Transformer creates full-color files that aren't sraw
            //They have 6666 as the cfa and all 0 for xtrans
            //However, Leica M Monochrom files are exactly the same!
            //So we have to check if the white balance tag exists.
            bool isWeird = (cfa[0][0]==6 && cfa[0][1]==6 && cfa[1][0]==6 && cfa[1][1]==6);
            //cout << "is weird: " << isWeird << endl;
            bool noWB = false;
            if (!isCR3)//we can't use exiv2 on CR3 yet and no CR3 cameras are monochrome
            {
                noWB = exifData["Exif.Photo.WhiteBalance"].toString().length()==0;
            }
            //cout << "white balance: " << wb << endl;
            isMonochrome = isWeird && noWB;
            //cout << "is monochrome: " << isMonochrome << endl;
            isSraw = isSraw || (isWeird && !isMonochrome);
            //cout << "is full color raw: " << isSraw << endl;


            isNikonSraw = libraw->is_nikon_sraw();
            if (isSraw)
            {
                raw_image.set_size(raw_height, raw_width*3);
                #pragma omp parallel for reduction (min:rawMin) reduction(max:rawMax)
                for (int row = 0; row < raw_height; row++)
                {
                    //IMAGE is an (width*height) by 4 array, not width by height by 4.
                    int rowoffset = (row + topmargin)*full_width;
                    for (int col = 0; col < raw_width; col++)
                    {
                        float tempBlackpoint = blackpoint;
                        if (blackRow > 0 && blackCol > 0)
                        {
                            tempBlackpoint = tempBlackpoint + libraw->imgdata.color.cblack[6 + (row%blackRow)*blackCol + col%blackCol];
                        }
                        //sraw comes from raw4 but only uses 3 channels
                        raw_image[row][col*3   ] = RAW4[rowoffset + col + leftmargin][0] - tempBlackpoint;
                        rawMin = std::min(rawMin, raw_image[row][col*3   ]);
                        rawMax = std::max(rawMax, raw_image[row][col*3   ]);
                        raw_image[row][col*3 +1] = RAW4[rowoffset + col + leftmargin][1] - tempBlackpoint;
                        rawMin = std::min(rawMin, raw_image[row][col*3 +1]);
                        rawMax = std::max(rawMax, raw_image[row][col*3 +1]);
                        raw_image[row][col*3 +2] = RAW4[rowoffset + col + leftmargin][2] - tempBlackpoint;
                        rawMin = std::min(rawMin, raw_image[row][col*3 +2]);
                        rawMax = std::max(rawMax, raw_image[row][col*3 +2]);
                    }
                }
            } else if (libraw->is_floating_point()){//we can't even get here until libraw supports floating point raw
                #pragma omp parallel for reduction (min:rawMin) reduction(max:rawMax)
                for (int row = 0; row < raw_height; row++)
                {
                    //IMAGE is an (width*height) by 4 array, not width by height by 4.
                    int rowoffset = (row + topmargin)*full_width;
                    for (int col = 0; col < raw_width; col++)
                    {
                        float tempBlackpoint = blackpoint;
                        if (blackRow > 0 && blackCol > 0)
                        {
                            tempBlackpoint = tempBlackpoint + libraw->imgdata.color.cblack[6 + (row%blackRow)*blackCol + col%blackCol];
                        }
                        raw_image[row][col] = RAWF[rowoffset + col + leftmargin] - tempBlackpoint;
                        rawMin = std::min(rawMin, raw_image[row][col]);
                        rawMax = std::max(rawMax, raw_image[row][col]);
                    }
                }
            } else {
                #pragma omp parallel for reduction (min:rawMin) reduction(max:rawMax)
                for (int row = 0; row < raw_height; row++)
                {
                    //IMAGE is an (width*height) by 4 array, not width by height by 4.
                    int rowoffset = (row + topmargin)*full_width;
                    for (int col = 0; col < raw_width; col++)
                    {
                        float tempBlackpoint = blackpoint;
                        if (blackRow > 0 && blackCol > 0)
                        {
                            tempBlackpoint = tempBlackpoint + libraw->imgdata.color.cblack[6 + (row%blackRow)*blackCol + col%blackCol];
                        }
                        raw_image[row][col] = RAW[rowoffset + col + leftmargin] - tempBlackpoint;
                        rawMin = std::min(rawMin, raw_image[row][col]);
                        rawMax = std::max(rawMax, raw_image[row][col]);
                    }
                }
            }

            //generate raw histogram
            if (WithHisto == histo)
            {
                histoInterface->updateHistRaw(raw_image, maxValue, cfa, xtrans, maxXtrans, isSraw, isMonochrome);
            }

            cout << "max of raw_image: " << rawMax << endl;
            cout << "min of raw_image: " << rawMin << endl;
        }
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

        matrix<float>& scaled_image = recovered_image;
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
            isSraw = stealVictim->isSraw;
            isNikonSraw = stealVictim->isNikonSraw;
            isMonochrome = stealVictim->isMonochrome;
            isCR3 = stealVictim->isCR3;
            raw_width = stealVictim->raw_width;
            raw_height = stealVictim->raw_height;
            //copy color matrix
            //get color matrix
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    camToRGB[i][j] = stealVictim->camToRGB[i][j];
                }
            }
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    camToRGB4[i][j] = stealVictim->camToRGB4[i][j];
                }
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
        else if (isSraw)//already demosaiced
        {
            //We just need to scale to 65535, and apply camera WB
            float inputscale = maxValue;
            float outputscale = 65535.0;
            float scaleFactor = outputscale / inputscale;
            input_image.set_size(raw_height, raw_width*3);
            if (isNikonSraw)
            {
                #pragma omp parallel for
                for (int row = 0; row < raw_height; row++)
                {
                    for (int col = 0; col < raw_width*3; col++)
                    {
                        int color = col % 3;
                        input_image(row, col) = raw_image(row, col) * scaleFactor;

                    }
                }
            }
            else
            {
                #pragma omp parallel for
                for (int row = 0; row < raw_height; row++)
                {
                    for (int col = 0; col < raw_width*3; col++)
                    {
                        int color = col % 3;
                        input_image(row, col) = raw_image(row, col) * scaleFactor * ((color==0) ? rCamMul : (color == 1) ? gCamMul : bCamMul);

                    }
                }
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
            const int border = 4;//used for amaze
            std::function<bool(double)> setProg = [](double) -> bool {return false;};

            cout << "raw width:  " << raw_width << endl;
            cout << "raw height: " << raw_height << endl;

            //before demosaic, you want to apply raw white balance
            //======================================================================
            //TODO: If the camera white balance disagrees with some sort of AWB by a *lot*, use an awb instead
            //======================================================================
            matrix<float> premultiplied(raw_height, raw_width);

            cout << "demosaic start" << timeDiff(timeRequested) << endl;
            struct timeval demosaic_time;
            gettimeofday(&demosaic_time, nullptr);

            if (maxXtrans > 0)
            {
                #pragma omp parallel for
                for (int row = 0; row < raw_height; row++)
                {
                    for (int col = 0; col < raw_width; col++)
                    {
                        uint color = xtrans[uint(row) % 6][uint(col) % 6];
                        premultiplied(row, col) = raw_image(row, col) * ((color==0) ? rCamMul : (color == 1) ? gCamMul : bCamMul);
                    }
                }
                markesteijn_demosaic(raw_width, raw_height, premultiplied, red, green, blue, xtrans, camToRGB4, setProg, 3, true);
                //there's no inputscale for markesteijn so we need to scale
                float scaleFactor = outputscale / inputscale;
                #pragma omp parallel for
                for (int row = 0; row < red.nr(); row++)
                {
                    for (int col = 0; col < red.nc(); col++)
                    {
                        red(row, col)   = red(row, col)   * scaleFactor;
                        green(row, col) = green(row, col) * scaleFactor;
                        blue(row, col)  = blue(row, col)  * scaleFactor;
                    }
                }
            }
            else if (isMonochrome)
            {
                float scaleFactor = outputscale / inputscale;
                for (int row = 0; row < raw_height; row++)
                {
                    for (int col = 0; col < raw_width; col++)
                    {
                        red(row, col)   = raw_image(row, col) * scaleFactor;
                        green(row, col) = raw_image(row, col) * scaleFactor;
                        blue(row, col)  = raw_image(row, col) * scaleFactor;
                    }
                }
            }
            else
            {
                #pragma omp parallel for
                for (int row = 0; row < raw_height; row++)
                {
                    for (int col = 0; col < raw_width; col++)
                    {
                        uint color = cfa[uint(row) & 1][uint(col) & 1];
                        premultiplied(row, col) = raw_image(row, col) * ((color==0) ? rCamMul : (color == 1) ? gCamMul : bCamMul);
                    }
                }
                if (demosaicParam.caEnabled > 0)
                {
                    //we need to apply white balance and then remove it for Auto CA Correct to work properly
                    double fitparams[2][2][16];
                    CA_correct(0, 0, raw_width, raw_height, true, demosaicParam.caEnabled, 0.0, 0.0, true, premultiplied, premultiplied, cfa, setProg, fitparams, false);
                }
                amaze_demosaic(raw_width, raw_height, 0, 0, raw_width, raw_height, premultiplied, red, green, blue, cfa, setProg, initialGain, border, inputscale, outputscale);
                //matrix<float> normalized_image(raw_height, raw_width);
                //normalized_image = premultiplied * (outputscale/inputscale);
                //lmmse_demosaic(raw_width, raw_height, normalized_image, red, green, blue, cfa, setProg, 3);//needs inputscale and output scale to be implemented
            }
            premultiplied.set_size(0, 0);
            cout << "demosaic end: " << timeDiff(demosaic_time) << endl;

            input_image.set_size(raw_height, raw_width*3);
            #pragma omp parallel for
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

        //Recover highlights now
        cout << "hlrecovery start:" << timeDiff (timeRequested) << endl;
        struct timeval hlrecovery_time;
        gettimeofday(&hlrecovery_time, nullptr);

        int height = scaled_image.nr();
        int width  = scaled_image.nc()/3;

        //Now, recover highlights.
        std::function<bool(double)> setProg = [](double) -> bool {return false;};
        //And return it back to a single layer
        if (demosaicParam.highlights >= 2)
        {
            recovered_image.set_size(height, width*3);
            //For highlight recovery, we need to split up the image into three separate layers.
            matrix<float> rChannel(height, width), gChannel(height, width), bChannel(height, width);

            #pragma omp parallel for
            for (int row = 0; row < height; row++)
            {
                for (int col = 0; col < width; col++)
                {
                    rChannel(row, col) = scaled_image(row, col*3    );
                    gChannel(row, col) = scaled_image(row, col*3 + 1);
                    bChannel(row, col) = scaled_image(row, col*3 + 2);
                }
            }

            //We applied the camMul camera multipliers before applying white balance.
            //Now we need to calculate the channel max and the raw clip levels.
            //Channel max:
            const float chmax[3] = {rChannel.max(), gChannel.max(), bChannel.max()};
            //Max clip point:
            const float clmax[3] = {65535.0f*rCamMul, 65535.0f*gCamMul, 65535.0f*bCamMul};

            HLRecovery_inpaint(width, height, rChannel, gChannel, bChannel, chmax, clmax, setProg);
            #pragma omp parallel for
            for (int row = 0; row < height; row++)
            {
                for (int col = 0; col < width; col++)
                {
                    recovered_image(row, col*3    ) = rChannel(row, col);
                    recovered_image(row, col*3 + 1) = gChannel(row, col);
                    recovered_image(row, col*3 + 2) = bChannel(row, col);
                }
            }
        } else if (demosaicParam.highlights == 0)
        {
            recovered_image.set_size(height, width*3);
            #pragma omp parallel for
            for (int row = 0; row < height; row++)
            {
                for (int col = 0; col < width; col++)
                {
                    recovered_image(row, col*3    ) = min(scaled_image(row, col*3    ), 65535.0f);
                    recovered_image(row, col*3 + 1) = min(scaled_image(row, col*3 + 1), 65535.0f);
                    recovered_image(row, col*3 + 2) = min(scaled_image(row, col*3 + 2), 65535.0f);
                }
            }
        } else {
            recovered_image = std::move(scaled_image);
        }

        //Lensfun processing
        cout << "lensfun start" << endl;
        lfDatabase *ldb = lf_db_create();
        QDir dir = QDir::home();
        QString dirstr = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        dirstr.append("/filmulator/version_2");
        std::string stdstring = dirstr.toStdString();
        ldb->Load(stdstring.c_str());

        std::string camName = demosaicParam.cameraName.toStdString();
        const lfCamera * camera = NULL;
        const lfCamera ** cameraList = ldb->FindCamerasExt(NULL,camName.c_str());

        //Set up stuff for rotation.
        //We expect rotation to be from -45 to +45
        //But -50 will be the signal from the UI to disable it.
        float rotationAngle = demosaicParam.rotationAngle * 3.1415926535/180;//convert degrees to radians
        if (demosaicParam.rotationAngle <= -49) {
            rotationAngle = 0;
        }
        cout << "cos rotationangle: " << cos(rotationAngle) << endl;
        cout << "sin rotationangle: " << sin(rotationAngle) << endl;
        bool lensfunGeometryCorrectionApplied = false;

        if (cameraList)
        {
            const float cropFactor = cameraList[0]->CropFactor;

            QString tempLensName = demosaicParam.lensName;
            if (tempLensName.length() > 0)
            {
                if (tempLensName.front() == "\\")
                {
                    //if the lens name starts with a backslash, don't filter by camera
                    tempLensName.remove(0,1);
                } else {
                    //if it doesn't start with a backslash, filter by camera
                    camera = cameraList[0];
                }
            }
            std::string lensName = tempLensName.toStdString();
            const lfLens * lens = NULL;
            const lfLens ** lensList = NULL;
            lensList = ldb->FindLenses(camera, NULL, lensName.c_str());
            if (lensList)
            {
                lens = lensList[0];

                //Now we set up the modifier itself with the lens and processing flags
#ifdef LF_GIT
                lfModifier * mod = new lfModifier(lens, demosaicParam.focalLength, cropFactor, width, height, LF_PF_F32);
#else //lensfun v0.3.95
                lfModifier * mod = new lfModifier(cropFactor, width, height, LF_PF_F32);
#endif

                int modflags = 0;
                if (demosaicParam.lensfunCA && !isMonochrome)
                {
#ifdef LF_GIT
                    modflags |= mod->EnableTCACorrection();
#else //lensfun v0.3.95
                    modflags |= mod->EnableTCACorrection(lens, demosaicParam.focalLength);
#endif
                }
                if (demosaicParam.lensfunVignetting)
                {
#ifdef LF_GIT
                    modflags |= mod->EnableVignettingCorrection(demosaicParam.fnumber, 1000.0f);
#else //lensfun v0.3.95
                    modflags |= mod->EnableVignettingCorrection(lens, demosaicParam.focalLength, demosaicParam.fnumber, 1000.0f);
#endif
                }
                if (demosaicParam.lensfunDistortion)
                {
#ifdef LF_GIT
                    modflags |= mod->EnableDistortionCorrection();
#else //lensfun v0.3.95
                    modflags |= mod->EnableDistortionCorrection(lens, demosaicParam.focalLength);
#endif
                    modflags |= mod->EnableScaling(mod->GetAutoScale(false));
                    cout << "Auto scale factor: " << mod->GetAutoScale(false) << endl;
                }

                //Now we actually perform the required processing.
                //First is vignetting.
                if (demosaicParam.lensfunVignetting)
                {
                    bool success = true;
                    #pragma omp parallel for
                    for (int row = 0; row < height; row++)
                    {
                        success = mod->ApplyColorModification(recovered_image[row], 0.0f, row, width, 1, LF_CR_3(RED, GREEN, BLUE), width);
                    }
                }

                //Next is CA, or distortion, or both.
                matrix<float> new_image;
                new_image.set_size(height, width*3);

                if (demosaicParam.lensfunCA || demosaicParam.lensfunDistortion)
                {
                    //ApplySubpixelGeometryDistortion
                    lensfunGeometryCorrectionApplied = true;
                    bool success = true;
                    int listWidth = width * 2 * 3;

                    //Check how far out of bounds we go
                    float maxOvershootDistance = 1.0f;
                    float semiwidth = (width-1)/2.0f;
                    float semiheight = (height-1)/2.0f;
                    #pragma omp parallel for reduction(max:maxOvershootDistance)
                    for (int row = 0; row < height; row++)
                    {
                        float positionList[listWidth];
                        success = mod->ApplySubpixelGeometryDistortion(0.0f, row, width, 1, positionList);
                        if (success)
                        {
                            for (int col = 0; col < width; col++)
                            {
                                int listIndex = col * 2 * 3; //list index
                                for (int c = 0; c < 3; c++)
                                {
                                    float coordX = positionList[listIndex+2*c] - semiwidth;
                                    float coordY = positionList[listIndex+2*c+1] - semiheight;
                                    float rotatedX = coordX * cos(rotationAngle) - coordY * sin(rotationAngle);
                                    float rotatedY = coordX * sin(rotationAngle) + coordY * cos(rotationAngle);

                                    float overshoot = 1.0f;

                                    if (abs(rotatedX) > semiwidth)
                                    {
                                        overshoot = max(abs(rotatedX)/semiwidth,overshoot);
                                    }
                                    if (abs(rotatedY) > semiheight)
                                    {
                                        overshoot = max(abs(rotatedY)/semiheight,overshoot);
                                    }

                                    if (overshoot > maxOvershootDistance)
                                    {
                                        maxOvershootDistance = overshoot;
                                    }
                                }
                            }
                        }
                    }

                    #pragma omp parallel for
                    for (int row = 0; row < height; row++)
                    {
                        float positionList[listWidth];
                        success = mod->ApplySubpixelGeometryDistortion(0.0f, row, width, 1, positionList);
                        if (success)
                        {
                            for (int col = 0; col < width; col++)
                            {
                                int listIndex = col * 2 * 3; //list index
                                for (int c = 0; c < 3; c++)
                                {
                                    float coordX = positionList[listIndex+2*c] - semiwidth;
                                    float coordY = positionList[listIndex+2*c+1] - semiheight;
                                    float rotatedX = (coordX * cos(rotationAngle) - coordY * sin(rotationAngle)) / maxOvershootDistance + semiwidth;
                                    float rotatedY = (coordX * sin(rotationAngle) + coordY * cos(rotationAngle)) / maxOvershootDistance + semiheight;
                                    int sX = max(0, min(width-1,  int(floor(rotatedX))))*3 + c;//startX
                                    int eX = max(0, min(width-1,  int(ceil(rotatedX))))*3 + c; //endX
                                    int sY = max(0, min(height-1, int(floor(rotatedY))));      //startY
                                    int eY = max(0, min(height-1, int(ceil(rotatedY))));       //endY
                                    float notUsed;
                                    float eWX = modf(rotatedX, &notUsed); //end weight X
                                    float eWY = modf(rotatedY, &notUsed); //end weight Y;
                                    float sWX = 1 - eWX;                //start weight X
                                    float sWY = 1 - eWY;                //start weight Y;
                                    new_image(row, col*3 + c) = recovered_image(sY, sX) * sWY * sWX +
                                                                recovered_image(eY, sX) * eWY * sWX +
                                                                recovered_image(sY, eX) * sWY * eWX +
                                                                recovered_image(eY, eX) * eWY * eWX;
                                }
                            }
                        }
                    }
                    recovered_image = std::move(new_image);
                }

                if (mod != NULL)
                {
                    delete mod;
                }
            }
            lf_free(lensList);
        }
        lf_free(cameraList);

        //cleanup lensfun
        if (ldb != NULL)
        {
            lf_db_destroy(ldb);
        }

        if (!lensfunGeometryCorrectionApplied)
        {
            //also do rotations on non-corrected images
            float maxOvershootDistance = 1.0f;
            float semiwidth = (width-1)/2.0f;
            float semiheight = (height-1)/2.0f;

            //check the four corners
            for (int row = 0; row < height; row += height-1)
            {
                for (int col = 0; col < width; col += width-1)
                {
                    float coordX = col - semiwidth;
                    float coordY = row - semiheight;
                    float rotatedX = coordX * cos(rotationAngle) - coordY * sin(rotationAngle);
                    float rotatedY = coordX * sin(rotationAngle) + coordY * cos(rotationAngle);

                    float overshoot = 1.0f;

                    if (abs(rotatedX) > semiwidth)
                    {
                        overshoot = max(abs(rotatedX)/semiwidth,overshoot);
                    }
                    if (abs(rotatedY) > semiheight)
                    {
                        overshoot = max(abs(rotatedY)/semiheight,overshoot);
                    }

                    if (overshoot > maxOvershootDistance)
                    {
                        maxOvershootDistance = overshoot;
                    }
                }
            }

            //Apply the rotation
            matrix<float> new_image;
            new_image.set_size(height, width*3);

            for (int row = 0; row < height; row++)
            {
                for (int col = 0; col < width; col++)
                {
                    float coordX = col - semiwidth;
                    float coordY = row - semiheight;
                    float rotatedX = (coordX * cos(rotationAngle) - coordY * sin(rotationAngle)) / maxOvershootDistance + semiwidth;
                    float rotatedY = (coordX * sin(rotationAngle) + coordY * cos(rotationAngle)) / maxOvershootDistance + semiheight;
                    int sX = max(0, min(width-1,  int(floor(rotatedX))))*3;//startX
                    int eX = max(0, min(width-1,  int(ceil(rotatedX))))*3; //endX
                    int sY = max(0, min(height-1, int(floor(rotatedY))));  //startY
                    int eY = max(0, min(height-1, int(ceil(rotatedY))));   //endY
                    float notUsed;
                    float eWX = modf(rotatedX, &notUsed); //end weight X
                    float eWY = modf(rotatedY, &notUsed); //end weight Y;
                    float sWX = 1 - eWX;                //start weight X
                    float sWY = 1 - eWY;                //start weight Y;
                    for (int c = 0; c < 3; c++)
                    {
                        new_image(row, col*3 + c) = recovered_image(sY, sX + c) * sWY * sWX +
                                                    recovered_image(eY, sX + c) * eWY * sWX +
                                                    recovered_image(sY, eX + c) * sWY * eWX +
                                                    recovered_image(eY, eX + c) * eWY * eWX;
                    }
                }
            }
            recovered_image = std::move(new_image);
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
        whiteBalance(recovered_image,
                     pre_film_image,
                     prefilmParam.temperature,
                     prefilmParam.tint,
                     camToRGB,
                     rCamMul, gCamMul, bCamMul,//needed as a reference but not actually applied
                     rPreMul, gPreMul, bPreMul,
                     65535.0f, pow(2, prefilmParam.exposureComp));

        if (NoCache == cache)
        {
            recovered_image.set_size( 0, 0 );
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
            histoInterface->updateHistPostFilm(filmulated_image, .0025f);//TODO connect this magic number to the qml
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
        int width  = int(round(min(tempHeight*tempAspect,float(imWidth))));
        int height = int(round(min(tempHeight, imWidth/tempAspect)));
        const float maxHoffset = (1.0f-(float(width)  / float(imWidth) ))/2.0f;
        const float maxVoffset = (1.0f-(float(height) / float(imHeight)))/2.0f;
        const float oddH = (!(int(round((imWidth  - width )/2.0))*2 == (imWidth  - width )))*0.5f;//it's 0.5 if it's odd, 0 otherwise
        const float oddV = (!(int(round((imHeight - height)/2.0))*2 == (imHeight - height)))*0.5f;//it's 0.5 if it's odd, 0 otherwise
        const float hoffset = (round(max(min(blackWhiteParam.cropHoffset, maxHoffset), -maxHoffset) * imWidth  + oddH) - oddH)/imWidth;
        const float voffset = (round(max(min(blackWhiteParam.cropVoffset, maxVoffset), -maxVoffset) * imHeight + oddV) - oddV)/imHeight;
        int startX = int(round(0.5f*(imWidth  - width ) + hoffset*imWidth));
        int startY = int(round(0.5f*(imHeight - height) + voffset*imHeight));
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
        cout << "crop start:" << timeDiff (timeRequested) << endl;
        struct timeval crop_time;
        gettimeofday(&crop_time, nullptr);

        downscale_and_crop(rotated_image,
                           cropped_image,
                           startX,
                           startY,
                           endX,
                           endY,
                           width,
                           height);

        cout << "crop end: " << timeDiff(crop_time) << endl;

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
                float shResult = shadows_highlights(float(in)/65535.0f,
                                                     curvesParam.shadowsX,
                                                     curvesParam.shadowsY,
                                                     curvesParam.highlightsX,
                                                     curvesParam.highlightsY);
                return ushort(65535*default_tonecurve(shResult));
            }
        );
        matrix<unsigned short>& film_curve_image = vibrance_saturation_image;
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

        if (!curvesParam.monochrome)
        {
            vibrance_saturation(film_curve_image,
                                vibrance_saturation_image,
                                curvesParam.vibrance,
                                curvesParam.saturation);
        } else {
            monochrome_convert(film_curve_image,
                               vibrance_saturation_image,
                               curvesParam.bwRmult,
                               curvesParam.bwGmult,
                               curvesParam.bwBmult);
        }

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
    for (ulong i = 0; i < completionTimes.size(); i++)
    {
        totalTime += completionTimes[i];
        float fractionCompleted = 0;
        if (i <= valid)
            fractionCompleted = 1;
        if (i == valid + 1)
            fractionCompleted = stepProgress;
        //if greater -> 0
        totalCompletedTime += completionTimes[i]*double(fractionCompleted);
    }
    histoInterface->setProgress(float(totalCompletedTime/totalTime));
}

//Do not call this on something that's already been used!
void ImagePipeline::setCache(Cache cacheIn)
{
    if (false == hasStartedProcessing)
    {
        cache = cacheIn;
    }
}

//This swaps the data between pipelines.
//The intended use is for preloading.
void ImagePipeline::swapPipeline(ImagePipeline * swapTarget)
{
    std::swap(valid, swapTarget->valid);
    std::swap(progress, swapTarget->progress);

    raw_image.swap(swapTarget->raw_image);

    std::swap(cfa, swapTarget->cfa);
    std::swap(xtrans, swapTarget->xtrans);
    maxXtrans = swapTarget->maxXtrans;

    raw_width = swapTarget->raw_width;
    raw_height = swapTarget->raw_height;

    std::swap(camToRGB, swapTarget->camToRGB);
    std::swap(camToRGB4, swapTarget->camToRGB4);

    std::swap(rCamMul, swapTarget->rCamMul);
    std::swap(gCamMul, swapTarget->gCamMul);
    std::swap(bCamMul, swapTarget->bCamMul);
    std::swap(rPreMul, swapTarget->rPreMul);
    std::swap(gPreMul, swapTarget->gPreMul);
    std::swap(bPreMul, swapTarget->bPreMul);

    std::swap(maxValue, swapTarget->maxValue);
    std::swap(isSraw, swapTarget->isSraw);
    std::swap(isNikonSraw, swapTarget->isNikonSraw);
    std::swap(isMonochrome, swapTarget->isMonochrome);
    std::swap(isCR3, swapTarget->isCR3);

    std::swap(exifData, swapTarget->exifData);
    std::swap(basicExifData, swapTarget->basicExifData);

    input_image.swap(swapTarget->input_image);
    recovered_image.swap(swapTarget->recovered_image);
    pre_film_image.swap(swapTarget->pre_film_image);
    filmulated_image.swap(swapTarget->filmulated_image);
    contrast_image.swap(swapTarget->contrast_image);
    color_curve_image.swap(swapTarget->color_curve_image);
    vibrance_saturation_image.swap(swapTarget->vibrance_saturation_image);
}

//This is used to copy only images from one pipeline to another,
// but downsampling to the set resolution.
//The intended use is for improving the quality of the quick preview
// in the case of distortion correction or leveling.
void ImagePipeline::copyAndDownsampleImages(ImagePipeline * copySource)
{
    //We only want to copy stuff starting with recovered image.
    downscale_and_crop(copySource->recovered_image, recovered_image, 0, 0, ((copySource->recovered_image.nc())/3)-1, copySource->recovered_image.nr()-1, resolution, resolution);
    downscale_and_crop(copySource->pre_film_image, pre_film_image, 0, 0, ((copySource->pre_film_image.nc())/3)-1, copySource->pre_film_image.nr()-1, resolution, resolution);
    downscale_and_crop(copySource->filmulated_image, filmulated_image, 0, 0, ((copySource->filmulated_image.nc())/3)-1, copySource->filmulated_image.nr()-1, resolution, resolution);
    //The stuff after filmulated_image is type <unsigned short> and so
    // we don't have a routine to scale them. But that's okay, I think.
    //Anything except tweaking saturation will pull from the higher res
    // data.
}

//This is used to update the histograms once data is copied on an image change
void ImagePipeline::rerunHistograms()
{
    if (WithHisto == histo)
    {
        if (valid >= Valid::load)
        {
            histoInterface->updateHistRaw(raw_image, maxValue, cfa, xtrans, maxXtrans, isSraw, isMonochrome);
        }
        if (valid >= Valid::prefilmulation)
        {
            histoInterface->updateHistPreFilm(pre_film_image, 65535);
        }
        if (valid >= Valid::filmulation)
        {
            histoInterface->updateHistPostFilm(filmulated_image, .0025f);
        }
        if (valid >= Valid::filmlikecurve)
        {
            histoInterface->updateHistFinal(vibrance_saturation_image);
        }
    }
}
