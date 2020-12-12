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

bool imread_jpeg(string input_image_filename, matrix<float> &returnmatrix,
		Exiv2::ExifData &exifData)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    FILE* jpeg = fopen(input_image_filename.c_str(), "r");
    if (!jpeg)
	{
        cerr << "imread_jpeg: Could not read input file!" << endl;
        return true;
	}
	/* here we set up the standard libjpeg error handler */
	cinfo.err = jpeg_std_error( &jerr );
	/* setup decompression process and source, then read JPEG header */
	jpeg_create_decompress( &cinfo );
	/* this makes the library read from infile */
	jpeg_stdio_src( &cinfo, jpeg );
	/* reading the image header which contains image information */
	jpeg_read_header( &cinfo, TRUE );
	int imagelength = cinfo.image_height;
	int imagewidth = cinfo.image_width;
	int num_chan = 3;
	/* Start decompression jpeg here */
	jpeg_start_decompress( &cinfo );
	/* Allocate resulting image */
	returnmatrix.set_size(imagelength,imagewidth*num_chan);
	/* now actually read the jpeg into the raw buffer */
	row_pointer[0] =
		(unsigned char *)malloc( cinfo.output_width*cinfo.num_components );
	float read_in; //Contains a 0 to 1 value from the jpeg
	/* read one scan line at a time */
	for (int row = 0; row < imagelength; row++)
	{
		jpeg_read_scanlines( &cinfo, row_pointer, 1 );
		for(int col = 0; col < imagewidth; col++)
		{
			for(int channel = 0; channel < 3; channel++)
			{
				read_in = float(row_pointer[0][col*num_chan + channel])/255;
				/*Apply reverse sRGB gamma transformation:
				http://en.wikipedia.org/wiki/SRGB#The_reverse_transformation*/
				if( read_in < 0.04045)
				{
					returnmatrix(row,col*num_chan + channel) =
						read_in*5072.36842105; //(2^16 -1)/12.92 
				}
				else
				{
					returnmatrix(row,col*num_chan + channel) =
						65535*pow((read_in+0.055)/1.055, 2.4);
				}
			}
		}
	}

    cout << "imread_jpeg exiv filename: " << input_image_filename << endl;
    auto image = Exiv2::ImageFactory::open(input_image_filename);
	assert(image.get() != 0);
    image->readMetadata();
	exifData = image->exifData();

	return false;
}
