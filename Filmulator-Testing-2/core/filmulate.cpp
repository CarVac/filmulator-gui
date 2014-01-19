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
#include <algorithm>
#include <stdio.h>
#include <unistd.h>


//Function-------------------------------------------------------------------------
matrix<float> filmulate(matrix<float> &input_image,
        float initial_developer_concentration,
        float reservoir_size,
        float developer_thickness,
        float crystals_per_pixel,
        float initial_crystal_radius,
        float initial_silver_salt_density,
        float developer_consumption_const,
        float crystal_growth_const,
        float silver_salt_consumption_const,
        float total_development_time,
        int agitate_count,
        int development_resolution,
        float film_area,
        float sigma_const,
        float layer_mix_const,
        float layer_time_divisor,
        int rolloff_boundary)
{
    //Timers
    struct timeval initialize_start, development_start, develop_start,
                   diffuse_start, agitate_start, layer_mix_start;
    double develop_dif = 0, diffuse_dif = 0, agitate_dif = 0, layer_mix_dif= 0;           
    gettimeofday(&initialize_start,NULL);

    int nrows = (int) input_image.nr();
    int ncols = (int) input_image.nc()/3;
    int npix = nrows*ncols;

    //Now we activate some of the crystals on the film. This is literally
    //akin to exposing film to light.
    matrix<float> active_crystals_per_pixel;
    active_crystals_per_pixel = exposure(input_image, crystals_per_pixel,
            rolloff_boundary);
    input_image.free();
 
    //We set the crystal radius to a small seed value for each color.
    matrix<float> crystal_radius;
    crystal_radius.set_size(nrows,ncols*3);
    crystal_radius = initial_crystal_radius;

    //All layers share developer, so we only make it the original image size.
    matrix<float> developer_concentration;
    developer_concentration.set_size(nrows,ncols);
    developer_concentration = initial_developer_concentration;

    //Each layer gets its own silver salt which will feed crystal growth.
    matrix<float> silver_salt_density;
    silver_salt_density.set_size(nrows,ncols*3);
    silver_salt_density = initial_silver_salt_density;

    //Now, we set up the reservoir.
    //Because we don't want the film area to influence the brightness, we
    // increase the reservoir size in proportion.
#define FILMSIZE 864;//36x24mm
    reservoir_size *= film_area/FILMSIZE;
    float reservoir_developer_concentration = initial_developer_concentration;

    //This is a value used in diffuse to set the length scale.
    float pixels_per_millimeter = sqrt(npix/film_area);

    //Here we do some math for the control logic for the differential
    //equation approximation computations.
    float timestep = total_development_time/development_resolution;
	int agitate_period;
	if(agitate_count > 0)
		agitate_period = floor(development_resolution/agitate_count);
    else
		agitate_period = 3*development_resolution;
	int half_agitate_period = floor(agitate_period/2);
   
    tout << "Initialization time: " << time_diff(initialize_start)
         << " seconds" << endl;
    gettimeofday(&development_start,NULL);

#ifdef DOUT
    ofstream debug_out;
    debug_out.open("debug_output.txt",ios::out);
#endif

    //Now we begin the main development/diffusion loop, which approximates the
    //differential equation of film development.
    for(int i = 0; i <= development_resolution; i++)
    {
        cout << "====== Developing..." << setw(3)<< 100*i/(development_resolution+1)
            << "% complete ======" <<endl;
        gettimeofday(&develop_start,NULL);
        
        //This is where we perform the chemical reaction part.
        //The crystals grow.
        //The developer in the active layer is consumed.
        //So is the silver salt in the film.
        // The amount consumed increases as the crystals grow larger.
        //Because the developer and silver salts are consumed in bright regions,
        // this reduces the rate at which they grow. This gives us global
        // contrast reduction.
        develop(crystal_radius,crystal_growth_const,active_crystals_per_pixel,
                silver_salt_density,developer_concentration,
                developer_thickness,developer_consumption_const,
                silver_salt_consumption_const,timestep);
        
        develop_dif += time_diff(develop_start);
        gettimeofday(&diffuse_start,NULL);

        //Now, we are going to perform the diffusion part.
        //Here we mix the layer among itself, which grants us the
        // local contrast increases.
        diffuse(developer_concentration,
                reservoir_developer_concentration,
                sigma_const,
                pixels_per_millimeter,
                timestep);

        diffuse_dif += time_diff(diffuse_start);

        gettimeofday(&layer_mix_start,NULL);        
        //This performs mixing between the active layer adjacent to the film
        // and the reservoir.
        //This keeps the effects from getting too crazy.
        layer_mix(developer_concentration,
                  developer_thickness,
                  reservoir_developer_concentration,
                  reservoir_size,
                  layer_mix_const,
                  layer_time_divisor,
                  pixels_per_millimeter,
                  timestep);
        
        layer_mix_dif += time_diff(layer_mix_start);
        
        gettimeofday(&agitate_start,NULL);
        
        //I want agitation to only occur in the middle of development, not
        //at the very beginning or the ends. So, I add half the agitate
        //period to the current cycle count.
        if((i+half_agitate_period) % agitate_period ==0)
            agitate(developer_concentration, developer_thickness,
                    reservoir_developer_concentration, reservoir_size,
                    pixels_per_millimeter);
       
        agitate_dif += time_diff(agitate_start);
#ifdef DOUT
        float mean_developer_concentration = mean(developer_concentration);
        float mean_crystal_radius = mean(crystal_radius);
        float mean_silver_salt_density = mean(silver_salt_density);
        debug_out << mean_developer_concentration << ","
                  << mean_crystal_radius << ","
                  << mean_silver_salt_density << ","
                  << reservoir_developer_concentration << endl;
        dout << "After agitate: developer concentration = " << mean_developer_concentration << endl;
#endif
    }
    cout<< "====== Development  100% complete ======" << endl;
    tout<<"Development time: "<<time_diff(development_start)<<" seconds"<<endl;
    tout << "Develop time: " << develop_dif << " seconds" << endl;
    tout << "Diffuse time: " << diffuse_dif << " seconds" << endl;
    tout << "Layer mix time: " << layer_mix_dif << " seconds" << endl;
    tout << "Agitate time: " << agitate_dif << " seconds" << endl;
    
    //Done filmulating, now do some housecleaning
    silver_salt_density.free();
    developer_concentration.free();


    //Now we compute the density (opacity) of the film.
    //We assume that overlapping crystals or dye clouds are
    //nonexistant. It works okay, for now...
    //The output is crystal_radius^2 * active_crystals_per_pixel
    struct timeval mult_start;
    gettimeofday(&mult_start,NULL);
    matrix<float> output_density;
    output_density = crystal_radius % crystal_radius % active_crystals_per_pixel;
    tout << "Output density time: "<<time_diff(mult_start) << endl;
#ifdef DOUT
    debug_out.close();
#endif
	return output_density;
}

