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

bool imwrite_tiff(matrix<int> &output_r, matrix<int> &output_g,
        matrix<int> &output_b, string outputfilename, Exiv2::ExifData exifData)
{
    int xsize, ysize;

    //This is to eliminate a bug whereby some image viewers (specifically
    // RawTherapee's image browser) rotate portrait images incorrectly.
    //Here I'm just going to make it read out directly.
    switch((int) exifData["Exif.Image.Orientation"].value().toLong())
    {
        case 6://right side down; camera resting on right hand
        case 8://left side down; camera hanging from right hand
            xsize = output_r.nr();
            ysize = output_r.nc();
            exifData["Exif.Image.ImageWidth"] = output_r.nr();
            exifData["Exif.Image.ImageLength"] = output_r.nc();
            break;
        default://normal; we ignore all other orientations.
            xsize = output_r.nc();
            ysize = output_r.nr();
    }
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
    switch((int) exifData["Exif.Image.Orientation"].value().toLong())
    {
        case 6://right side down
            for (int j = 0; j < ysize; j++)
            {
                for (int i = 0; i < xsize; i++)
                {
                    buf[i*3  ] = (unsigned short) output_r(xsize-1-i,j);
                    buf[i*3+1] = (unsigned short) output_g(xsize-1-i,j);
                    buf[i*3+2] = (unsigned short) output_b(xsize-1-i,j);
                }
                if (TIFFWriteScanline(out, buf, j, 0) <0)
                    break;
            }
            break;
        case 8://left side down
            for (int j = 0; j < ysize; j++)
            {
                for (int i = 0; i < xsize; i++)
                {
                    buf[i*3  ] = (unsigned short) output_r(i,ysize-1-j);
                    buf[i*3+1] = (unsigned short) output_g(i,ysize-1-j);
                    buf[i*3+2] = (unsigned short) output_b(i,ysize-1-j);
                }
                if (TIFFWriteScanline(out, buf, j, 0) <0)
                    break;
            }
            break;
        default:
            for (int j = 0; j < ysize; j++)
            {
                for(int i = 0; i < xsize; i ++)
                {
                    buf[i*3  ] = (unsigned short) output_r(j,i);
                    buf[i*3+1] = (unsigned short) output_g(j,i);
                    buf[i*3+2] = (unsigned short) output_b(j,i);
                }
                if (TIFFWriteScanline(out, buf, j, 0) < 0)
                    break;
            }
    }
    (void) TIFFClose(out);

    if (buf)
        _TIFFfree(buf);

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(outputfilename.c_str());
    assert(image.get() != 0);

    exifData["Exif.Image.Orientation"] = 1;//set all images to unrotated
    image->setExifData(exifData);
    image->writeMetadata();

    return 0;
}
