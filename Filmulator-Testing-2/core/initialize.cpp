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
#include <fstream>

void initialize(string input_configuration,
        float &initial_developer_concentration, float &reservoir_size,
        float &developer_thickness, float &crystals_per_pixel,
        float &initial_crystal_radius, float &initial_silver_salt_density,
        float &developer_consumption_const, float &crystal_growth_const,
        float &silver_salt_consumption_const, int &total_development_time,
        int &agitate_count, float &development_resolution, float &film_area,
        float &sigma_const, float &layer_mix_const, float &layer_time_divisor,
        float &std_cutoff, int &rolloff_boundary)
{
    ifstream infile;
    char temp[100];
    infile.open(input_configuration.c_str(),ios::in);
    
    if (!infile)
    {
        cerr << "Can't open input file " << input_configuration << endl;
        exit(1);
    }
    
    infile >> temp >> initial_developer_concentration;
    infile >> temp >> reservoir_size;
    infile >> temp >> developer_thickness;
    infile >> temp >> crystals_per_pixel;
    infile >> temp >> initial_crystal_radius;
    infile >> temp >> initial_silver_salt_density;
    infile >> temp >> developer_consumption_const;
    infile >> temp >> crystal_growth_const;
    infile >> temp >> silver_salt_consumption_const;
    infile >> temp >> total_development_time;
    infile >> temp >> agitate_count;
    infile >> temp >> development_resolution;
    infile >> temp >> film_area;
    infile >> temp >> sigma_const;
    infile >> temp >> layer_mix_const;
    infile >> temp >> layer_time_divisor;
    infile >> temp >> std_cutoff;
    infile >> temp >> rolloff_boundary;
    
    
    infile.close();
    return;
}
