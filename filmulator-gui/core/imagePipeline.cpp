#include "imagePipeline.h"
#include "filmSim.hpp"
#include "../database/exifFunctions.h"
#include "../database/camconst.h"
#include <QDir>
#include <QStandardPaths>
#include "nlmeans/nlmeans.hpp"
#include "rawtherapee/rt_routines.h"

ImagePipeline::ImagePipeline(Cache cacheIn, Histo histoIn, QuickQuality qualityIn)
{
    cache = cacheIn;
    histo = histoIn;
    quality = qualityIn;
    valid = Valid::none;
    filename = "";

    completionTimes.resize(Valid::count);
    completionTimes[Valid::none] = 0;
    completionTimes[Valid::load] = 5;
    completionTimes[Valid::demosaic] = 30;
    completionTimes[Valid::postdemosaic] = 10;
    completionTimes[Valid::nrnlmeans] = 20;
    completionTimes[Valid::nrimpulse] = 10;
    completionTimes[Valid::nrchroma] = 20;
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
                                                    Exiv2::ExifData &exifOutput,
                                                    const QString fileHash,//make this empty string if you don't want to mess around with validity
                                                    ImagePipeline * stealVictim)//defaults to nullptr
{
    //Say that we've started processing to prevent cache status from changing..
    hasStartedProcessing = true;
    //Record when the function was requested. This is so that the function will not give up
    // until a given short time has elapsed.
    gettimeofday(&timeRequested, nullptr);
    histoInterface = interface_in;

    //check that file requested matches the file associated with the parameter manager
    if (fileHash != "")
    {
        QString paramIndex = paramManager->getImageIndex();
        paramIndex.truncate(32);
        if (fileHash != paramIndex)
        {
            cout << "processImage shuffle mismatch:  Requested Index: " << fileHash.toStdString() << endl;
            cout << "processImage shuffle mismatch:  Parameter Index: " << paramIndex.toStdString() << endl;
            cout << "processImage shuffle mismatch:  full pipeline?: " << (quality == HighQuality) << endl;
            valid = none;
        }
        fileID = paramIndex;
    } else {
        QString paramIndex = paramManager->getImageIndex();
        paramIndex.truncate(32);
        fileID = paramIndex;
    }

    valid = paramManager->getValid();
    if (NoCache == cache || true == cacheEmpty)
    {
        valid = none;//we need to start fresh if nothing is going to be cached.
    }

    //If we are a high-res pipeline that's going to steal data, skip to filmulation
    if (stealData)
    {
        if (stealVictim == nullptr)
        {
            cout << "stealVictim should not be null!" << endl;
        }
        valid = max(valid, prefilmulation);
        paramManager->setValid(valid);
    }

    //If we think we have valid image data,
    //check that the file last processed corresponds to the one requested.
    if (valid > none && !stealData)
    {
        //if something has been processed before, and we think it's valid
        //it had better be the same filename.
        if (paramManager->getFullFilename() != filename.toStdString())
        {
            cout << "processImage paramManager filename doesn't match pipeline filename" << endl;
            cout << "processImage paramManager filename: " << paramManager->getFullFilename() << endl;
            cout << "processImage pipeline filename:     " << filename.toStdString() << endl;
            cout << "processImage setting validity to none due to filename" << endl;
            valid = none;
        }
    }

    cout << "ImagePipeline::processImage valid: " << valid << endl;

    updateProgress(valid, 0.0f);
    switch (valid)
    {
    case partload: [[fallthrough]];
    case none://Load image into buffer
    {
        LoadParams loadParam;
        AbortStatus abort;
        //See whether to abort or not, while grabbing the latest parameters.
        std::tie(valid, abort, loadParam) = paramManager->claimLoadParams();
        if (abort == AbortStatus::restart)
        {
            cout << "ImagePipeline::processImage: aborted at the start" << endl;
            return emptyMatrix();
        }

        filename = QString::fromStdString(loadParam.fullFilename);

        isCR3 = false;

        isCR3 = QString::fromStdString(loadParam.fullFilename).endsWith(".cr3", Qt::CaseInsensitive);
        const bool isDNG = QString::fromStdString(loadParam.fullFilename).endsWith(".dng", Qt::CaseInsensitive);
        if (isCR3)
        {
            cout << "processImage this is a CR3!" << endl;
        }

        if (!loadParam.tiffIn && !loadParam.jpegIn)
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
#define RAWF3 libraw->imgdata.rawdata.float3_image
#define RAWF4 libraw->imgdata.rawdata.float4_image
#define IDATA libraw->imgdata.idata
#define LENS  libraw->imgdata.lens
#define MAKER libraw->imgdata.lens.makernotes
#define OTHER libraw->imgdata.other
#define SIZES libraw->imgdata.sizes
#define OPTIONS libraw->imgdata.rawparams.options

#ifndef WIN32
            if (libraw->is_floating_point())
            {
                //tell libraw to not convert to int when unpacking.
                OPTIONS = OPTIONS & ~LIBRAW_RAWOPTIONS_CONVERTFLOAT_TO_INT;
            }
#endif // WIN32
            //This makes IMAGE contains the sensel value and 3 blank values at every
            //location.
            libraw_error = libraw->unpack();
            if (libraw_error)
            {
                cout << "processImage: Could not read input file, or was canceled" << endl;
                cout << "libraw error text: " << libraw_strerror(libraw_error) << endl;
                return emptyMatrix();
            }

            bool isFloat = libraw->have_fpdata();

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
            cout << "processImage filename (matrix): " << loadParam.fullFilename << endl;
            for (int i = 0; i < 3; i++)
            {
                cout << "processImage camToRGB matrix: ";
                for (int j = 0; j < 3; j++)
                {
                    camToRGB[i][j] = libraw->imgdata.color.rgb_cam[i][j];
                    cout << camToRGB[i][j] << " ";
                }
                cout << endl;
            }
            if (!isDNG)
            {
                for (int i = 0; i < 3; i++)
                {
                    cout << "processImage xyzToCam matrix: ";
                    for (int j = 0; j < 3; j++)
                    {
                        xyzToCam[i][j] = libraw->imgdata.color.cam_xyz[i][j];
                        cout << xyzToCam[i][j] << " ";
                    }
                    cout << endl;
                }
            } else { //For Sigma fp and fp L cameras LibRaw doesn't report cam_xyz
                cout << "processImage dng color matrix illuminant: " << libraw->imgdata.color.dng_color[0].illuminant << endl;
                cout << "processImage dng color matrix illuminant: " << libraw->imgdata.color.dng_color[1].illuminant << endl;
                int dngProfile = 1;
                if (daylightScore(libraw->imgdata.color.dng_color[0].illuminant) < daylightScore(libraw->imgdata.color.dng_color[1].illuminant))
                {
                    dngProfile = 0;
                }
                cout << "processImage Using dng color matrix number " << dngProfile << endl;
                for (int i = 0; i < 3; i++)
                {
                    cout << "processImage xyzToCam matrix: ";
                    for (int j = 0; j < 3; j++)
                    {
                        xyzToCam[i][j] = libraw->imgdata.color.dng_color[dngProfile].colormatrix[i][j];
                        cout << xyzToCam[i][j] << " ";
                    }
                    cout << endl;
                }
            }
            //LibRaw doesn't give a cam_xyz matrix from the Sigma fp's dng
            //We must reconstruct cam_xyz from rgb_cam and the srgb-to-xyz d65 matrix
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
            //some cameras have individual color channel subtraction.
            //this seems to be an offset from the overall blackpoint
            float rBlack = libraw->imgdata.color.cblack[0];
            float gBlack = libraw->imgdata.color.cblack[1];
            float bBlack = libraw->imgdata.color.cblack[2];
            float g2Black = libraw->imgdata.color.cblack[3];
            float maxChanBlack = max(rBlack, max(gBlack, max(bBlack, g2Black)));
            //Still others have a matrix to subtract.
            int blackRow = int(libraw->imgdata.color.cblack[4]);
            int blackCol = int(libraw->imgdata.color.cblack[5]);

            cout << "BLACKPOINT: ";
            cout << blackpoint << endl;
            cout << "color channel blackpoints" << endl;
            cout << "R  blackpoint: " << rBlack << endl;
            cout << "G  blackpoint: " << gBlack << endl;
            cout << "B  blackpoint: " << bBlack << endl;
            cout << "G2 blackpoint: " << g2Black << endl;
            cout << "block-based blackpoint dimensions:" << endl;
            cout << "blackpoint dim 1: " << libraw->imgdata.color.cblack[4] << endl;
            cout << "blackpoint dim 2: " << libraw->imgdata.color.cblack[5] << endl;
            double sumBlockBlackpoint = 0;
            int count = 0;
            if (blackRow > 0 && blackCol > 0)
            {
                cout << "block-based blackpoint: " << endl;
                for (int i = 0; i < blackRow; i++)
                {
                    for (int j = 0; j < blackCol; j++)
                    {
                        sumBlockBlackpoint += libraw->imgdata.color.cblack[6 + i*blackCol + j];
                        count++;
                        cout << libraw->imgdata.color.cblack[6 + i*blackCol + j] << "  ";
                    }
                    cout << endl;
                }
            }
            double meanBlockBlackpoint = 0;
            if (count > 0)
            {
                meanBlockBlackpoint = sumBlockBlackpoint / count;
            }
            cout << "Mean of block-based blackpoint: " << meanBlockBlackpoint << endl;

            //get white saturation values
            cout << "WHITE SATURATION ===================================" << endl;
            cout << "data_maximum: " << libraw->imgdata.color.data_maximum << endl;
            cout << "maximum: " << libraw->imgdata.color.maximum << endl;

            //Calculate the white point based on the camera settings.
            //This needs the black point subtracted, and a fudge factor to ensure clipping is hard and fast.
            double camconstWhite[4];

            //Some cameras have a black offset, as well, even if the black level is already specified.
            double camconstBlack[4];

            QString makeModel = IDATA.make;
            makeModel.append(" ");
            makeModel.append(IDATA.model);
            bool camconstSuccess = CAMCONST_READ_OK == camconst_read(makeModel, OTHER.iso_speed, OTHER.aperture, camconstWhite, camconstBlack);

            cout << "is the file dng?: " << isDNG << endl;

            //we only process with 3 channels later, so we merge the two green channels here...
            camconstWhite[1] = min(camconstWhite[1], camconstWhite[3]);

            double camconstWhiteMax = max(max(max(camconstWhite[0], camconstWhite[1]), camconstWhite[2]), camconstWhite[3]);
            double camconstWhiteAvg = (camconstWhite[0] + camconstWhite[1] + camconstWhite[2] + camconstWhite[3])/4;
            double camconstBlackAvg = (camconstBlack[0] + camconstBlack[1] + camconstBlack[2] + camconstBlack[3])/4;

            //If the black levels are significantly different, we'll add them.
            if (camconstBlackAvg != blackpoint && !isDNG) //dngs provide their own correct black level and we should trust it
            {
                cout << "Black level discrepancy" << endl;
                cout << "CamConst black: " << camconstBlackAvg << endl;
                cout << "LibRaw black:   " << blackpoint << endl;
                cout << "block black:    " << meanBlockBlackpoint << endl;
                if (blackpoint != 0 && camconstSuccess)
                {
                    if (abs((camconstBlackAvg/blackpoint) - 1) < 0.5)
                    {
                        //if they're within 50%, we want to replace the libraw one
                        blackpoint = camconstBlack[0];
                        rBlack  = 0;
                        gBlack  = camconstBlack[1] - camconstBlack[0];
                        bBlack  = camconstBlack[2] - camconstBlack[0];
                        maxChanBlack = max(rBlack, max(gBlack, bBlack));
                    } else {
                        //Ignore if they're very different, this only applies to Panasonics and there's a better way
                    }
                } else {
                    //if the libraw blackpoint is 0 then we replace it, unless there was a block-based blackpoint
                    if (meanBlockBlackpoint == 0 && camconstSuccess)
                    {
                        blackpoint = camconstBlack[0];
                        rBlack  = 0;
                        gBlack  = camconstBlack[1] - camconstBlack[0];
                        bBlack  = camconstBlack[2] - camconstBlack[0];
                        g2Black = camconstBlack[3] - camconstBlack[0];
                        maxChanBlack = max(rBlack, max(gBlack, max(bBlack, g2Black)));
                    }
                }
                cout << "new black: " << blackpoint << endl;
            }

            //Modern Nikons have camconst.json white levels specified as if they were 14-bit
            // even if the raw files are 12-bit-only, like the entry level cams
            //So we need to detect if it's 12-bit and if the camconst specifies as 14-bit.
            if ((QString(IDATA.make) == "Nikon") && (libraw->imgdata.color.maximum < 4096) && (camconstWhiteAvg >= 4096))
            {
                camconstWhite[0] = camconstWhite[0]*4095/16383;
                camconstWhite[1] = camconstWhite[1]*4095/16383;
                camconstWhite[2] = camconstWhite[2]*4095/16383;
                cout << "Nikon 12-bit camconst white clipping point: " << camconstWhite[0] << endl;
            }


            if (camconstSuccess && camconstWhiteAvg > 0 && !isDNG) //dngs provide their own correct whitepoint and we should trust it over camconst
            {
                maxValue = camconstWhiteMax - blackpoint - maxChanBlack - meanBlockBlackpoint;
                cout << "camconst r white clipping point: " << camconstWhite[0] << endl;
                cout << "camconst g white clipping point: " << camconstWhite[1] << endl;
                cout << "camconst b white clipping point: " << camconstWhite[2] << endl;
                colorMaxValue[0] = camconstWhite[0] - blackpoint - maxChanBlack - meanBlockBlackpoint;
                colorMaxValue[1] = camconstWhite[1] - blackpoint - maxChanBlack - meanBlockBlackpoint;
                colorMaxValue[2] = camconstWhite[2] - blackpoint - maxChanBlack - meanBlockBlackpoint;
            } else {
                maxValue = libraw->imgdata.color.maximum - blackpoint - maxChanBlack - meanBlockBlackpoint;
                cout << "libraw fallback or dng white clipping point: " << libraw->imgdata.color.maximum << endl;
                colorMaxValue[0] = maxValue;
                colorMaxValue[1] = maxValue;
                colorMaxValue[2] = maxValue;
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
            float rawRMax = std::numeric_limits<float>::min();
            float rawGMax = std::numeric_limits<float>::min();
            float rawBMax = std::numeric_limits<float>::min();

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
            if (isFloat && isSraw) { //floating point full-color-per-pixel raws
                raw_image.set_size(raw_height, raw_width*3);
                #pragma omp parallel for reduction (min:rawMin) reduction(max:rawMax) reduction(max:rawRMax) reduction(max:rawGMax) reduction(max:rawBMax)
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
                        raw_image[row][col*3   ] = min(RAWF4[rowoffset + col + leftmargin][0] - tempBlackpoint, colorMaxValue[0]);
                        rawMin = std::min(rawMin, raw_image[row][col*3   ]);
                        rawMax = std::max(rawMax, raw_image[row][col*3   ]);
                        rawRMax = std::max(rawRMax, raw_image[row][col*3   ]);
                        raw_image[row][col*3 +1] = min(RAWF4[rowoffset + col + leftmargin][1] - tempBlackpoint, colorMaxValue[1]);
                        rawMin = std::min(rawMin, raw_image[row][col*3 +1]);
                        rawMax = std::max(rawMax, raw_image[row][col*3 +1]);
                        rawGMax = std::max(rawGMax, raw_image[row][col*3 +1]);
                        raw_image[row][col*3 +2] = min(RAWF4[rowoffset + col + leftmargin][2] - tempBlackpoint, colorMaxValue[2]);
                        rawMin = std::min(rawMin, raw_image[row][col*3 +2]);
                        rawMax = std::max(rawMax, raw_image[row][col*3 +2]);
                        rawBMax = std::max(rawBMax, raw_image[row][col*3 +2]);
                    }
                }
            } else if (isSraw) { //full-color-per-pixel integer raws
                raw_image.set_size(raw_height, raw_width*3);
                #pragma omp parallel for reduction (min:rawMin) reduction(max:rawMax) reduction(max:rawRMax) reduction(max:rawGMax) reduction(max:rawBMax)

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
                        raw_image[row][col*3   ] = min(RAW4[rowoffset + col + leftmargin][0] - tempBlackpoint, colorMaxValue[0]);
                        rawMin = std::min(rawMin, raw_image[row][col*3   ]);
                        rawMax = std::max(rawMax, raw_image[row][col*3   ]);
                        rawRMax = std::max(rawRMax, raw_image[row][col*3   ]);
                        raw_image[row][col*3 +1] = min(RAW4[rowoffset + col + leftmargin][1] - tempBlackpoint, colorMaxValue[1]);
                        rawMin = std::min(rawMin, raw_image[row][col*3 +1]);
                        rawMax = std::max(rawMax, raw_image[row][col*3 +1]);
                        rawGMax = std::max(rawGMax, raw_image[row][col*3 +1]);
                        raw_image[row][col*3 +2] = min(RAW4[rowoffset + col + leftmargin][2] - tempBlackpoint, colorMaxValue[2]);
                        rawMin = std::min(rawMin, raw_image[row][col*3 +2]);
                        rawMax = std::max(rawMax, raw_image[row][col*3 +2]);
                        rawBMax = std::max(rawBMax, raw_image[row][col*3 +2]);
                    }
                }
            } else if (isFloat) { //floating point one-color-per-pixel raws
                #pragma omp parallel for reduction (min:rawMin) reduction(max:rawMax) reduction(max:rawRMax) reduction(max:rawGMax) reduction(max:rawBMax)

                for (int row = 0; row < raw_height; row++)
                {
                    //IMAGE is an (width*height) by 4 array, not width by height by 4.
                    int rowoffset = (row + topmargin)*full_width;
                    for (int col = 0; col < raw_width; col++)
                    {
                        float tempBlackpoint = blackpoint;
                        float tempWhitepoint = maxValue;
                        int color = cfa[row % 2][col % 2];
                        if (color == 0) {
                            tempBlackpoint += rBlack;
                            tempWhitepoint = colorMaxValue[0];
                        }
                        if (color == 1) {
                            tempBlackpoint += gBlack;
                            tempWhitepoint = colorMaxValue[1];
                        }
                        if (color == 2) {
                            tempBlackpoint += bBlack;
                            tempWhitepoint = colorMaxValue[2];
                        }
                        if (color == 3) {
                            tempBlackpoint += g2Black;
                            tempWhitepoint = colorMaxValue[1];
                        }
                        if (blackRow > 0 && blackCol > 0)
                        {
                            tempBlackpoint = min(tempBlackpoint + libraw->imgdata.color.cblack[6 + (row%blackRow)*blackCol + col%blackCol], tempWhitepoint);
                        }
                        raw_image[row][col] = RAWF[rowoffset + col + leftmargin] - tempBlackpoint;
                        rawMin = std::min(rawMin, raw_image[row][col]);
                        rawMax = std::max(rawMax, raw_image[row][col]);
                        if (color == 0) {rawRMax = std::max(rawRMax, raw_image[row][col]);}
                        if (color == 1) {rawGMax = std::max(rawGMax, raw_image[row][col]);}
                        if (color == 2) {rawBMax = std::max(rawBMax, raw_image[row][col]);}
                        if (color == 3) {rawGMax = std::max(rawGMax, raw_image[row][col]);}
                    }
                }
            } else { //normal one-color-per-pixel integer raws
                #pragma omp parallel for reduction (min:rawMin) reduction(max:rawMax) reduction(max:rawRMax) reduction(max:rawGMax) reduction(max:rawBMax)

                for (int row = 0; row < raw_height; row++)
                {
                    //IMAGE is an (width*height) by 4 array, not width by height by 4.
                    int rowoffset = (row + topmargin)*full_width;
                    for (int col = 0; col < raw_width; col++)
                    {
                        float tempBlackpoint = blackpoint;
                        float tempWhitepoint = maxValue;
                        int color = cfa[row % 2][col % 2];
                        if (color == 0) {
                            tempBlackpoint += rBlack;
                            tempWhitepoint = colorMaxValue[0];
                        }
                        if (color == 1) {
                            tempBlackpoint += gBlack;
                            tempWhitepoint = colorMaxValue[1];
                        }
                        if (color == 2) {
                            tempBlackpoint += bBlack;
                            tempWhitepoint = colorMaxValue[2];
                        }
                        if (color == 3) {
                            tempBlackpoint += g2Black;
                            tempWhitepoint = colorMaxValue[1];
                        }
                        if (blackRow > 0 && blackCol > 0)
                        {
                            tempBlackpoint = min(tempBlackpoint + libraw->imgdata.color.cblack[6 + (row%blackRow)*blackCol + col%blackCol], tempWhitepoint);
                        }
                        raw_image[row][col] = RAW[rowoffset + col + leftmargin] - tempBlackpoint;
                        rawMin = std::min(rawMin, raw_image[row][col]);
                        rawMax = std::max(rawMax, raw_image[row][col]);
                        if (color == 0) {rawRMax = std::max(rawRMax, raw_image[row][col]);}
                        if (color == 1) {rawGMax = std::max(rawGMax, raw_image[row][col]);}
                        if (color == 2) {rawBMax = std::max(rawBMax, raw_image[row][col]);}
                        if (color == 3) {rawGMax = std::max(rawGMax, raw_image[row][col]);}
                    }
                }
            }

            //generate raw histogram
            if (WithHisto == histo)
            {
                histoInterface->updateHistRaw(raw_image, colorMaxValue, cfa, xtrans, maxXtrans, isSraw, isMonochrome);
            }

            cout << "max of raw_image: " << rawMax << endl;
            cout << "min of raw_image: " << rawMin << endl;
            cout << "max of raw red:   " << rawRMax << endl;
            cout << "max of raw green: " << rawGMax << endl;
            cout << "max of raw blue:  " << rawBMax << endl;
        }
        valid = paramManager->markLoadComplete();
        updateProgress(valid, 0.0f);
        [[fallthrough]];
    }
    case partdemosaic: [[fallthrough]];
    case load://Do demosaic, or load non-raw images
    {
        LoadParams loadParam;
        DemosaicParams demosaicParam;
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

        if (loadParam.tiffIn)
        {
            if (imread_tiff(loadParam.fullFilename, demosaiced_image, exifData))
            {
                cerr << "Could not open image " << loadParam.fullFilename << "; Exiting..." << endl;
                return emptyMatrix();
            }
        }
        else if (loadParam.jpegIn)
        {
            if (imread_jpeg(loadParam.fullFilename, demosaiced_image, exifData))
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
            demosaiced_image.set_size(raw_height, raw_width*3);
            if (isNikonSraw)
            {
                #pragma omp parallel for
                for (int row = 0; row < raw_height; row++)
                {
                    for (int col = 0; col < raw_width*3; col++)
                    {
                        demosaiced_image(row, col) = raw_image(row, col) * scaleFactor;
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
                        demosaiced_image(row, col) = raw_image(row, col) * scaleFactor * ((color==0) ? rPreMul : (color == 1) ? gPreMul : bPreMul);
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
                        premultiplied(row, col) = raw_image(row, col) * ((color==0) ? rPreMul : (color == 1) ? gPreMul : bPreMul);
                    }
                }
                if (demosaicParam.demosaicMethod == 0)
                {
                    markesteijn_demosaic(raw_width, raw_height, premultiplied, red, green, blue, xtrans, camToRGB4, setProg, 3, true);
                } else { //if it's 1, use xtransfast
                    xtransfast_demosaic(raw_width, raw_height, premultiplied, red, green, blue, xtrans, setProg);
                }
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
                        premultiplied(row, col) = raw_image(row, col) * ((color==0) ? rPreMul : (color == 1) ? gPreMul : bPreMul);
                    }
                }
                if (demosaicParam.caEnabled > 0)
                {
                    //we need to apply white balance and then remove it for Auto CA Correct to work properly
                    double fitparams[2][2][16];
                    CA_correct(0, 0, raw_width, raw_height, true, demosaicParam.caEnabled, 0.0, 0.0, true, premultiplied, premultiplied, cfa, setProg, fitparams, false);
                }

                if (demosaicParam.demosaicMethod == 0)
                {
                    amaze_demosaic(raw_width, raw_height, 0, 0, raw_width, raw_height, premultiplied, red, green, blue, cfa, setProg, initialGain, border, inputscale, outputscale);
                } else { //if it's 1, use LMMSE
                    premultiplied.mult_this(1/inputscale);
                    lmmse_demosaic(raw_width, raw_height, premultiplied, red, green, blue, cfa, setProg, 3);//doesn't like inputs > 1
                    //igv_demosaic(raw_width, raw_height, premultiplied, red, green, blue, cfa, setProg);//doesn't like inputs > 1
                    red.mult_this(outputscale);
                    green.mult_this(outputscale);
                    blue.mult_this(outputscale);
                }
            }
            premultiplied.set_size(0, 0);
            cout << "demosaic end: " << timeDiff(demosaic_time) << endl;

            demosaiced_image.set_size(raw_height, raw_width*3);
            #pragma omp parallel for
            for (int row = 0; row < raw_height; row++)
            {
                for (int col = 0; col < raw_width; col++)
                {
                    demosaiced_image(row, col*3    ) =   red(row, col);
                    demosaiced_image(row, col*3 + 1) = green(row, col);
                    demosaiced_image(row, col*3 + 2) =  blue(row, col);
                }
            }
        }
        cout << "load time: " << timeDiff(imload_time) << endl;

        cout << "ImagePipeline::processImage: Demosaic complete." << endl;
        valid = paramManager->markDemosaicComplete();
        updateProgress(valid, 0.0f);
        [[fallthrough]];
    }
    case partpostdemosaic: [[fallthrough]];
    case demosaic://Do postdemosaic work
    {
        PostDemosaicParams postDemosaicParam;
        AbortStatus abort;
        std::tie(valid, abort, postDemosaicParam) = paramManager->claimPostDemosaicParams();
        if (abort == AbortStatus::restart)
        {
            cout << "imagePipeline.cpp: aborted at demosaic" << endl;
            return emptyMatrix();
        }

        //First thing after demosaic is to apply the user's white balance.
        if (!isMonochrome)
        {
            rawWhiteBalance(demosaiced_image, post_demosaic_image,
                            postDemosaicParam.temperature, postDemosaicParam.tint, xyzToCam,
                            rPreMul, gPreMul, bPreMul,//undoes these
                            rUserMul, gUserMul, bUserMul);//used later for highlight recovery

            cout << "WB pre multiplier R: " << rPreMul << endl;
            cout << "WB pre multiplier G: " << gPreMul << endl;
            cout << "WB pre multiplier B: " << bPreMul << endl;
            cout << "WB cam multiplier R: " << rCamMul << endl;
            cout << "WB cam multiplier G: " << gCamMul << endl;
            cout << "WB cam multiplier B: " << bCamMul << endl;
            cout << "WB user multiplier R: " << rUserMul << endl;
            cout << "WB user multiplier G: " << gUserMul << endl;
            cout << "WB user multiplier B: " << bUserMul << endl;
        } else {
            post_demosaic_image = demosaiced_image;
        }

        //Recover highlights now
        cout << "hlrecovery start:" << timeDiff(timeRequested) << endl;
        struct timeval hlrecovery_time;
        gettimeofday(&hlrecovery_time, nullptr);

        int height = post_demosaic_image.nr();
        int width  = post_demosaic_image.nc()/3;

        //Now, recover highlights.
        std::function<bool(double)> setProg = [](double) -> bool {return false;};
        //And return it back to a single layer
        if (postDemosaicParam.highlights >= 2 && !isMonochrome)
        {
            //For highlight recovery, we need to split up the image into three separate layers.
            matrix<float> rChannel(height, width), gChannel(height, width), bChannel(height, width);

            #pragma omp parallel for
            for (int row = 0; row < height; row++)
            {
                for (int col = 0; col < width; col++)
                {
                    rChannel(row, col) = post_demosaic_image(row, col*3    );
                    gChannel(row, col) = post_demosaic_image(row, col*3 + 1);
                    bChannel(row, col) = post_demosaic_image(row, col*3 + 2);
                }
            }

            //We applied the camMul camera multipliers before applying white balance.
            //Now we need to calculate the channel max and the raw clip levels.
            //Channel max:
            const float chmax[3] = {rChannel.max(), gChannel.max(), bChannel.max()};
            //Max clip point:
            const float clmax[3] = {65535.0f*rUserMul*colorMaxValue[0]/maxValue,
                                    65535.0f*gUserMul*colorMaxValue[1]/maxValue,
                                    65535.0f*bUserMul*colorMaxValue[2]/maxValue};

            HLRecovery_inpaint(width, height, rChannel, gChannel, bChannel, chmax, clmax, setProg);
            #pragma omp parallel for
            for (int row = 0; row < height; row++)
            {
                for (int col = 0; col < width; col++)
                {
                    post_demosaic_image(row, col*3    ) = rChannel(row, col);
                    post_demosaic_image(row, col*3 + 1) = gChannel(row, col);
                    post_demosaic_image(row, col*3 + 2) = bChannel(row, col);
                }
            }
        } else if (postDemosaicParam.highlights == 0 && !isMonochrome)
        {
            #pragma omp parallel for
            for (int row = 0; row < height; row++)
            {
                for (int col = 0; col < width; col++)
                {
                    post_demosaic_image(row, col*3    ) = min(post_demosaic_image(row, col*3    ), 65535.0f);
                    post_demosaic_image(row, col*3 + 1) = min(post_demosaic_image(row, col*3 + 1), 65535.0f);
                    post_demosaic_image(row, col*3 + 2) = min(post_demosaic_image(row, col*3 + 2), 65535.0f);
                }
            }
        } else { //params = 1, or isMonochrome
            //do nothing
        }
        cout << "hlrecovery duration: " << timeDiff(hlrecovery_time) << endl;

        //Apply exposure compensation.
        //This is ideally done before noise reduction so that you don't need dramatically different
        // thresholds for underexposed images.

        const float expCompMult = pow(2, postDemosaicParam.exposureComp);

        #pragma omp parallel for
        for (int row = 0; row < height; row++)
        {
            for (int col = 0; col < width*3; col++)
            {
                post_demosaic_image(row, col) = post_demosaic_image(row, col) * expCompMult;
            }
        }

        valid = paramManager->markPostDemosaicComplete();
        updateProgress(valid, 0.0f);
        [[fallthrough]];
    }
    case partnrnlmeans: [[fallthrough]];
    case postdemosaic://Do nonlocal means (luma+chroma) noise reduction
    {
        NlmeansNRParams nrParam;
        AbortStatus abort;
        std::tie(valid, abort, nrParam) = paramManager->claimNlmeansNRParams();
        if (abort == AbortStatus::restart)
        {
            cout << "imagePipeline aborted at nlmeans noise reduction" << endl;
            return emptyMatrix();
        }

        if (nrParam.nrEnabled && nrParam.nlStrength > 0)
        {
            cout << "Luma NR preprocessing start: " << timeDiff(timeRequested) << endl;
            matrix<float> denoised(post_demosaic_image.nr(), post_demosaic_image.nc());
            matrix<float> preconditioned = post_demosaic_image;

            if (cache == NoCache)
            {
                post_demosaic_image.set_size(0, 0);
                cacheEmpty = true;
            } else {
                cacheEmpty = false;
            }

            //we don't want to apply nonexistent WB multipliers to monochrome images
            const float rMulTemp = isMonochrome ? 1.0f : rUserMul;
            const float gMulTemp = isMonochrome ? 1.0f : gUserMul;
            const float bMulTemp = isMonochrome ? 1.0f : bUserMul;

#pragma omp parallel for
            for (int row = 0; row < preconditioned.nr(); row++)
            {
                for (int col = 0; col < preconditioned.nc(); col+=3)
                {
                    preconditioned(row, col+0) = sRGB_forward_gamma_unclipped(preconditioned(row, col+0)/(rMulTemp*65535.0f));
                    preconditioned(row, col+1) = sRGB_forward_gamma_unclipped(preconditioned(row, col+1)/(gMulTemp*65535.0f));
                    preconditioned(row, col+2) = sRGB_forward_gamma_unclipped(preconditioned(row, col+2)/(bMulTemp*65535.0f));
                }
            }
            float offset = std::max(-preconditioned.min() + 0.001f, 0.001f);
            float scale = std::max(preconditioned.max() + offset, 1.0f);
#pragma omp parallel for
            for (int row = 0; row < preconditioned.nr(); row++)
            {
                for (int col = 0; col < preconditioned.nc(); col++)
                {
                    preconditioned(row, col) = (preconditioned(row, col) + offset)/scale;
                    if (isnan(preconditioned(row,col)))
                    {
                        preconditioned(row,col) = 0.0f;
                    }
                }
            }

            const int numClusters = nrParam.nlClusters;
            const float clusterThreshold = nrParam.nlThresh;
            const float strength = nrParam.nlStrength;

            cout << "Luma NR processing start: " << timeDiff(timeRequested) << endl;
            struct timeval nrTime;
            gettimeofday(&nrTime, nullptr);

            if (kMeansNLMApprox(preconditioned,
                                numClusters,
                                clusterThreshold,
                                strength,
                                preconditioned.nr(),
                                preconditioned.nc()/3,
                                denoised,
                                paramManager)){
                cout << "imagePipeline aborted at nlmeans noise reduction" << endl;
                return emptyMatrix();
            }
            cout << "Nlmeans NR duration: " << timeDiff(nrTime) << endl;

            //Undo the preconditioning
#pragma omp parallel for
            for (int row = 0; row < denoised.nr(); row++)
            {
                for (int col = 0; col < denoised.nc(); col+=3)
                {
                    denoised(row, col+0) = sRGB_inverse_gamma_unclipped(scale*denoised(row, col+0) - offset)*rMulTemp*65535.0f;
                    denoised(row, col+1) = sRGB_inverse_gamma_unclipped(scale*denoised(row, col+1) - offset)*gMulTemp*65535.0f;
                    denoised(row, col+2) = sRGB_inverse_gamma_unclipped(scale*denoised(row, col+2) - offset)*bMulTemp*65535.0f;
                }
            }

            raw_to_oklab(denoised, nlmeans_nr_image, camToRGB);
            denoised.set_size(0, 0);
        } else if (nrParam.nrEnabled) {
            raw_to_oklab(post_demosaic_image, nlmeans_nr_image, camToRGB);

            if (cache == NoCache)
            {
                post_demosaic_image.set_size(0, 0);
                cacheEmpty = true;
            } else {
                cacheEmpty = false;
            }
        }
        //If noise reduction is not enabled, we'll completely skip copying so that the non-NR path is as fast as possible
        //If we're not caching, we do not want to erase the input data unless it actually got used already.

        valid = paramManager->markNlmeansNRComplete();
        updateProgress(valid, 0.0f);
        [[fallthrough]];
    }
    case partnrimpulse: [[fallthrough]];
    case nrnlmeans: //do impulse denoise
    {
        ImpulseNRParams nrParam;
        AbortStatus abort;
        std::tie(valid, abort, nrParam) = paramManager->claimImpulseNRParams();
        if (abort == AbortStatus::restart)
        {
            cout << "imagePipeline aborted at impulse noise reduction" << endl;
            return emptyMatrix();
        }

        if (nrParam.nrEnabled && nrParam.impulseThresh > 0)
        {
            cout << "Impulse NR processing start: " << timeDiff(timeRequested) << endl;
            struct timeval nrTime;
            gettimeofday(&nrTime, nullptr);

            const bool eraseNRInput = (cache == NoCache);
            impulse_nr(nlmeans_nr_image, impulse_nr_image, nrParam.impulseThresh, 1.0, eraseNRInput);

            cout << "Impulse NR duration: " << timeDiff(nrTime) << endl;
        } else if (nrParam.nrEnabled) {
            impulse_nr_image = nlmeans_nr_image;
        }

        if (cache == NoCache)
        {
            nlmeans_nr_image.set_size(0, 0);
            cacheEmpty = true;
        } else {
            cacheEmpty = false;
        }

        valid = paramManager->markImpulseNRComplete();
        updateProgress(valid, 0.0f);
        [[fallthrough]];
    }
    case partnrchroma: [[fallthrough]];
    case nrimpulse: //do chroma denoise
    {
        ChromaNRParams nrParam;
        AbortStatus abort;
        std::tie(valid, abort, nrParam) = paramManager->claimChromaNRParams();
        if (abort == AbortStatus::restart)
        {
            cout << "imagePipeline aborted at chroma noise reduction" << endl;
            return emptyMatrix();
        }

        if (nrParam.nrEnabled && nrParam.chromaStrength > 0)
        {
            cout << "Chroma NR processing start: " << timeDiff(timeRequested) << endl;
            struct timeval nrTime;
            gettimeofday(&nrTime, nullptr);

            const bool eraseNRInput = (cache == NoCache);
            RGB_denoise(0, impulse_nr_image, chroma_nr_image, nrParam.chromaStrength, 0.0f, 0.0f, paramManager, eraseNRInput);

            cout << "Chroma NR duration: " << timeDiff(nrTime) << endl;
        } else if (nrParam.nrEnabled) {
            chroma_nr_image = impulse_nr_image;
        }
        paramManager->markChromaNRComplete();


        if (cache == NoCache)
        {
            impulse_nr_image.set_size(0, 0);
            cacheEmpty = true;
        }
        else
        {
            cacheEmpty = false;
        }
        valid = paramManager->markChromaNRComplete();
        updateProgress(valid, 0.0f);
        [[fallthrough]];
    }
    case partprefilmulation: [[fallthrough]];
    case nrchroma://Do pre-filmulation work.
    {
        PrefilmParams prefilmParam;
        cout << "imagePipeline beginning pre-filmulation" << endl;
        AbortStatus abort;
        std::tie(valid, abort, prefilmParam) = paramManager->claimPrefilmParams();
        if (abort == AbortStatus::restart)
        {
            cout << "imagePipeline aborted at pre-filmulation" << endl;
            return emptyMatrix();
        }

        //Copy from the correct location if no NR
        //If NR, then convert from oklab back to raw colors
        //Then apply lensfun to raw colors

        matrix<float> prefilm_input_image;
        if (prefilmParam.nrEnabled)
        {
            oklab_to_raw(chroma_nr_image, prefilm_input_image,camToRGB);
            if (cache == NoCache)
            {
                chroma_nr_image.set_size(0, 0);
            }
        } else {
            prefilm_input_image = post_demosaic_image;
        }
        int height = prefilm_input_image.nr();
        int width = prefilm_input_image.nc()/3;

        //Lensfun processing
        cout << "lensfun start" << endl;
        lfDatabase *ldb = lf_db_create();
        QDir dir = QDir::home();
        QString dirstr = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        dirstr.append("/filmulator/version_2");
        std::string stdstring = dirstr.toStdString();
        ldb->Load(stdstring.c_str());

        std::string camName = prefilmParam.cameraName.toStdString();
        const lfCamera * camera = NULL;
        const lfCamera ** cameraList = ldb->FindCamerasExt(NULL,camName.c_str());

        //Set up stuff for rotation.
        //We expect rotation to be from -45 to +45
        //But -50 will be the signal from the UI to disable it.
        float rotationAngle = prefilmParam.rotationAngle * 3.1415926535/180;//convert degrees to radians
        if (prefilmParam.rotationAngle <= -49) {
            rotationAngle = 0;
        }
        cout << "cos rotationangle: " << cos(rotationAngle) << endl;
        cout << "sin rotationangle: " << sin(rotationAngle) << endl;
        bool lensfunGeometryCorrectionApplied = false;

        if (cameraList)
        {
            const float cropFactor = cameraList[0]->CropFactor;

            QString tempLensName = prefilmParam.lensName;
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
                lfModifier * mod = new lfModifier(lens, prefilmParam.focalLength, cropFactor, width, height, LF_PF_F32);
#else //lensfun v0.3.95
                lfModifier * mod = new lfModifier(cropFactor, width, height, LF_PF_F32);
#endif

                int modflags = 0;
                if (prefilmParam.lensfunCA && !isMonochrome)
                {
#ifdef LF_GIT
                    modflags |= mod->EnableTCACorrection();
#else //lensfun v0.3.95
                    modflags |= mod->EnableTCACorrection(lens, prefilmParam.focalLength);
#endif
                }
                if (prefilmParam.lensfunVignetting)
                {
#ifdef LF_GIT
                    modflags |= mod->EnableVignettingCorrection(prefilmParam.fnumber, 1000.0f);
#else //lensfun v0.3.95
                    modflags |= mod->EnableVignettingCorrection(lens, prefilmParam.focalLength, demosaicParam.fnumber, 1000.0f);
#endif
                }
                if (prefilmParam.lensfunDistortion)
                {
#ifdef LF_GIT
                    modflags |= mod->EnableDistortionCorrection();
#else //lensfun v0.3.95
                    modflags |= mod->EnableDistortionCorrection(lens, prefilmParam.focalLength);
#endif
                    modflags |= mod->EnableScaling(mod->GetAutoScale(false));
                    cout << "Auto scale factor: " << mod->GetAutoScale(false) << endl;
                }

                //Now we actually perform the required processing.
                //First is vignetting.
                if (prefilmParam.lensfunVignetting)
                {
                    bool success = true;
#pragma omp parallel for
                    for (int row = 0; row < height; row++)
                    {
                        success = mod->ApplyColorModification(prefilm_input_image[row], 0.0f, row, width, 1, LF_CR_3(RED, GREEN, BLUE), width);
                    }
                }

                //Next is CA, or distortion, or both.
                if (prefilmParam.lensfunCA || prefilmParam.lensfunDistortion)
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

                    pre_film_image.set_size(height, width*3);
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
                                    pre_film_image(row, col*3 + c) = prefilm_input_image(sY, sX) * sWY * sWX +
                                                                     prefilm_input_image(eY, sX) * eWY * sWX +
                                                                     prefilm_input_image(sY, eX) * sWY * eWX +
                                                                     prefilm_input_image(eY, eX) * eWY * eWX;
                                }
                            }
                        }
                    }
                }// else {
                    //demosaiced image isn't populated
                    //if geometry wasn't changed, then we'll move stuff over later.
                //}

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

        cout << "after lensfun " << endl;

        //also do rotations on non-corrected images
        if (!lensfunGeometryCorrectionApplied)
        {
            if (rotationAngle != 0.0f)
            {
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
                pre_film_image.set_size(height, width*3);

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
                            pre_film_image(row, col*3 + c) = prefilm_input_image(sY, sX + c) * sWY * sWX +
                                                             prefilm_input_image(eY, sX + c) * eWY * sWX +
                                                             prefilm_input_image(sY, eX + c) * sWY * eWX +
                                                             prefilm_input_image(eY, eX + c) * eWY * eWX;
                        }
                    }
                }
            } else {
                //if we never rotate and never use lensfun geometry correction, we need to move the image over
                pre_film_image.swap(prefilm_input_image);
            }
        }

        //resize into a small image
        if (quality == LowQuality)
        {
            cout << "thumbnail scale start:" << timeDiff (timeRequested) << endl;
            struct timeval downscale_time;
            gettimeofday(&downscale_time, nullptr);
            downscale_and_crop(pre_film_image, pre_film_image_small, 0, 0, (pre_film_image.nc()/3)-1,pre_film_image.nr()-1, 600, 600);
            cout << "thumbnail scale end: " << timeDiff( downscale_time ) << endl;
        }
        else if (quality == PreviewQuality)
        {
            cout << "preview scale start:" << timeDiff (timeRequested) << endl;
            struct timeval downscale_time;
            gettimeofday(&downscale_time, nullptr);
            //Make previews have same even/oddness as the source image
            int paritywidth = resolution + resolution%2 + (pre_film_image.nc()/3)%2;
            int parityheight = resolution + resolution%2 + (pre_film_image.nr())%2;
            downscale_and_crop(pre_film_image, pre_film_image_small, 0, 0, (pre_film_image.nc()/3)-1,pre_film_image.nr()-1, paritywidth, parityheight);
            cout << "preview scale end: " << timeDiff( downscale_time ) << endl;
        }



        if (WithHisto == histo)
        {
            //grab crop and rotation parameters
            CropParams cropParam = paramManager->claimCropParams();
            cropHeight = cropParam.cropHeight;
            cropAspect = cropParam.cropAspect;
            cropHoffset = cropParam.cropHoffset;
            cropVoffset = cropParam.cropVoffset;
            rotation = cropParam.rotation;
            histoInterface->updateHistPreFilm(pre_film_image, 65535,
                                              rotation,
                                              cropHeight, cropAspect,
                                              cropHoffset, cropVoffset);
        }

        cout << "ImagePipeline::processImage: Prefilmulation complete." << endl;

        valid = paramManager->markPrefilmComplete();
        updateProgress(valid, 0.0f);
        [[fallthrough]];
    }
    case partfilmulation: [[fallthrough]];
    case prefilmulation://Do filmulation
    {

        matrix<float> film_input_image;
        if (stealData)
        {
            cout << "imagePipeline stealing data" << endl;
            exifData = stealVictim->exifData;
            rCamMul = stealVictim->rCamMul;
            gCamMul = stealVictim->gCamMul;
            bCamMul = stealVictim->bCamMul;
            rPreMul = stealVictim->rPreMul;
            gPreMul = stealVictim->gPreMul;
            bPreMul = stealVictim->bPreMul;
            rUserMul = stealVictim->rUserMul;
            gUserMul = stealVictim->gUserMul;
            bUserMul = stealVictim->bUserMul;
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
                    xyzToCam[i][j] = stealVictim->xyzToCam[i][j];
                }
            }
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    camToRGB4[i][j] = stealVictim->camToRGB4[i][j];
                }
            }
            if (!isMonochrome)
            {
                raw_to_sRGB(stealVictim->pre_film_image, film_input_image, camToRGB);
            } else {
                film_input_image = stealVictim->pre_film_image;
            }
        } else {
            cout << "imagePipeline not stealing data" << endl;
            if (quality == LowQuality || quality == PreviewQuality)
            {
                //grab shrunken image
                if (!isMonochrome)
                {
                    raw_to_sRGB(pre_film_image_small, film_input_image, camToRGB);
                } else {
                    film_input_image = pre_film_image_small;
                }
            } else {
                if (!isMonochrome)
                {
                    raw_to_sRGB(pre_film_image, film_input_image, camToRGB);
                } else {
                    film_input_image = pre_film_image;
                }
            }

            if (NoCache == cache)
            {
                pre_film_image.set_size(0, 0);
                pre_film_image_small.set_size(0, 0);
                cacheEmpty = true;
            }
            else
            {
                cacheEmpty = false;
            }
        }

        cout << "imagePipeline beginning filmulation" << endl;
        cout << "imagePipeline image width:  " << film_input_image.nc()/3 << endl;
        cout << "imagePipeline image height: " << film_input_image.nr() << endl;

        //We don't need to check abort status out here, because
        //the filmulate function will do so inside its loop.
        //We just check for it returning an empty matrix.

        //Here we do the film simulation on the image...
        //If filmulate detects an abort, it returns true.
        if (filmulate(film_input_image,
                      filmulated_image,
                      paramManager,
                      this))
        {
            cout << "imagePipeline aborted at filmulation" << endl;
            return emptyMatrix();
        }

        if (WithHisto == histo)
        {
            //grab crop and rotation parameters
            CropParams cropParam = paramManager->claimCropParams();
            bool updatePreFilm = false;
            if (cropHeight != cropParam.cropHeight ||
                    cropAspect != cropParam.cropAspect ||
                    cropHoffset != cropParam.cropHoffset ||
                    cropVoffset != cropParam.cropVoffset ||
                    rotation != cropParam.rotation)
            {
                updatePreFilm = true;
            }
            cropHeight = cropParam.cropHeight;
            cropAspect = cropParam.cropAspect;
            cropHoffset = cropParam.cropHoffset;
            cropVoffset = cropParam.cropVoffset;
            rotation = cropParam.rotation;
            if (updatePreFilm)
            {
                if (!stealData)
                {
                    histoInterface->updateHistPreFilm(pre_film_image, 65535,
                                                      rotation,
                                                      cropHeight, cropAspect,
                                                      cropHoffset, cropVoffset);
                } else {
                    histoInterface->updateHistPreFilm(stealVictim->pre_film_image, 65535,
                                                      rotation,
                                                      cropHeight, cropAspect,
                                                      cropHoffset, cropVoffset);
                }
            }
            histoInterface->updateHistPostFilm(filmulated_image, .0025f,//TODO connect this magic number to the qml
                                               rotation,
                                               cropHeight, cropAspect,
                                               cropHoffset, cropVoffset);
        }

        cout << "ImagePipeline::processImage: Filmulation complete." << endl;

        valid = paramManager->markFilmComplete();
        updateProgress(valid, 0.0f);
        [[fallthrough]];
    }
    case partblackwhite: [[fallthrough]];
    case filmulation://Do whitepoint_blackpoint
    {
        cout << "imagePipeline beginning whitepoint blackpoint" << endl;
        cout << "imagePipeline image width:  " << filmulated_image.nc()/3 << endl;
        cout << "imagePipeline image height: " << filmulated_image.nr() << endl;

        BlackWhiteParams blackWhiteParam;
        AbortStatus abort;
        std::tie(valid, abort, blackWhiteParam) = paramManager->claimBlackWhiteParams();
        if (abort == AbortStatus::restart)
        {
            cout << "imagePipeline aborted at whitepoint blackpoint" << endl;
            return emptyMatrix();
        }

        //Update histograms if necessary to correspond to crop
        if (WithHisto == histo)
        {
            //grab crop and rotation parameters
            bool updatePrePostFilm = false;
            if (cropHeight != blackWhiteParam.cropHeight ||
                    cropAspect != blackWhiteParam.cropAspect ||
                    cropHoffset != blackWhiteParam.cropHoffset ||
                    cropVoffset != blackWhiteParam.cropVoffset ||
                    rotation != blackWhiteParam.rotation)
            {
                updatePrePostFilm = true;
            }
            cropHeight = blackWhiteParam.cropHeight;
            cropAspect = blackWhiteParam.cropAspect;
            cropHoffset = blackWhiteParam.cropHoffset;
            cropVoffset = blackWhiteParam.cropVoffset;
            rotation = blackWhiteParam.rotation;
            if (updatePrePostFilm)
            {
                if (!stealData)
                {
                    histoInterface->updateHistPreFilm(pre_film_image, 65535,
                                                      rotation,
                                                      cropHeight, cropAspect,
                                                      cropHoffset, cropVoffset);
                } else {
                    histoInterface->updateHistPreFilm(stealVictim->pre_film_image, 65535,
                                                      rotation,
                                                      cropHeight, cropAspect,
                                                      cropHoffset, cropVoffset);
                }
                histoInterface->updateHistPostFilm(filmulated_image, .0025f,//TODO connect this magic number to the qml
                                                   rotation,
                                                   cropHeight, cropAspect,
                                                   cropHoffset, cropVoffset);
            }
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
        cout << "imagePipeline beginning dummy color curve" << endl;
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
            cacheEmpty = true;
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
        cout << "imagePipeline beginning film like curve" << endl;

        FilmlikeCurvesParams curvesParam;
        AbortStatus abort;
        std::tie(valid, abort, curvesParam) = paramManager->claimFilmlikeCurvesParams();
        if (abort == AbortStatus::restart)
        {
            cout << "imagePipeline aborted at color curve" << endl;
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
        cout << "imagePipeline beginning output" << endl;
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

    cout << "imagePipeline aborted at end" << endl;
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

    std::swap(filename, swapTarget->filename);

    std::swap(cfa, swapTarget->cfa);
    std::swap(xtrans, swapTarget->xtrans);
    std::swap(maxXtrans, swapTarget->maxXtrans);

    std::swap(raw_width, swapTarget->raw_width);
    std::swap(raw_height, swapTarget->raw_height);

    std::swap(camToRGB, swapTarget->camToRGB);
    std::swap(xyzToCam, swapTarget->xyzToCam);
    std::swap(camToRGB4, swapTarget->camToRGB4);

    std::swap(rCamMul, swapTarget->rCamMul);
    std::swap(gCamMul, swapTarget->gCamMul);
    std::swap(bCamMul, swapTarget->bCamMul);
    std::swap(rPreMul, swapTarget->rPreMul);
    std::swap(gPreMul, swapTarget->gPreMul);
    std::swap(bPreMul, swapTarget->bPreMul);
    std::swap(rUserMul, swapTarget->rUserMul);
    std::swap(gUserMul, swapTarget->gUserMul);
    std::swap(bUserMul, swapTarget->bUserMul);

    std::swap(maxValue, swapTarget->maxValue);
    std::swap(isSraw, swapTarget->isSraw);
    std::swap(isNikonSraw, swapTarget->isNikonSraw);
    std::swap(isMonochrome, swapTarget->isMonochrome);
    std::swap(isCR3, swapTarget->isCR3);

    std::swap(cropHeight, swapTarget->cropHeight);
    std::swap(cropAspect, swapTarget->cropAspect);
    std::swap(cropHoffset, swapTarget->cropHoffset);
    std::swap(cropVoffset, swapTarget->cropVoffset);
    std::swap(rotation, swapTarget->rotation);

    std::swap(exifData, swapTarget->exifData);
    std::swap(basicExifData, swapTarget->basicExifData);

    raw_image.swap(swapTarget->raw_image);
    demosaiced_image.swap(swapTarget->demosaiced_image);
    post_demosaic_image.swap(swapTarget->post_demosaic_image);
    nlmeans_nr_image.swap(swapTarget->nlmeans_nr_image);
    impulse_nr_image.swap(swapTarget->impulse_nr_image);
    chroma_nr_image.swap(swapTarget->chroma_nr_image);
    pre_film_image.swap(swapTarget->pre_film_image);
    pre_film_image_small.swap(swapTarget->pre_film_image_small);
    filmulated_image.swap(swapTarget->filmulated_image);
    contrast_image.swap(swapTarget->contrast_image);
    color_curve_image.swap(swapTarget->color_curve_image);
    vibrance_saturation_image.swap(swapTarget->vibrance_saturation_image);
}

//This is used to update the histograms once data is copied on an image change
void ImagePipeline::rerunHistograms()
{
    if (WithHisto == histo)
    {
        if (valid >= Valid::load)
        {
            histoInterface->updateHistRaw(raw_image, colorMaxValue, cfa, xtrans, maxXtrans, isSraw, isMonochrome);
        }
        if (valid >= Valid::prefilmulation)
        {
            histoInterface->updateHistPreFilm(pre_film_image, 65535,
                                              rotation,
                                              cropHeight, cropAspect, cropHoffset, cropVoffset);
        }
        if (valid >= Valid::filmulation)
        {
            histoInterface->updateHistPostFilm(filmulated_image, .0025f,
                                               rotation,
                                               cropHeight, cropAspect, cropHoffset, cropVoffset);
        }
        if (valid >= Valid::filmlikecurve)
        {
            histoInterface->updateHistFinal(vibrance_saturation_image);
        }
    }
}

//Return the average level of each channel of the image sampled at a 21x21
// square.
//The square is positioned relative to the image dimensions of the cropped image.
void ImagePipeline::sampleWB(const float xPos, const float yPos,
                             const int rotation,
                             const float cropHeight, const float cropAspect,
                             const float cropVoffset, const float cropHoffset,
                             float &red, float &green, float &blue)
{
    if (xPos < 0 || xPos > 1 || yPos < 0 || yPos > 1)
    {
        red = -1;
        green = -1;
        blue = -1;
        return;
    }

    //recovered_image is what we're looking to sample.
    //It already has the camera multipliers applied, so we have to divide by them later.

    //First we rotate it.
    matrix<float> rotated_image;
    rotate_image(pre_film_image, rotated_image, rotation);

    //Then we crop the recovered image
    //This is copied from the actual image pipeline.
    const int imWidth  = rotated_image.nc()/3;
    const int imHeight = rotated_image.nr();

    const float tempHeight = imHeight*max(min(1.0f,cropHeight),0.0f);//restrict domain to 0:1
    const float tempAspect = max(min(10000.0f,cropAspect),0.0001f);//restrict aspect ratio
    int width  = int(round(min(tempHeight*tempAspect,float(imWidth))));
    int height = int(round(min(tempHeight, imWidth/tempAspect)));
    const float maxHoffset = (1.0f-(float(width)  / float(imWidth) ))/2.0f;
    const float maxVoffset = (1.0f-(float(height) / float(imHeight)))/2.0f;
    const float oddH = (!(int(round((imWidth  - width )/2.0))*2 == (imWidth  - width )))*0.5f;//it's 0.5 if it's odd, 0 otherwise
    const float oddV = (!(int(round((imHeight - height)/2.0))*2 == (imHeight - height)))*0.5f;//it's 0.5 if it's odd, 0 otherwise
    const float hoffset = (round(max(min(cropHoffset, maxHoffset), -maxHoffset) * imWidth  + oddH) - oddH)/imWidth;
    const float voffset = (round(max(min(cropVoffset, maxVoffset), -maxVoffset) * imHeight + oddV) - oddV)/imHeight;
    int startX = int(round(0.5f*(imWidth  - width ) + hoffset*imWidth));
    int startY = int(round(0.5f*(imHeight - height) + voffset*imHeight));
    int endX = startX + width  - 1;
    int endY = startY + height - 1;

    if (cropHeight <= 0)//it shall be turned off
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


    rotated_image.set_size(0, 0);

    //Now we compute the x position
    const int sampleX = round(xPos * (width-1));
    const int sampleY = round(yPos * (height-1));
    const int sampleStartX = max(0, sampleX - 10);
    const int sampleStartY = max(0, sampleY - 10);
    const int sampleEndX = min(width-1, sampleX + 10);
    const int sampleEndY = min(height-1, sampleY + 10);

    double rSum = 0;
    double gSum = 0;
    double bSum = 0;
    int count = 0;
    for (int row = sampleStartY; row <= sampleEndY; row++)
    {
        for (int col = sampleStartX; col <= sampleEndX; col++)
        {
            rSum += cropped_image(row, col*3    );
            gSum += cropped_image(row, col*3 + 1);
            bSum += cropped_image(row, col*3 + 2);
            count++;
        }
    }
    if (count < 1)//some sort of error occurs
    {
        red = -1;
        green = -1;
        blue = -1;
        return;
    }

    //Compute the average and also divide by the camera WB multipliers
    red   = rSum / (rUserMul*count);
    green = gSum / (gUserMul*count);
    blue  = bSum / (bUserMul*count);
    cout << "custom WB sampled r: " << red << endl;
    cout << "custom WB sampled g: " << green << endl;
    cout << "custom WB sampled b: " << blue << endl;
}

void ImagePipeline::clearInvalid(Valid validIn)
{
    if (validIn < load)
    {
        raw_image.set_size(0, 0);
    }
    if (validIn < demosaic)
    {
        demosaiced_image.set_size(0, 0);
    }
    if (validIn < postdemosaic)
    {
        post_demosaic_image.set_size(0, 0);
    }
    if (validIn < nrnlmeans)
    {
        nlmeans_nr_image.set_size(0, 0);
    }
    if (validIn < nrimpulse)
    {
        impulse_nr_image.set_size(0, 0);
    }
    if (validIn < nrchroma)
    {
        chroma_nr_image.set_size(0, 0);
    }
    if (validIn < prefilmulation)
    {
        pre_film_image.set_size(0, 0);
        pre_film_image_small.set_size(0, 0);
    }
    if (validIn < filmulation)
    {
        filmulated_image.set_size(0, 0);
    }
    if (validIn < blackwhite)
    {
        contrast_image.set_size(0, 0);
    }
    if (validIn < colorcurve)
    {
        color_curve_image.set_size(0, 0);
    }
    if (validIn < filmlikecurve)
    {
        vibrance_saturation_image.set_size(0, 0);
    }
}
