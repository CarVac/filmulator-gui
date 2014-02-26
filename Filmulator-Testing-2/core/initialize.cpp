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
        filmulateParams &filmParams)
{
    ifstream infile;
    char temp[100]; //Throw away first column (variable names)
    infile.open(input_configuration.c_str(),ios::in);
    
    if (!infile)
    {
        cerr << "Can't open input file " << input_configuration << endl;
        exit(1);
    }

/*    infile >> temp >> filmParams.initial_developer_concentration;
    infile >> temp >> filmParams.reservoir_size;
    infile >> temp >> filmParams.developer_thickness;
    infile >> temp >> filmParams.crystals_per_pixel;
    infile >> temp >> filmParams.initial_crystal_radius;
    infile >> temp >> filmParams.initial_silver_salt_density;
    infile >> temp >> filmParams.developer_consumption_const;
    infile >> temp >> filmParams.crystal_growth_const;
    infile >> temp >> filmParams.silver_salt_consumption_const;
    infile >> temp >> filmParams.total_development_time;
    infile >> temp >> filmParams.agitate_count;
    infile >> temp >> filmParams.development_resolution;
    infile >> temp >> filmParams.film_area;
    infile >> temp >> filmParams.sigma_const;
    infile >> temp >> filmParams.layer_mix_const;
    infile >> temp >> filmParams.layer_time_divisor;
    infile >> temp >> filmParams.std_cutoff;
    infile >> temp >> filmParams.rolloff_boundary;
    */
    
    
    infile.close();
    return;
}
