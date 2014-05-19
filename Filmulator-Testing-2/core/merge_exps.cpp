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

// This file contains the function merge_exps, which merges exposures
// intelligently for HDR. It takes in linear brightness data and when neither
// exposure is clipped it performs a weighted average based on relative
// brightness and how many exposures have been averaged before.
#include "filmsim.hpp"
#include <stdlib.h>
#include <time.h>

bool merge_exps(matrix<float> &input_image, const matrix<float> &temp_image,
        float &exposure_weight, float initial_exposure_comp,
        float &last_exposure_factor, string filename,
        float input_exposure_comp)
{
//First order of business is to determine the relative brightness of the two
//images (we don't trust the exposure compensation value given by the user to
//be correct or even if it is, perfectly accurate).
    int numrows = input_image.nr();
    int numcols = input_image.nc();
    double init_exp_factor = pow(2,-initial_exposure_comp);
    double input_exp_factor = pow(2,-input_exposure_comp);
    double input_sum = init_exp_factor;//only initially
    double exp_factor;
    double temp_sum = 0;
    int i = 0;
    srand(time(NULL));
    int xcoord;
    int ycoord;
    //This samples points that are valid in both images 
    while (i < max(max(numrows, numcols/3),300))
    {
        xcoord = int(fmod(double(rand())*double(rand()), double(numcols/3)));
        ycoord = int(fmod(double(rand())*double(rand()), double(numrows)));
        if(input_image(ycoord,xcoord*3)/init_exp_factor < 60000 &&
                input_image(ycoord,xcoord*3+1)/init_exp_factor < 60000 &&
                input_image(ycoord,xcoord*3+2)/init_exp_factor < 60000 &&
                temp_image(ycoord,xcoord*3) < 60000 &&
                temp_image(ycoord,xcoord*3+1) < 60000 &&
                temp_image(ycoord,xcoord*3+2) < 60000)
        {
            input_sum += double(input_image(ycoord,xcoord*3))
                + double(input_image(ycoord,xcoord*3+1))
                + double(input_image(ycoord,xcoord*3+2));
            temp_sum += double(temp_image(ycoord,xcoord*3))
                + double(temp_image(ycoord,xcoord*3+1))
                + double(temp_image(ycoord,xcoord*3+2));
            i++;
        }
    }
    exp_factor = input_sum/temp_sum;
    //This checks that the image is brighter than the previous one, within 5%
    //error.
    if(exp_factor > (last_exposure_factor*1.05))
    {
        cerr << "Image " << filename << " is darker than the previous image. Exiting..." << endl;
        return true;
    }
    //Now we merge the exposures.
    //The weighting of the new image is the ratio of the first exposure factor
    //to the current exposure factor.
    //The first exposure factor should be the biggest, so the greater the
    //ratio the more the weighting should be.
    float new_exp_weight = init_exp_factor/exp_factor;
    int temp_max_channel;
    int clip_thresh = 61000;//If any channel exceeds this value, we start
                            // to roll off its weight.
    float exp_weight_factor;//This will store the weighting for the rolloff.
    int colr, colg, colb;//hold indices for red, green, and blue columns
    for (int col = 0; col < numcols/3; col++)
    {
        colr = col*3;
        colg = col*3+1;
        colb = col*3+2;
        for (int row = 0; row < numrows; row++)
        {
           temp_max_channel = max(max(temp_image(row,colr),
                       temp_image(row,colg)),temp_image(row,colb));
        // Here we compute the weighting
           exp_weight_factor = min(1,max(clip_thresh-temp_max_channel,0)/1000);

           //Here we do a weighted average of the cumulative exposure
           //and the corrected (multiplied by exp_factor) exposure from the
           //more brightly exposed image.
           input_image(row,colr) = (input_image(row,colr)*
                   exposure_weight + temp_image(row,colr)*exp_factor*
                   new_exp_weight*exp_weight_factor) /
               (exposure_weight + new_exp_weight*exp_weight_factor);
           input_image(row,colg) = (input_image(row,colg)*
                   exposure_weight + temp_image(row,colg)*exp_factor*
                   new_exp_weight*exp_weight_factor) /
               (exposure_weight + new_exp_weight*exp_weight_factor);
           input_image(row,colb) = (input_image(row,colb)*
                   exposure_weight + temp_image(row,colb)*exp_factor*
                   new_exp_weight*exp_weight_factor) /
               (exposure_weight + new_exp_weight*exp_weight_factor);
        }
    }
    last_exposure_factor = input_exp_factor;//TODO: Questionable if correct.
    exposure_weight = exposure_weight + new_exp_weight;
    return false;
}
