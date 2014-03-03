#include "filmsim.hpp"
#include <algorithm>

// r,g,b values are from 0 to 1
// h = [0,360], s = [0,1], v = [0,1]
//		if s == 0, then h = -1 (undefined)

void RGBtoHSV( float r, float g, float b, float &h, float &s, float &v )
{
    float minimum, maximum, delta;

    minimum = min(min( r, g), b );
    maximum = max(max( r, g), b );
    v = maximum;				// v

    delta = maximum - minimum;

    if( maximum != 0 )
        s = delta / maximum;		// s
    else {
        // r = g = b = 0		// s = 0, v is undefined
        s = 0;
        h = -1;
        return;
    }

    if( r == maximum )
        h = ( g - b ) / delta;		// between yellow & magenta
    else if( g == maximum )
        h = 2 + ( b - r ) / delta;	// between cyan & yellow
    else
        h = 4 + ( r - g ) / delta;	// between magenta & cyan

    h *= 60;				// degrees
    if( h < 0 )
        h += 360;

}

void HSVtoRGB( float h, float s, float v, float &r, float &g, float &b)
{
    int i;
    float f, p, q, t;

    if( s == 0 ) {
        // achromatic (grey)
        r = g = b = v;
        return;
    }

    h /= 60;			// sector 0 to 5
    i = floor( h );
    f = h - i;			// factorial part of h
    p = v * ( 1 - s );
    q = v * ( 1 - s * f );
    t = v * ( 1 - s * ( 1 - f ) );

    switch( i ) {
        case 0:
            r = v;
            g = t;
            b = p;
            break;
        case 1:
            r = q;
            g = v;
            b = p;
            break;
        case 2:
            r = p;
            g = v;
            b = t;
            break;
        case 3:
            r = p;
            g = q;
            b = v;
            break;
        case 4:
            r = t;
            g = p;
            b = v;
            break;
        default:		// case 5:
            r = v;
            g = p;
            b = q;
            break;
    }

}

void vibrance_saturation(matrix<unsigned short> &input,
                         matrix<unsigned short> &output,
                         double vibrance, double saturation)
{
    double yIntercept = vibrance + saturation;
    double slope = 1-vibrance;
    int nrows = input.nr();
    int ncols = input.nc();
    output.set_size(nrows,ncols);
    for(int i = 0; i < nrows; i++)
        for(int j = 0; j < ncols; j += 3)
        {
            float r = float(input(i,j  ))/65535.0;
            float g = float(input(i,j+1))/65535.0;
            float b = float(input(i,j+2))/65535.0;
            float h,s,v;
            RGBtoHSV(r,g,b,h,s,v);
            s = max(min( yIntercept + slope*s, 1.0),0.0);
            HSVtoRGB(h,s,v,r,g,b);
            output(i,j  ) = r*65535;
            output(i,j+1) = g*65535;
            output(i,j+2) = b*65535;
        }
}
