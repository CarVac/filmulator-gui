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
#include "string.h"
#include <sys/time.h>

int main(int argc, char* argv[])
{
    //Setup for timing
    struct timeval total_start;
    gettimeofday(&total_start,NULL);

    string input_configuration;
    std::vector<string> input_filename_list;
    std::vector<float> input_exposure_compensation;
    int hdr_count = 0;
	bool hdr = false;
	bool tiff = false;
	bool jpeg_in = false;
    bool slide_reversal = false;
    bool set_whitepoint = false;
	float whitepoint;
	bool jpeg_out = false;
    bool tonecurve_out = false;
    int highlights = 0; //dcraw highlight recovery parameter

        
    if(read_args(argc,
                 argv,
                 input_configuration,
                 input_filename_list,
                 input_exposure_compensation,
                 hdr_count,
                 hdr,
                 tiff,
                 jpeg_in,
                 set_whitepoint,
                 whitepoint,
                 jpeg_out,
                 tonecurve_out,
                 highlights))
    {
        return 1;
    }

	float initial_developer_concentration;
    float reservoir_size;
    float developer_thickness;
    float crystals_per_pixel;
    float initial_crystal_radius;
    float initial_silver_salt_density;
    float developer_consumption_const;
    float crystal_growth_const;
    float silver_salt_consumption_const;
    int   total_development_time;
    int   agitate_count;
    float development_resolution;
    float film_area;
    float sigma_const;
    float layer_mix_const;
    float layer_time_divisor;
    float std_cutoff;
	string tempdir;
    int rolloff_boundary;
    
    initialize(input_configuration,
            initial_developer_concentration,
            reservoir_size,
            developer_thickness,
            crystals_per_pixel,
            initial_crystal_radius,
            initial_silver_salt_density,
            developer_consumption_const,
            crystal_growth_const,
            silver_salt_consumption_const,
            total_development_time,
            agitate_count,
            development_resolution,
            film_area,
            sigma_const,
            layer_mix_const,
            layer_time_divisor,
            std_cutoff,
            rolloff_boundary);

    //Set up for timing
    struct timeval read_start;
    gettimeofday(&read_start,NULL);

    //Load image
    matrix<float> input_image;
	Exiv2::ExifData exifData;
    if(imload(input_filename_list, input_exposure_compensation,
                input_image, tiff, jpeg_in, exifData, highlights))
    {
        cout << "Error loading images" << endl;
        return 1;
    }
	exifData["Exif.Image.ProcessingSoftware"] = "Filmulator"; 
    
    tout << "Read time: " << time_diff(read_start) << " seconds" << endl;
    cout << "Image loaded." << endl;

    matrix<float> output_density = filmulate(input_image,
                                            initial_developer_concentration,
                                            reservoir_size,
                                            developer_thickness,
                                            crystals_per_pixel,
                                            initial_crystal_radius,
                                            initial_silver_salt_density,
                                            developer_consumption_const,
                                            crystal_growth_const,
                                            silver_salt_consumption_const,
                                            total_development_time,
                                            agitate_count,
                                            development_resolution,
                                            film_area,
                                            sigma_const,
                                            layer_mix_const,
                                            layer_time_divisor,
                                            rolloff_boundary);

    
    //Postprocessing: normallize and apply a tone curve
    int nrows = output_density.nr();
    int ncols = output_density.nc()/3;
	matrix<int> output_r(nrows,ncols);
	matrix<int> output_g(nrows,ncols);
	matrix<int> output_b(nrows,ncols);
	
    postprocess(output_density, set_whitepoint, whitepoint, tonecurve_out,
                std_cutoff, output_r,output_g,output_b);
    	
    //Output the file
    output_file(output_r, output_g, output_b, input_filename_list, jpeg_out,
                exifData);

    tout << "Total time: " << time_diff(total_start) << " seconds" << endl;

	return 0;
}
