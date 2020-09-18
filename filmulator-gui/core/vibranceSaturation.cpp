#include "filmSim.hpp"
#include <algorithm>
#include <iostream>

using std::cout;
using std::endl;

// r,g,b values are from 0 to 65535
// h = [0,6], s = [0,1], v = [0,1]
//		if s == 0, then h = -1 (undefined)

inline void RGBtoHSV65535( float r, float g, float b, float &h, float &s, float &v )
{

    const float minimum = min(min( r, g), b );
    const float maximum = max(max( r, g), b );
    v = maximum / 65535.f;				// v

    const float delta = maximum - minimum;

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

    if( h < 0 )
        h += 6;

}

inline void HSVtoRGB65535( float h, float s, float v, float &r, float &g, float &b)
{
    v *= 65535.f;
    if ( s == 0 ) {
        // achromatic (grey)
        r = g = b = v;
        return;
    }

    const int i = h; // floor() is very slow, and h is always >= 0

    const float f = h - i;			// factorial part of h
    const float vs = v * s;
    const float p = v - vs;
    const float q = v - f * vs;
    const float t = p + v - q;

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

constexpr float shift23=(1<<23);
constexpr float OOshift23=1.0/(1<<23);

inline float myLog2(float i)
{
    constexpr float LogBodge=0.346607f;
    float x;
    float y;
    x=*(int *)&i;
    x*= OOshift23; //1/pow(2,23);
    x=x-127;

    y=x-floorf(x);
    y=(y-y*y)*LogBodge;
    return x+y;
}
inline float myPow2(float i)
{
    constexpr float PowBodge=0.33971f;
    float x;
    float y=i-floorf(i);
    y=(y-y*y)*PowBodge;

    x=i+127-y;
    x*= shift23; //pow(2,23);
    *(int*)&x=(int)x;
    return x;
}

inline float myPow(float a, float b)
{
    //(avoid bugs with tiny a)
    return a < std::numeric_limits<float>::min() ? a : myPow2(b*myLog2(a));
}

void vibrance_saturation(const matrix<unsigned short> &input,
                         matrix<unsigned short> &output,
                         float vibrance, float saturation)
{

    if ( abs( vibrance ) < 0.00001 && abs( saturation ) < 0.00001 ) //no adjustment
    {
        output = input;
        return;
    }
    //else, apply the adjustment.
    const int nrows = input.nr();
    const int ncols = input.nc();
    const float gamma = pow(2,-vibrance);
    const float sat = pow(2,saturation);
    output.set_size(nrows,ncols);

    #pragma omp parallel for schedule(dynamic)
    for(int i = 0; i < nrows; i++) {
        for(int j = 0; j < ncols; j += 3)
        {
            float r = input(i,j  );
            float g = input(i,j+1);
            float b = input(i,j+2);
            float h,s,v;
            RGBtoHSV65535(r,g,b,h,s,v);
            s = max(min( sat*myPow(s,gamma), 1.f),0.f);
            HSVtoRGB65535(h,s,v,r,g,b);
            output(i,j  ) = r;
            output(i,j+1) = g;
            output(i,j+2) = b;
        }
    }
}

void monochrome_convert(const matrix<unsigned short> &input,
                        matrix<unsigned short> &output,
                        float rmult, float gmult, float bmult)
{
    const int nrows = input.nr();
    const int ncols = input.nc();
    output.set_size(nrows, ncols);

    #pragma omp parallel for schedule(dynamic)
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
