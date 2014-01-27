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
#include "filmsim.hpp"

bool imread_tiff(string input_image_filename, matrix<float> &returnmatrix,
		Exiv2::ExifData &exifData)
{
    TIFFSetWarningHandler(NULL);
    TIFF* tif = TIFFOpen(input_image_filename.c_str(), "r");
    if (!tif)
	{
        cerr << endl << "Could not read input file!" << endl;
        return true;
	}
	uint32 imagelength;
	uint32 imagewidth;
	uint16 num_chan;//number of color channels
	unsigned short * buf16;
	unsigned char  * buf8;
	uint16 bits_per_sample;

	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &imagelength);
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &imagewidth);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &num_chan);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);

	returnmatrix.set_size(imagelength,imagewidth*3);

    //The matrix is 3x wider than the image, interleaving the channels.
	if(bits_per_sample == 16)
	{
	  buf16 = (unsigned short *)_TIFFmalloc(TIFFScanlineSize(tif));
      for ( unsigned int row = 0; row < imagelength; row++)
	  {
	      TIFFReadScanline(tif, buf16, row);
          for( unsigned int col = 0; col < imagewidth; col++)
		  {
			  returnmatrix(row,col*3    ) = buf16[col*num_chan    ];
			  returnmatrix(row,col*3 + 1) = buf16[col*num_chan + 1];
              returnmatrix(row,col*3 + 2) = buf16[col*num_chan + 2];
		  }
	  }
	  _TIFFfree(buf16);
	}
	else
	{
	  buf8 = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(tif));
      for ( unsigned int row = 0; row < imagelength; row++)
	  {
	      TIFFReadScanline(tif, buf8, row);
          for( unsigned int col = 0; col < imagewidth; col++)
		  {
			  returnmatrix(row,col*3    ) = buf8[col*num_chan    ];
			  returnmatrix(row,col*3    ) *= 257;
			  returnmatrix(row,col*3 + 1) = buf8[col*num_chan + 1];
			  returnmatrix(row,col*3 + 1) *= 257;
              returnmatrix(row,col*3 + 2) = buf8[col*num_chan + 2];
              returnmatrix(row,col*3 + 2) *= 257;
		  }
	  }
	  _TIFFfree(buf8 );
	}
	TIFFClose(tif);

	Exiv2::Image::AutoPtr image =
		Exiv2::ImageFactory::open(input_image_filename.c_str());
	assert(image.get() != 0);
    image->readMetadata();
	exifData = image->exifData();

	return false;
}
