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

void postprocess(matrix<float> &output_density, bool set_whitepoint,
                        float whitepoint, bool tonecurve_out, float std_cutoff,
                        matrix<int> &output_r, matrix<int> &output_g,
                        matrix<int> &output_b)
{
    struct timeval postprocess_start;
    gettimeofday(&postprocess_start,NULL);
    float output_max = whitepoint;
    if(!set_whitepoint)
    {
        float mean_val = mean(output_density);
        float std = sqrt(variance(output_density));
        float max_image = max(output_density);
        float max_allowed = mean_val + std_cutoff*std;

        if(max_image < max_allowed)
            output_max = max_image;
        else
            output_max = max_allowed;
	}
    matrix<float> tempMatrix = output_density * 255/output_max;

    //Tone curve        
	LUT lookup;
    //Filling a lookup table.
	if(tonecurve_out)
		lookup.fill(&tonecurve);
	else
		lookup.fill(&flatcurve);

    //Here we apply the tone curve.
    //Here, we split the interleaved matrix into three separate ones.
    apply_tone_curve(lookup,tempMatrix,output_r,output_g,output_b);
    tout << "Postprocess time: " 
         << time_diff(postprocess_start) << " seconds" << endl;
}
