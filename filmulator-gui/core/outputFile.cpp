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

void output_file(matrix<unsigned short> &output, vector<string> input_filename_list,
                 bool jpeg_out, Exiv2::ExifData exifData)
{
    struct timeval write_start;
    gettimeofday(&write_start,NULL);    

    string output_image_filename = input_filename_list[0];
    int namelength = output_image_filename.length();
    int extension_pos = namelength;
    for(;extension_pos >= 0;extension_pos--)//Find extension so that it can be cleaved
        if(output_image_filename[extension_pos] == '.')
            break;
    if(extension_pos == 0) //no extension
        extension_pos = namelength;
    output_image_filename = output_image_filename.substr(0,extension_pos);
    if(input_filename_list.size() != 1)
    {
        output_image_filename = output_image_filename + "-HDR";
    }
    output_image_filename = output_image_filename + "-output";

    
    if (jpeg_out)
    {
        imwrite_jpeg(output,output_image_filename,exifData);
    }
    else
    {
        imwrite_tiff(output, output_image_filename,exifData);
    }

    tout << "Write time: " << timeDiff(write_start) << " seconds" << endl;
}
