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
#include <cmath>

float default_tonecurve(float input)
{
    //These are the coordinates for a quadratic bezier curve's
    //control points. They should be in ascending order.
    double p0x = 0;
    double p0y = 0;
    double p1x = 0.2;
    double p1y = 1;
    double p2x = 1;
    double p2y = 1;

    //Math for a quadratic bezier curve
    //a, b, c are as in a standard quadratic formula
    //x or y are the two axes of computation.
    double ax = p0x - 2*p1x + p2x;
    double ay = p0y - 2*p1y + p2y;
    double bx = -2*p0x + 2*p1x;
    double by = -2*p0y + 2*p1y;
    double cx = p0x - double(input);
    double cy = p0y;

    //The bezier curves are defined parametrically, with respect to t.
    //We need to find with respect to x, so we need to find what t value
    //corresponds to the x.
    double t_value = (-bx + sqrt(bx*bx - 4*ax*cx))/(2*ax);

    double y_out = (ay*t_value*t_value + by*t_value + cy);
    float output = (float)(y_out);
    
	return output;
}

//This code was derived from the RawTherapee project, which says it was taken
//from Adobe's reference implementation for tone curves.
//
//I couldn't find the original source, though...
void film_like_curve(matrix<unsigned short> &input,
                     matrix<unsigned short> &output, LUT &lookup)
{
    int xsize = input.nc();
    int ysize = input.nr();
    output.set_size(ysize,xsize);

#pragma omp parallel shared(lookup, input, output,xsize, ysize)
    {
#pragma omp for schedule(dynamic) nowait
    for (int i = 0; i < ysize; i++)
    {
        for(int j = 0; j < xsize; j = j + 3 )
        {
            unsigned short r = input(i,j);
            unsigned short g = input(i,j+1);
            unsigned short b = input(i,j+2);

            if (r >= g)
            {
                if      (g > b) RGBTone (r, g, b, lookup); // Case1: r>= g>  b
                else if (b > r) RGBTone (b, r, g, lookup); // Case2: b>  r>= g
                else if (b > g) RGBTone (r, b, g, lookup); // Case3: r>= b>  g
                else							           // Case4: r>= g== b
                {
                    r = lookup[r];
                    g = lookup[g];
                    b = g;
                }
            }
            else
            {
                if      (r >= b) RGBTone (g, r, b, lookup); // Case5: g>  r>= b
                else if (b >  g) RGBTone (b, g, r, lookup); // Case6: b>  g>  r
                else             RGBTone (g, b, r, lookup); // Case7: g>= b>  r
            }
            output(i,j) = r;
            output(i,j+1) = g;
            output(i,j+2) = b;
        }
    }
    }
}

void RGBTone (unsigned short& r, unsigned short& g, unsigned short& b, LUT &lookup)
{
    unsigned short rold=r,gold=g,bold=b;

    r = lookup[rold];
    b = lookup[bold];
    float rf = r;
    float bf = b;
    float roldf = rold;
    float goldf = gold;
    float boldf = bold;
    g = bf + ((rf - bf) * (goldf - boldf) / (roldf - boldf));
}
