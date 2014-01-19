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

int flatcurve(int input)
{
	return input;
}

int tonecurve(int input)
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
    double cx = p0x - double(input)/65535;
    double cy = p0y;

    //The bezier curves are defined parametrically, with respect to t.
    //We need to find with respect to x, so we need to find what t value
    //corresponds to the x.
    double t_value = (-bx + sqrt(bx*bx - 4*ax*cx))/(2*ax);

    double y_out = (ay*t_value*t_value + by*t_value + cy)*65535;
	int output = (int)(y_out); // TODO: Something else should go here
    
    //Uncomment for some debug points on the control curve.
//    if (input%1000 == 1)
//    {
//        cout << "input/65535 = " << double(input)/65535 << endl;
//        cout << "t_value = " << t_value << endl;
//        cout << "output = " << y_out/65535 << endl;
//    }
	return output;
}

//This code was derived from the RawTherapee project, which says it was taken
//from Adobe's reference implementation for tone curves.
//
//I couldn't find the original source, though...
void apply_tone_curve(LUT &lookup,matrix<float> &output_density,
					  matrix<int> &output_r,matrix<int> &output_g,
					  matrix<int> &output_b)
{
	int xsize = output_r.nc();
	int ysize = output_r.nr();

    //Here I set up indices for reading the interlaced colors.
    int ir, ig, ib;

#pragma omp parallel shared(lookup, output_density, output_r, output_g,\
                            output_b, xsize, ysize) private(ir, ig, ib)
    {
#pragma omp for schedule(dynamic) nowait
	for (int i = 0; i < xsize; i++)
	{
        //Setting up the indices for the colors.
        ir = i*3;
        ig = ir + 1;
        ib = ir + 2;
		for(int j = 0; j < ysize; j ++)
		{
			float r = output_density(j,ir)+0.5f; //rounding
			float g = output_density(j,ig)+0.5f;
			float b = output_density(j,ib)+0.5f;

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
			output_r(j,i) = r;
			output_g(j,i) = g;
			output_b(j,i) = b;
		}
    }
    }
}

void RGBTone (float& r, float& g, float& b, LUT &lookup)
{
    float rold=r,gold=g,bold=b;

    r = lookup[rold];
    b = lookup[bold];
    g = b + ((r - b) * (gold - bold) / (rold - bold));
}

