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
#include <utility>
#include <omp.h>
#include "math.h"
#define ORDER 7 //This is the number of box blur iterations.
using namespace std;

//In real film, developer diffuses in a 2D gaussian on the film surface.
//Since 2D gaussians are separable, we perform two 1D gaussians in x and y.
//Since approximating a gaussian with a convolution is fairly computationally
//expensive, we approximate it using repeated box blurs. 

//Helper function to diffuse in x direction
void diffuse_x(matrix<float> &developer_concentration,
               int convlength,
               int convrad, int pad, int paddedwidth, int order,
               float swell_factor);

void diffuse(matrix<float> &developer_concentration, 
		     float sigma_const,
		     float pixels_per_millimeter,
             float timestep)
{
    //This is the standard deviation we want for the blur in pixels.
    float sigma = sqrt(timestep*pow(sigma_const*pixels_per_millimeter,2));
    int order = ORDER;
    
    int length = developer_concentration.nr();
    int width = developer_concentration.nc();

    //Length is the total size of the blur box determined by the desired
    //gaussian size and the number of iterations.
    int convlength = floor(sqrt(pow(sigma,2)*(12/ORDER)+1));
    if (convlength % 2 == 0)
    {
        convlength += 1;
    }

    //convrad is the radius of the convolution box. It's useful later on.
    int convrad = (convlength-1)/2;

    //We will be doing lots of averaging, but holding off on dividing by the
    //number of values averaged. The number of values averaged is always
    //<convlength>^<order>
    float swell_factor = 1.0/pow(convlength,order);

    //Here we allocate a matrix sized the same as a row, plus room for padding
    //mirrored from the internal content.
    int pad = order*convrad;
    int paddedwidth = 2*pad + width;
    int paddedheight = 2*pad + length;

    diffuse_x(developer_concentration,convlength,convrad,pad,paddedwidth,
              order, swell_factor);
    matrix<float> transposed_developer_concentration(width,length);
    developer_concentration.transpose_to(transposed_developer_concentration);
    diffuse_x(transposed_developer_concentration,convlength,convrad,pad,
              paddedheight, order, swell_factor);
    transposed_developer_concentration.transpose_to(developer_concentration);

    return;
}

void diffuse_x(matrix<float> &developer_concentration, int convlength,
               int convrad, int pad, int paddedwidth, int order,
               float swell_factor)
{
    int length = developer_concentration.nr();
    int width = developer_concentration.nc();

    vector<float> hpadded(paddedwidth); //stores one padded line
    vector<float> htemp(paddedwidth); // stores result of box blur

    //This is the running sum used to compute the box blur.
    float running_sum = 0;

    //These are indices for loops.
    int row = 0;
    int col = 0;
    int pass = 0;

#pragma omp parallel shared(developer_concentration,convlength,convrad,order,\
        length,width,paddedwidth,pad,swell_factor)\
    firstprivate(row,col,pass,htemp,running_sum,hpadded)
    {
#pragma omp for schedule(dynamic) nowait
        for (row = 0; row<length;row++)
        {
            //Mirror the start of padded from the row.
            for (col = 0; col < pad; col++)
            {
                hpadded[col] = developer_concentration(row,pad-col);
            }
            //Fill the center of padded with a row.
            for (col = 0; col < width; col++)
            {
                hpadded[col+pad] = developer_concentration(row,col);
            }
            //Mirror the row onto the end of padded.
            for (col = 0; col < pad; col++)
            {
                hpadded[col+pad+width] = 
                    developer_concentration(row,width-2-col);
            }
            for (pass = 0; pass < order; pass++)
            {
                //Perform a box blur, but hold off on the divisions in the
                //averaging calculations
                
                //Start the running sum going at the beginning of the pad.
                running_sum = 0;
                for (col= pass*convrad; col < (pass*convrad)+convlength; col++)
                {
                    running_sum += hpadded[col];
                }

                //Start moving down the row.
                for (col = (pass+1)*convrad;
                     col < paddedwidth-(pass+1)*convrad;
                     col++)
                {
                    htemp[col] = running_sum;
                    running_sum = running_sum + hpadded[col+convrad+1];
                    running_sum = running_sum - hpadded[col-convrad];
                }
                
                //Copy what was in htemp to hpadded for the next iteration.
                //Swap just swaps pointers, so it is O(1)
                swap(htemp,hpadded);
            }
            //Now we're done with the convolution of one row, and we can copy
            //it back into developer_concentration. But we should also do the
            //divisions that we never did during our previous averaging. 
            for (col = 0; col < width; col++)
            {
                developer_concentration(row,col) =
                	hpadded[col+pad]*swell_factor;
            }
        }   
    }
}
