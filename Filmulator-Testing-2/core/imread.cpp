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
#include "filmSim.hpp"

bool imread( string input_image_filename, matrix<float> &returnmatrix,
             Exiv2::ExifData &exifData, int highlights, bool caEnabled, bool lowQuality )
{
    //Create image processor for reading raws.
    LibRaw image_processor;

    //Open the file.
    const char *cstr = input_image_filename.c_str();
    if ( 0 != image_processor.open_file( cstr ) )
    {
        cerr << "Could not read input file!" << endl;
        return true;
    }
     //Make abbreviations for brevity in accessing data.
#define SIZES image_processor.imgdata.sizes
#define PARAM image_processor.imgdata.params
#define IMAGE image_processor.imgdata.image
#define COLOR image_processor.imgdata.color

    //Now we'll set demosaic and other processing settings.
    PARAM.user_qual = 10;//10 is AMaZE; -q[#] in dcraw
    PARAM.no_auto_bright = 1;//Don't autoadjust brightness (-W)
    PARAM.output_bps = 16;//16 bits per channel (-6)
    PARAM.gamm[ 0 ] = 1;
    PARAM.gamm[ 1 ] = 1;//Linear gamma (-g 1 1)
    PARAM.ca_correc = caEnabled;//Turn on CA auto correction
    PARAM.cared = 0;
    PARAM.cablue = 0;
    PARAM.output_color = 1;//1: Use sRGB regardless.
    PARAM.use_camera_wb = 1;//1: Use camera WB setting (-w)
    PARAM.highlight = highlights;//Set highlight recovery (-H #)

    if ( lowQuality )
    {
        PARAM.half_size = 1;//half-size output, should dummy down demosaic.
        //PARAM.user_qual = 0;//may not be necessary
        PARAM.ca_correc = 0;//turn off auto CA correction.
    }

    //This makes IMAGE contains the sensel value and 3 blank values at every
    //location.
    image_processor.unpack();

    //This calls the dcraw processing on the raw sensel data.
    //Now, it contains 3 color values and one blank value at every location.
    //We will ignore the last blank value.
    image_processor.dcraw_process();

    long rSum = 0, gSum = 0, bSum = 0;
    returnmatrix.set_size( SIZES.iheight, SIZES.iwidth*3 );
    for ( int row = 0; row < SIZES.iheight; row++ )
    {
        //IMAGE is an (width*height) by 4 array, not width by height by 4.
        int rowoffset = row*SIZES.iwidth;
        for ( int col = 0; col < SIZES.iwidth; col++ )
        {
            returnmatrix( row, col*3     ) = IMAGE[ rowoffset + col ][ 0 ];//R
            returnmatrix( row, col*3 + 1 ) = IMAGE[ rowoffset + col ][ 1 ];//G
            returnmatrix( row, col*3 + 2 ) = IMAGE[ rowoffset + col ][ 2 ];//B
            rSum += IMAGE[ rowoffset + col ][ 0 ];
            gSum += IMAGE[ rowoffset + col ][ 1 ];
            bSum += IMAGE[ rowoffset + col ][ 2 ];
        }
    }
    cout << "imread color outputs" << endl;
    double dim = SIZES.iheight * SIZES.iwidth;
    cout << double(rSum) / dim << " ";
    cout << double(gSum) / dim << " ";
    cout << double(bSum) / dim << endl;


    image_processor.recycle();
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open( cstr );
    assert( image.get() != 0 );
    image->readMetadata();
    exifData = image->exifData();

        return false;
}
