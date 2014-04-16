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
#include <algorithm>

float default_tonecurve( float input, bool enabled )
{
    if( !enabled )
        return input;

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
    double cx = p0x - double( input );
    double cy = p0y;

    //The bezier curves are defined parametrically, with respect to t.
    //We need to find with respect to x, so we need to find what t value
    //corresponds to the x.
    double t_value = ( -bx + sqrt( bx*bx - 4*ax*cx ) ) / ( 2*ax );

    double y_out = ( ay*t_value*t_value + by*t_value + cy );
    float output = float( y_out );
    
	return output;
}


float shadows_highlights ( float input,
                           float shadowsX,
                           float shadowsY,
                           float highlightsX,
                           float highlightsY )
{

    float x = input;
    float a = shadowsX;
    float b = shadowsY;
    float c = highlightsX;
    float d = highlightsY;

    float y0a = 0.00; // initial y
    float x0a = 0.00; // initial x
    float y1a = b;    // 1st influence y
    float x1a = a;    // 1st influence x
    float y2a = d;    // 2nd influence y
    float x2a = c;    // 2nd influence x
    float y3a = 1.00; // final y
    float x3a = 1.00; // final x

    float A =   x3a - 3*x2a + 3*x1a - x0a;
    float B = 3*x2a - 6*x1a + 3*x0a;
    float C = 3*x1a - 3*x0a;
    float D =   x0a;

    float E =   y3a - 3*y2a + 3*y1a - y0a;
    float F = 3*y2a - 6*y1a + 3*y0a;
    float G = 3*y1a - 3*y0a;
    float H =   y0a;

    // Solve for t given x (using Newton-Raphson), then solve for y given t.
    // Assume for the first guess that t = x.
    float currentt = x;
    int nRefinementIterations = 5;
    for ( int i=0; i < nRefinementIterations; i++ )
    {
        float currentx = xFromT( currentt, A, B, C, D );
        float currentslope = slopeFromT( currentt, A, B, C );
        currentt -= ( currentx - x ) * ( currentslope );
        currentt = min( max( currentt, float( 0 ) ), float( 1 ) );
    }

    float y = yFromT ( currentt, E, F, G, H );
    return y;
}

// Helper functions:
float slopeFromT( float t, float A, float B, float C )
{
    float dtdx = 1.0 / ( 3.0*A*t*t + 2.0*B*t + C);
    return dtdx;
}

float xFromT( float t, float A, float B, float C, float D )
{
    float x = A*( t*t*t ) + B*( t*t ) + C*t + D;
    return x;
}

float yFromT( float t, float E, float F, float G, float H )
{
    float y = E*( t*t*t ) + F*( t*t ) + G*t + H;
    return y;
}

//This code was derived from the RawTherapee project, which says it was taken
//from Adobe's reference implementation for tone curves.
//
//I couldn't find the original source, though...
//
//An explanation of the algorithm:
//The algorithm applies the designated tone curve on the highest and the lowest
// values.
//On the middle value, instead of using the tone curve, which would induce hue
// shifts, it simply maintains the relative spacing between the color components.
//It makes no difference for linear tone "curves".
void film_like_curve( matrix<unsigned short> &input,
                      matrix<unsigned short> &output,
                      LUT &lookup )
{
    int xsize = input.nc();
    int ysize = input.nr();
    output.set_size( ysize,xsize );

#pragma omp parallel shared( lookup, input, output, xsize, ysize )
    {
#pragma omp for schedule( dynamic ) nowait
    for ( int i = 0; i < ysize; i++ )
    {
        for ( int j = 0; j < xsize; j = j + 3 )
        {
            unsigned short r = input( i, j   );
            unsigned short g = input( i, j+1 );
            unsigned short b = input( i, j+2 );

            if ( r >= g )
            {
                if      ( g > b ) RGBTone ( r, g, b, lookup ); // Case1: r>= g>  b
                else if ( b > r ) RGBTone ( b, r, g, lookup ); // Case2: b>  r>= g
                else if ( b > g ) RGBTone ( r, b, g, lookup ); // Case3: r>= b>  g
                else							           // Case4: r>= g== b
                {
                    //RGBTone fails if the first and last arguments are the same.
                    //So in this case, since that might happen, don't call it.
                    r = lookup[ r ];
                    g = lookup[ g ];
                    b = g;
                }
            }
            else
            {
                if      ( r >= b ) RGBTone ( g, r, b, lookup ); // Case5: g>  r>= b
                else if ( b >  g ) RGBTone ( b, g, r, lookup ); // Case6: b>  g>  r
                else               RGBTone ( g, b, r, lookup ); // Case7: g>= b>  r
            }
            output( i, j   ) = r;
            output( i, j+1 ) = g;
            output( i, j+2 ) = b;
        }
    }
    }
}

//This is what does the actual computation of the middle value.
//It assumes that r and b are the extreme values, and that they are different.
void RGBTone( unsigned short& hi, unsigned short& mid, unsigned short& lo, LUT &lookup )
{
    unsigned short rOld=hi, gOld=mid, bOld=lo;

    hi = lookup[ rOld ];
    lo = lookup[ bOld ];
    float rf = hi;
    float bf = lo;
    float roldf = rOld;
    float goldf = gOld;
    float boldf = bOld;
    mid = bf + ( ( rf - bf ) * ( goldf - boldf ) / ( roldf - boldf ) );
}
