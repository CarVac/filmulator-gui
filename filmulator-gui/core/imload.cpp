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

// This file contains the function imload, which calls an image retrieval
// function and loads the data into a matrix.
#include "filmSim.hpp"

bool imload(std::string filename,
            matrix<float> &input_image,
            bool tiff, bool jpeg_in, Exiv2::ExifData &exifData, int highlights,
            bool caEnabled, bool lowQuality )
{
    if(tiff)
    {
        if(imread_tiff(filename, input_image, exifData))
        {
            cerr << "Could not open image " << filename <<
                    "; Exiting..." << endl;
            return true;
        }
    }
    else if(jpeg_in)
    {
        if(imread_jpeg(filename, input_image, exifData))
        {
            cerr << "Could not open image " << filename <<
                    "; Exiting..." << endl;
            return true;
        }
    }
    else//raw
    {
        if( imread(filename, input_image, exifData, highlights,
                   caEnabled, lowQuality))
        {
            cerr << "Could not open image " << filename <<
                    "; Exiting..." << endl;
            return true;
        }
    }
    return false;
}
