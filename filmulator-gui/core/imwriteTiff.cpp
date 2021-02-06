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
    if(!TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE)) {cout << "couldn't set tiff compression" << endl;}
    //End Magic

    std::string make = exifData["Exif.Image.Make"].toString();
    TIFFSetField(out, TIFFTAG_MAKE, make.c_str());
    std::string model = exifData["Exif.Image.Model"].toString();
    TIFFSetField(out, TIFFTAG_MODEL, model.c_str());
    TIFFSetField(out, TIFFTAG_SOFTWARE, "Filmulator");
    std::string copyright = exifData["Exif.Image.Copyright"].toString();
    TIFFSetField(out, TIFFTAG_COPYRIGHT, copyright.c_str());
    std::string lensinfo = exifData["Exif.Image.LensInfo"].toString();
    TIFFSetField(out, TIFFTAG_LENSINFO, lensinfo.c_str());
    std::string datetime = exifData["Exif.Image.DateTime"].toString();
    TIFFSetField(out, TIFFTAG_DATETIME, datetime.c_str());

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

    TIFFWriteDirectory(out);

    (void) TIFFClose(out);

    if (buf)
        _TIFFfree(buf);

    //Programs reading tiffs really freak out if you write the full exif data
    //So we write only the basics here.
    Exiv2::ExifData minimumData;
    minimumData["Exif.Photo.ISOSpeedRatings"] = exifData["Exif.Photo.ISOSpeedRatings"];
    minimumData["Exif.Photo.ExposureTime"]    = exifData["Exif.Photo.ExposureTime"];
    minimumData["Exif.Photo.FNumber"]         = exifData["Exif.Photo.FNumber"];
    minimumData["Exif.Photo.FocalLength"]     = exifData["Exif.Photo.FocalLength"];

    minimumData.sortByTag(); //darktable's does this so maybe we should

    auto image = Exiv2::ImageFactory::open(outputfilename);
    assert(image.get() != 0);

    image->setExifData(minimumData);
    image->writeMetadata();

    return 0;
}
