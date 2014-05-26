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

bool imwrite_tiff(matrix<unsigned short> output, string outputfilename,
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

    exifData["Exif.Image.Orientation"] = 1;//set all images to unrotated
    exifData["Exif.Image.ImageWidth"] = output.nr();
    exifData["Exif.Image.ImageLength"] = output.nc()/3;

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(outputfilename.c_str());
    assert(image.get() != 0);

    image->setExifData(exifData);
    image->writeMetadata();

    return 0;
}
