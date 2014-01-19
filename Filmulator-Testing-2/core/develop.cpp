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

void develop(matrix<float> &crystal_radius,
        float crystal_growth_const,
		const matrix<float> &active_crystals_per_pixel,
        matrix<float> &silver_salt_density,
        matrix<float> &developer_concentration, float developer_thickness,
        float developer_consumption_const, float silver_salt_consumption_const,
        float timestep)
{

    //Setting up dimensions and boundaries.
    int height = developer_concentration.nr();
    int width = developer_concentration.nc();
    //We still count columns of pixels, because we must process them
    // whole, so to ensure this we runthree adjacent elements at a time.
    
    //Here we pre-compute some repeatedly used values.
    float cgc = crystal_growth_const*timestep;
    float dcc = 2.0*developer_consumption_const/(developer_thickness*3.0);
    float sscc = silver_salt_consumption_const*2.0;

    //These are only used once per loop, so they don't have to be matrices.
    float delta_crystal_radiusr;
    float delta_crystal_radiusg;
    float delta_crystal_radiusb;
    float delta_crystal_volumer;
    float delta_crystal_volumeg;
    float delta_crystal_volumeb;

    //These are the column indices for red, green, and blue.
    int row, col, colr, colg, colb;

#pragma omp parallel shared(developer_concentration,silver_salt_density,\
        crystal_radius,active_crystals_per_pixel,cgc,dcc,sscc) private(row,\
            col,colr,colg,colb,delta_crystal_radiusr,delta_crystal_radiusg,\
            delta_crystal_radiusb,delta_crystal_volumer,delta_crystal_volumeg,\
            delta_crystal_volumeb)
    {

#pragma omp for schedule(dynamic) nowait
        for (row = 0; row < height; row++)
        {
            for (col = 0; col < width; col++)
            {
                colr = col*3;
                colg = colr+1;
                colb = colr+2;
                //This is the rate of thickness accumulating on the crystals.
                delta_crystal_radiusr = developer_concentration(row,col)*
                    silver_salt_density(row,colr)*cgc;
                delta_crystal_radiusg = developer_concentration(row,col)*
                    silver_salt_density(row,colg)*cgc;
                delta_crystal_radiusb = developer_concentration(row,col)*
                    silver_salt_density(row,colb)*cgc;
    
                //The volume change is proportional to 4*pi*r^2*dr.
                //We kinda shuffled around the constants, so ignore the lack of
                //the 4 and the pi.
                //However, there are varying numbers of crystals, so we also
                //multiply by the number of crystals per pixel.
                delta_crystal_volumer = delta_crystal_radiusr*
                    crystal_radius(row,colr)*crystal_radius(row,colr)*
                    active_crystals_per_pixel(row,colr);
                delta_crystal_volumeg = delta_crystal_radiusg*
                    crystal_radius(row,colg)*crystal_radius(row,colg)*
                    active_crystals_per_pixel(row,colg);
                delta_crystal_volumeb = delta_crystal_radiusb*
                    crystal_radius(row,colb)*crystal_radius(row,colb)*
                    active_crystals_per_pixel(row,colb);
    
                //Now we apply the new crystal radius.
                crystal_radius(row,colr) = crystal_radius(row,colr) +
                    delta_crystal_radiusr;
                crystal_radius(row,colg) = crystal_radius(row,colg) +
                    delta_crystal_radiusg;
                crystal_radius(row,colb) = crystal_radius(row,colb) +
                    delta_crystal_radiusb;
    
                //Here is where we consume developer. The 3 layers of film,
                //(one per color) share the same developer.
                developer_concentration(row,col) =
                    developer_concentration(row,col) -
                    dcc*(
                            delta_crystal_volumer +
                            delta_crystal_volumeg +
                            delta_crystal_volumeb);
                if (developer_concentration(row,col) < 0)
                    developer_concentration(row,col) = 0;
                //Here, silver salts are consumed in proportion to how much
                //silver was deposited on the crystals. Unlike the developer,
                //each color layer has its own separate amount in this sim.
                silver_salt_density(row,colr) = silver_salt_density(row,colr) -
                    sscc * delta_crystal_volumer;
                silver_salt_density(row,colg) = silver_salt_density(row,colg) -
                    sscc * delta_crystal_volumeg;
                silver_salt_density(row,colb) = silver_salt_density(row,colb) -
                    sscc * delta_crystal_volumeb;
            }
        }
    }
    return;
}
