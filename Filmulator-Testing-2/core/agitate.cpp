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

void agitate ( matrix<float> &developer_concentration,
    float developer_thickness, float &reservoir_developer_concentration,
    float reservoir_size, float pixels_per_millimeter )
{
    int npixels = developer_concentration.nc()*
        developer_concentration.nr();
    float total_developer = sum(developer_concentration)*
        developer_thickness/pow(pixels_per_millimeter,2) +
        reservoir_developer_concentration*reservoir_size;
    float contact_layer_size = npixels*developer_thickness/
        pow(pixels_per_millimeter,2);
    reservoir_developer_concentration = total_developer/(reservoir_size +
        contact_layer_size);
    developer_concentration = reservoir_developer_concentration;
    return;
}
