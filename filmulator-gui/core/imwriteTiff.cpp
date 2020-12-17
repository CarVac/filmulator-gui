/* 
 * This file is part of Filmulator.
 *
 * Copyright 2013 Omer Mano and Carlo Vaccari
 *
 * Filmulator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Filmulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Filmulator. If not, see <http://www.gnu.org/licenses/>
 */
#include "filmSim.hpp"

bool imwrite_tiff(const matrix<unsigned short>& output, string outputfilename,
                  Exiv2::ExifData exifData)
{
    int xsize, ysize;
    xsize = output.nc()/3;
    ysize = output.nr();



    outputfilename = outputfilename + ".tif";
    TIFF *out = TIFFOpen(outputfilename.c_str(),"w");
    if (!out)
    {
        cerr << "Can't open file for writing" << endl;
        return 1;
    }	
    TIFFSetField(out, TIFFTAG_IMAGEWIDTH, xsize);  
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, ysize);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 3); //RGB
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 16);
    TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
    //Magic below
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    //End Magic

    tsize_t linebytes = 3 * xsize * 2;//Size in bytes of a line
    unsigned short *buf = NULL;
    buf =(unsigned short *)_TIFFmalloc(linebytes);
    for (int j = 0; j < ysize; j++)
    {
        for(int i = 0; i < xsize; i ++)
        {
            buf[i*3  ] = output(j,i*3  );
            buf[i*3+1] = output(j,i*3+1);
            buf[i*3+2] = output(j,i*3+2);
        }
        if (TIFFWriteScanline(out, buf, j, 0) < 0)
            break;
    }
    (void) TIFFClose(out);

    if (buf)
        _TIFFfree(buf);

    cleanExif(exifData);

    exifData["Exif.Image.Orientation"] = 1;//set all images to unrotated
    exifData["Exif.Image.ImageWidth"] = output.nc()/3;
    exifData["Exif.Image.ImageLength"] = output.nr();
    exifData["Exif.Photo.ColorSpace"] = 1;
    exifData["Exif.Image.ProcessingSoftware"] = "Filmulator";

    cout << "imwrite_tiff exiv filename: " << outputfilename << endl;
    auto image = Exiv2::ImageFactory::open(outputfilename);
    assert(image.get() != 0);

    image->setExifData(exifData);
    //image->writeMetadata();

    return 0;
}

//exif stripping based on darktable's exif.cc
static void remove_exif_keys(Exiv2::ExifData &exifData, const char *keys[], unsigned int n_keys)
{
    for (unsigned int i=0; i < n_keys; i++)
    {
        try {
            Exiv2::ExifData::iterator pos;
            while ((pos = exifData.findKey(Exiv2::ExifKey(keys[i]))) != exifData.end())
            {
                exifData.erase(pos);
            }
        } catch (Exiv2::AnyError &e) {
            //catch invalid tag
        }
    }
}

void cleanExif(Exiv2::ExifData &exifData)
{
    // Remove thumbnail
    auto thumb = Exiv2::ExifThumb(exifData);
    thumb.erase();
    {
        static const char *keys[] = {
            "Exif.Thumbnail.Compression",
            "Exif.Thumbnail.XResolution",
            "Exif.Thumbnail.YResolution",
            "Exif.Thumbnail.ResolutionUnit",
            "Exif.Thumbnail.JPEGInterchangeFormat",
            "Exif.Thumbnail.JPEGInterchangeFormatLength"
        };
        static const int n_keys = 6;
        remove_exif_keys(exifData, keys, n_keys);
    }

    // only compressed images may set these
    {
        static const char *keys[] = {
            "Exif.Photo.PixelXDimension",
            "Exif.Photo.PixelYDimension"
        };
        static const int n_keys = 2;
        remove_exif_keys(exifData, keys, n_keys);
    }

    {
        static const char *keys[] = {
            "Exif.Image.ImageWidth",
            "Exif.Image.ImageLength",
            "Exif.Image.BitsPerSample",
            "Exif.Image.Compression",
            "Exif.Image.PhotometricInterpretation",
            "Exif.Image.FillOrder",
            "Exif.Image.SamplesPerPixel",
            "Exif.Image.StripOffsets",
            "Exif.Image.RowsPerStrip",
            "Exif.Image.StripByteCounts",
            "Exif.Image.PlanarConfiguration",
            "Exif.Image.DNGVersion",
            "Exif.Image.DNGBackwardVersion"
        };
        static const int n_keys = 13;
        remove_exif_keys(exifData, keys, n_keys);
    }

    //remove subimages
    for(Exiv2::ExifData::iterator i = exifData.begin(); i != exifData.end();)
    {
        static const std::string needle = "Exif.SubImage";
        if(i->key().compare(0, needle.length(), needle) == 0)
        {
            i = exifData.erase(i);
        } else {
            ++i;
        }
    }

    {
        static const char *keys[] = {
            // Canon color space info
            "Exif.Canon.ColorSpace",
            "Exif.Canon.ColorData",

            // Nikon thumbnail data
            "Exif.Nikon3.Preview",
            "Exif.NikonPreview.JPEGInterchangeFormat",

            // DNG stuff that is irrelevant or misleading
            "Exif.Image.DNGPrivateData",
            "Exif.Image.DefaultBlackRender",
            "Exif.Image.DefaultCropOrigin",
            "Exif.Image.DefaultCropSize",
            "Exif.Image.RawDataUniqueID",
            "Exif.Image.OriginalRawFileName",
            "Exif.Image.OriginalRawFileData",
            "Exif.Image.ActiveArea",
            "Exif.Image.MaskedAreas",
            "Exif.Image.AsShotICCProfile",
            "Exif.Image.OpcodeList1",
            "Exif.Image.OpcodeList2",
            "Exif.Image.OpcodeList3",
            "Exif.Photo.MakerNote",

            // Pentax thumbnail data
            "Exif.Pentax.PreviewResolution",
            "Exif.Pentax.PreviewLength",
            "Exif.Pentax.PreviewOffset",
            "Exif.PentaxDng.PreviewResolution",
            "Exif.PentaxDng.PreviewLength",
            "Exif.PentaxDng.PreviewOffset",
            // Pentax color info
            "Exif.PentaxDng.ColorInfo",

            // Minolta thumbnail data
            "Exif.Minolta.Thumbnail",
            "Exif.Minolta.ThumbnailOffset",
            "Exif.Minolta.ThumbnailLength",

            // Sony thumbnail data
            "Exif.SonyMinolta.ThumbnailOffset",
            "Exif.SonyMinolta.ThumbnailLength",

            // Olympus thumbnail data
            "Exif.Olympus.Thumbnail",
            "Exif.Olympus.ThumbnailOffset",
            "Exif.Olympus.ThumbnailLength",

            "Exif.Image.BaselineExposureOffset"
        };
        static const int n_keys = 34;
        remove_exif_keys(exifData, keys, n_keys);
    }

#if EXIV2_MINOR_VERSION >= 23
    {
        //older exiv2 will drop all exif if this is executed
        //Samsung makernote cleanup
        static const char *keys[] = {
            "Exif.Samsung2.SensorAreas",
            "Exif.Samsung2.ColorSpace",
            "Exif.Samsung2.EncryptionKey",
            "Exif.Samsung2.WB_RGGBLevelsUncorrected",
            "Exif.Samsung2.WB_RGGBLevelsAuto",
            "Exif.Samsung2.WB_RGGBLevelsIlluminator1",
            "Exif.Samsung2.WB_RGGBLevelsIlluminator2",
            "Exif.Samsung2.WB_RGGBLevelsBlack",
            "Exif.Samsung2.ColorMatrix",
            "Exif.Samsung2.ColorMatrixSRGB",
            "Exif.Samsung2.ColorMatrixAdobeRGB",
            "Exif.Samsung2.ToneCurve1",
            "Exif.Samsung2.ToneCurve2",
            "Exif.Samsung2.ToneCurve3",
            "Exif.Samsung2.ToneCurve4"
        };
        static const int n_keys = 15;
        remove_exif_keys(exifData, keys, n_keys);
    }
#endif

    {
        static const char *keys[] = {
            // Embedded color profile info
            "Exif.Image.CalibrationIlluminant1",
            "Exif.Image.CalibrationIlluminant2",
            "Exif.Image.ColorMatrix1",
            "Exif.Image.ColorMatrix2",
            "Exif.Image.ForwardMatrix1",
            "Exif.Image.ForwardMatrix2",
            "Exif.Image.ProfileCalibrationSignature",
            "Exif.Image.ProfileCopyright",
            "Exif.Image.ProfileEmbedPolicy",
            "Exif.Image.ProfileHueSatMapData1",
            "Exif.Image.ProfileHueSatMapData2",
            "Exif.Image.ProfileHueSatMapDims",
            "Exif.Image.ProfileHueSatMapEncoding",
            "Exif.Image.ProfileLookTableData",
            "Exif.Image.ProfileLookTableDims",
            "Exif.Image.ProfileLookTableEncoding",
            "Exif.Image.ProfileName",
            "Exif.Image.ProfileToneCurve",
            "Exif.Image.ReductionMatrix1",
            "Exif.Image.ReductionMatrix2"
        };
        static const int n_keys = 20;
        remove_exif_keys(exifData, keys, n_keys);
    }

    /*{
        static const char *keys[] = {
        };
        static const int n_keys = 0;
        remove_exif_keys(exifData, keys, n_keys);
    }*/
}
