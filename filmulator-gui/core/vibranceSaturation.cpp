#include "filmSim.hpp"
#include <algorithm>
#include <iostream>

using std::cout;
using std::endl;

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

float shift23=(1<<23);
float OOshift23=1.0/(1<<23);

float myLog2(float i)
{
    float LogBodge=0.346607f;
    float x;
    float y;
    x=*(int *)&i;
    x*= OOshift23; //1/pow(2,23);
    x=x-127;

    y=x-floorf(x);
    y=(y-y*y)*LogBodge;
    return x+y;
}
float myPow2(float i)
{
    float PowBodge=0.33971f;
    float x;
    float y=i-floorf(i);
    y=(y-y*y)*PowBodge;

    x=i+127-y;
    x*= shift23; //pow(2,23);
    *(int*)&x=(int)x;
    return x;
}

float myPow(float a, float b)
{
    float result;
    if(a < std::numeric_limits<float>::min())
        result = a; //(avoid bugs with tiny a)
    else
        result = myPow2(b*myLog2(a));
    return result;
}

void vibrance_saturation(matrix<unsigned short> &input,
                         matrix<unsigned short> &output,
                         float vibrance, float saturation)
{
    int nrows = input.nr();
    int ncols = input.nc();
    double gamma = pow(2,-vibrance);
    double sat = pow(2,saturation);
    output.set_size(nrows,ncols);
    if ( abs( vibrance ) < 0.00001 && abs( saturation ) < 0.00001 ) //no adjustment
    {
        output = input;
        return;
    }
    //else, apply the adjustment.
#pragma omp parallel shared(output, input) firstprivate(nrows,ncols,saturation,gamma)
    {
#pragma omp for schedule(dynamic) nowait
    for(int i = 0; i < nrows; i++)
        for(int j = 0; j < ncols; j += 3)
        {
            float r = float(input(i,j  ))/65535.0f;
            float g = float(input(i,j+1))/65535.0f;
            float b = float(input(i,j+2))/65535.0f;
            float h,s,v;
            RGBtoHSV(r,g,b,h,s,v);
            s = max(min( sat*myPow(s,gamma), 1.0),0.0);
            HSVtoRGB(h,s,v,r,g,b);
            output(i,j  ) = r*65535;
            output(i,j+1) = g*65535;
            output(i,j+2) = b*65535;
        }
    }
}

void monochrome_convert(matrix<unsigned short> &input,
                        matrix<unsigned short> &output,
                        float rmult, float gmult, float bmult)
{
    int nrows = input.nr();
    int ncols = input.nc();
    output.set_size(nrows, ncols);

    for(int i = 0; i < nrows; i++)
    {
        for (int j = 0; j < ncols; j += 3)
        {
            int gray = input(i,j)*rmult + input(i,j+1)*gmult + input(i,j+2)*bmult;
            gray = max(0,min(gray, 65535));
            output(i, j  ) = gray;
            output(i, j+1) = gray;
            output(i, j+2) = gray;
        }
    }
}
