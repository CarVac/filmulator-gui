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
        thumb.setJpegThumbnail(thumbPath);

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
