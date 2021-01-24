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

bool imwrite_jpeg(matrix<unsigned short> &output, string outputfilename,
                  Exiv2::ExifData exifData, int quality, string thumbPath, bool writeExif)
{
    int xsize = output.nc()/3;
    int ysize = output.nr();
    if (quality > 100)
    {
        quality = 100;
    }
    else if (quality < 10)
    {
        quality = 10;
    }
	
	outputfilename = outputfilename + ".jpg";
	//From example.c of libjpeg-turbo
	/* This struct contains the JPEG compression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	* It is possible to have several such structures, representing multiple
	* compression/decompression processes, in existence at once.  We refer
	* to any one struct (and its associated working data) as a "JPEG object".
	*/
	struct jpeg_compress_struct cinfo;
	/* This struct represents a JPEG error handler.  It is declared separately
	* because applications often want to supply a specialized error handler
	* (see the second half of this file for an example).  But here we just
	* take the easy way out and use the standard error handler, which will
	* print a message on stderr and call exit() if compression fails.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct jpeg_error_mgr jerr;
	/* More stuff */
	FILE * outfile;		/* target file */

	/* Step 1: allocate and initialize JPEG compression object */

	/* We have to set up the error handler first, in case the initialization
	* step fails.  (Unlikely, but it could happen if you are out of memory.)
	* This routine fills in the contents of struct jerr, and returns jerr's
	* address which we place into the link field in cinfo.
	*/
	cinfo.err = jpeg_std_error(&jerr);
	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);

	/* Step 2: specify data destination (eg, a file) */
	/* Note: steps 2 and 3 can be done in either order. */

	/* Here we use the library-supplied code to send compressed data to a
	* stdio stream.  You can also write your own code to do something else.
	* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	* requires it in order to write binary files.
	*/
	if ((outfile = fopen(outputfilename.c_str(), "wb")) == NULL) {
	fprintf(stderr, "can't open %s for writing\n", outputfilename.c_str());
	exit(1);
	}
	jpeg_stdio_dest(&cinfo, outfile);

	/* Step 3: set parameters for compression */

	/* First we supply a description of the input image.
	* Four fields of the cinfo struct must be filled in:
	*/
	cinfo.image_width = xsize; 	/* image width and height, in pixels */
	cinfo.image_height = ysize;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	/* Now use the library's routine to set default compression parameters.
	* (You must set at least cinfo.in_color_space before calling this,
	* since the defaults depend on the source color space.)
	*/
	jpeg_set_defaults(&cinfo);
	/* Now you can set any non-default parameters you wish to.
	* Here we just illustrate the use of quality (quantization table) scaling:
	*/
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	/* Step 4: Start compressor */

	/* TRUE ensures that we will write a complete interchange-JPEG file.
	* Pass TRUE unless you are very sure of what you're doing.
	*/
	jpeg_start_compress(&cinfo, TRUE);

	/* Step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */

	/* Here we use the library's state variable cinfo.next_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	* To keep things simple, we pass one scanline per call; you can pass
	* more if you wish, though.
	*/
	
	tsize_t linebytes = xsize*3; /* JSAMPLEs per row in image */
	JSAMPLE *buf = NULL;
	buf =(JSAMPLE *)malloc(linebytes);
	for (int j = 0; j < ysize; j++)
	{
		for(int i = 0; i < xsize; i ++)
		{
            buf[i*3  ] = dither_round(output(j,i*3  ));
            buf[i*3+1] = dither_round(output(j,i*3+1));
            buf[i*3+2] = dither_round(output(j,i*3+2));
		}
		(void) jpeg_write_scanlines(&cinfo,&buf,1);
	}
	
	/* Step 6: Finish compression */

	jpeg_finish_compress(&cinfo);
	/* After finish_compress, we can close the output file. */
	fclose(outfile);

	/* Step 7: release JPEG compression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_compress(&cinfo);
	
    if (writeExif)
    {
        cleanExif(exifData);

        exifData["Exif.Image.Orientation"] = uint16_t(1);//set all images to unrotated
        exifData["Exif.Image.ImageWidth"] = output.nc()/3;
        exifData["Exif.Image.ImageLength"] = output.nr();
        exifData["Exif.Photo.ColorSpace"] = 1;
        exifData["Exif.Image.ProcessingSoftware"] = "Filmulator";

        //Fix the thumbnails
        auto thumb = Exiv2::ExifThumb(exifData);
        thumb.erase();
        //Find the thumbnail for this image
        //thumb.setJpegThumbnail(thumbPath);

        cout << "imwrite_jpeg exiv filename: " << outputfilename << endl;
        auto image = Exiv2::ImageFactory::open(outputfilename);
        assert(image.get() != 0);

        image->setExifData(exifData);
        image->writeMetadata();
    }
	
	return 0;
}

JSAMPLE dither_round(int full_int)
{
	float intermediate = full_int;
	intermediate = ((intermediate + 1)/256) -1; // converted to output range
	float dither = rand();
	dither = dither/RAND_MAX; //produces 0<=dither<=1 
	dither = 0.9999*dither; //dither now cannot cause rounding one integer to the next
	return (JSAMPLE)(intermediate + dither);
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
