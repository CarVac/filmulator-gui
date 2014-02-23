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

//imread.cpp uses libraw to load raw files.
#include "filmsim.hpp"

bool imread(string input_image_filename, matrix<float> &returnmatrix,
		Exiv2::ExifData &exifData, int highlights)
{
    //Create image processor for reading raws.
    LibRaw image_processor;
    //Make abbreviations for brevity in accessing data.
#define SIZES image_processor.imgdata.sizes
#define PARAM image_processor.imgdata.params
#define IMAGE image_processor.imgdata.image
#define COLOR image_processor.imgdata.color

    //Now we'll set demosaic and other processing settings.
    PARAM.user_qual = 10;//10 is AMaZE; -q[#] in dcraw
    PARAM.no_auto_bright = 1;//Don't autoadjust brightness (-W)
    PARAM.output_bps = 16;//16 bits per channel (-6)
    PARAM.gamm[0] = 1;
    PARAM.gamm[1] = 1;//Linear gamma (-g 1 1)
    PARAM.ca_correc = 0;//Turn on CA auto correction; disabled for now
    PARAM.use_camera_wb = 1;//Use camera WB setting (-w)
    PARAM.highlight = highlights;//Set highlight recovery (-H #)

    const char *cstr = input_image_filename.c_str();
    if(0 != image_processor.open_file(cstr))
    {
        cerr << "Could not read input file!" << endl;
        return true;
    }
/*
    cout << "used these multipliers: " << COLOR.cam_mul[0] << ", "
            << COLOR.cam_mul[1] << ", "
               << COLOR.cam_mul[2] << ", "
                  << COLOR.cam_mul[3] << endl;
    cout << "Daylight multipliers: " << COLOR.pre_mul[0] << ", "
            << COLOR.pre_mul[1] << ", "
               << COLOR.pre_mul[2] << ", "
                  << COLOR.pre_mul[3] << endl;
                  */
    
    //This makes IMAGE contains the sensel value and 3 blank values at every
    //location.
    image_processor.unpack();

    //This calls the dcraw processing on the raw sensel data.
    //Now, it contains 3 color values and one blank value at every location.
    //We will ignore the last blank value.
    image_processor.dcraw_process();

    returnmatrix.set_size(SIZES.iheight,SIZES.iwidth*3);
    for(int row = 0; row < SIZES.iheight; row++)
    {
        //IMAGE is an (width*height) by 4 array, not width by height by 4.
        int rowoffset = row*SIZES.iwidth;
        for(int col = 0; col < SIZES.iwidth; col++)
        {
            returnmatrix(row,col*3    ) = IMAGE[rowoffset+col][0];//R
            returnmatrix(row,col*3 + 1) = IMAGE[rowoffset+col][1];//G
            returnmatrix(row,col*3 + 2) = IMAGE[rowoffset+col][2];//B
        }
    }
    image_processor.recycle();
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(cstr);
    assert(image.get() != 0);
    image->readMetadata();
    exifData = image->exifData();

	return false;
}
