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
// function and loads the data into a matrix. If the length of the filename
// list is greater than one, it uses that and the input exposure compensation
// to merge the files into one HDR image.
#include "filmsim.hpp"

bool imload(std::vector<string> input_filename_list,
        std::vector<float> input_exposure_compensation,
        matrix<float> &input_image,
		bool tiff, bool jpeg_in, Exiv2::ExifData &exifData, int highlights)
{
// If there is only one filename, then simply read it in and apply exposure
// compensation. If there are more than one, read the first one (the brightest
// one) in before moving on.
	if(tiff)
	{
        if(imread_tiff(input_filename_list[0], input_image, exifData))
        {
            cout << "Could not open image " << input_filename_list[0] <<
                "; Exiting..." << endl;
            return true;
        }

	}
	else if(jpeg_in)
	{
		if(imread_jpeg(input_filename_list[0], input_image, exifData))
		{
			cout << "Could not open image " << input_filename_list[0] <<
				"; Exiting..." << endl;
			return true;
		}
	}
	else//raw
	{
        if(imread(input_filename_list[0], input_image, exifData, highlights))
        {
            cout << "Could not open image " << input_filename_list[0] <<
                "; Exiting..." << endl;
            return true;
        }
    }
    input_image *= pow(2,-input_exposure_compensation[0]);
    // This line filters out non-HDR images.
    if(input_filename_list.size()==1)
    {
        return false;
    }

    //This line sanitizes the input; if the input exposure compensations are
    //not in increasing order (or at least identical) it rejects them.
    for ( int i=1; i < input_exposure_compensation.size(); i++)
    {
        if (input_exposure_compensation[i-1] > input_exposure_compensation[i])
        {
            cout << "HDR exposures must be in ascending order of brightness. Exiting" << endl;
            return true;
        }
    }
    //Here we make some temporary variables for reading in a second image.
    matrix<float> temp_image;
    //This next variable is for making weighted averaging when merging
    //exposures. 
    float exposure_weight = 1;
    //This next variable is for making sure that the images actually are in
    //increasing order of brightness.
    float last_exposure_factor = pow(2,-input_exposure_compensation[0]);
    for ( int i=1; i < input_filename_list.size(); i++)
    {
        cout << "Reading image " << input_filename_list[i] << "." << endl;
		if(tiff)
		{
			if(imread_tiff(input_filename_list[i], temp_image, exifData))
			{
				cout << "Could not open image " << input_filename_list[i] <<
					"; Exiting..." << endl;
				return true;
			}
		}
        else//raw
        {
            if(imread(input_filename_list[i], temp_image, exifData, 0))
            {
                cout << "Could not open image " << input_filename_list[i] <<
                    "; Exiting..." << endl;
                return true;
            }
        }

        if(input_image.nr()!=temp_image.nr() ||
                input_image.nc()!=temp_image.nc())
        {
            cout << "Image " << input_filename_list[i] <<
                " has mismatching image size;" << endl << "Exiting..." <<
                endl;
            return true;
        }
        if (merge_exps(input_image, temp_image,
                exposure_weight, input_exposure_compensation[0],
                last_exposure_factor, input_filename_list[i],
                input_exposure_compensation[i]))
        {
            return true;
        }
    }
    return false;
}
