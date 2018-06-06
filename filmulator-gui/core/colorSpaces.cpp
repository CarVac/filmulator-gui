#include "filmSim.hpp"
#include "lut.hpp"

//Constants for conversion to and from L*a*b*
#define LAB_EPSILON (216.0/24389.0)
#define LAB_KAPPA (24389.0/27.0)

//D50 reference white point in XYZ for conversion to and from L*a*b*
//#define LAB_XR 0.9642 //This is supposedly what Adobe uses.
#define LAB_XR 0.96422 //This is the sum of the X row of sRGB_to_XYZ
#define LAB_YR 1.00000
//#define LAB_ZR 0.8249 //This is supposedly what Adobe uses.
#define LAB_ZR 0.82511 //This is the sum of the Z row of sRGB_to_XYZ

//Converts sRGB with D50 illuminant to XYZ with D50 illuminant.
void sRGB_to_XYZ(float  R, float  G, float  B,
                 float &X, float &Y, float &Z)
{
    X = 0.4360747*R + 0.3850649*G + 0.1430804*B;
    Y = 0.2225045*R + 0.7168786*G + 0.0606169*B;
    Z = 0.0139322*R + 0.0971045*G + 0.7141733*B;
}

//Converts XYZ with D50 illuminant to sRGB with D50 illuminant.
void XYZ_to_sRGB(float  X, float  Y, float  Z,
                 float &R, float &G, float &B)
{
    R =  3.1338561*X - 1.6168667*Y - 0.4906146*Z;
    G = -0.9787684*X + 1.9161415*Y + 0.0334540*Z;
    B =  0.0719453*X - 0.2289914*Y + 1.4052427*Z;
}

//Linearizes gamma-curved sRGB.
//Reference: http://www.brucelindbloom.com/index.html?Eqn_RGB_to_XYZ.html
//http://stackoverflow.com/questions/6475373/optimizations-for-pow-with-const-non-integer-exponent
float sRGB_inverse_gamma(float c)
{
    if (c < 0)
    {
        return 0;
    }
    else if (c <= 0.04045)
    {
        return c / 12.92;
    }
    else if (c < 1)
    {
        return pow((c+0.055)/1.055,2.4);
    }
    else
    {
        return 1;
    }
}

//Gamma-compresses linear into sRGB.
//http://stackoverflow.com/questions/6475373/optimizations-for-pow-with-const-non-integer-exponent
float sRGB_forward_gamma(float c)
{
    if (c < 0)
    {
        return 0;
    }
    else if (c <= 0.0031308)
    {
        return c * 12.92;
    }
    else if (c < 1)
    {
        return 1.055*pow(c,1/2.4) - 0.055;
    }
    else
    {
        return 1;
    }
}

//Linearize L* curved XYZ (coming from L*a*b*)
//Reference: http://www.brucelindbloom.com/index.html?Eqn_Lab_to_XYZ.html
float Lab_inverse_gamma(float c)
{
    if(c < 0)
    {
        return 0;
    }
    else if (c <= 0.08)
    {
        return c / LAB_KAPPA;
    }
    else if (c < 1)
    {
        return pow((c+0.16)/1.16,3);
    }
    else
    {
        return 1;
    }
}

//L* curve linear XYZ data in preparation to going to L*a*b*
//Reference: http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_Lab.html
float Lab_forward_gamma(float c)
{
    if(c < 0)
    {
        return 0;
    }
    else if (c <= LAB_EPSILON)
    {
        return (LAB_KAPPA*c+16)/116;
    }
    else if (c < 1)
    {
        return pow(c,1.0/3.0);
    }
    else
    {
        return 1;
    }
}

//Arithmetic operations from L* curved XYZ to L*a*b*
//Reference: http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_Lab.html
void XYZ_to_Lab(float fx, float fy, float fz,
                 float &L, float &a, float &b)
{
    L = max( 0.0f, min(1.0f, 116*fy - 16  ));
    a = max(-1.0f, min(1.0f, 500*(fx - fy)));
    b = max(-1.0f, min(1.0f, 200*(fy - fz)));
}

//Arithmetic operations from L*a*b* to L* curved XYZ
void Lab_to_XYZ(float   L, float   a, float   b,
                 float &fx, float &fy, float &fz)
{
    fy = (L + 16)/116;
    fx = a/500 + fy;
    fz = fy - b/200;
}


//Converts gamma-curved sRGB D50 to L*a*b*, all unsigned shorts from 0 to 65535.
//The L* is 0 = 0, 1 = 65535.
//a* and b* are -1 = 1, 0 = 32768, +1 = 65535.
//Reference: http://www.brucelindbloom.com/index.html?Eqn_RGB_to_XYZ.html
void sRGB_to_Lab_s(matrix<unsigned short> &in,
                   matrix<unsigned short> &Lab)
{
    int nRows = in.nr();
    int nCols = in.nc();

    Lab.set_size(nRows, nCols);

#pragma omp parallel shared(in, Lab) firstprivate(nRows, nCols)
    {
#pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < nRows; i++)
        {
            for (int j = 0; j < nCols; j += 3)
            {
                //First, linearize the sRGB.
                float r = sRGB_inverse_gamma(float(in(i, j  ))/65535.0);
                float g = sRGB_inverse_gamma(float(in(i, j+1))/65535.0);
                float b = sRGB_inverse_gamma(float(in(i, j+2))/65535.0);

                //Next, convert to XYZ.
                float x, y, z;
                sRGB_to_XYZ(r, g, b, x, y, z);

                //Next, convert to L*a*b*
                //First we must scale by the white point of D50.
                float xr = x/LAB_XR;
                float yr = y/LAB_YR;
                float zr = z/LAB_ZR;
                //Using that, we apply the L* curve.
                float fx = Lab_forward_gamma(xr);
                float fy = Lab_forward_gamma(yr);
                float fz = Lab_forward_gamma(zr);
                //Next is the not-quite-matrix operations.
                float L, a;// b is already declared;
                XYZ_to_Lab(fx, fy, fz, L, a, b);
                Lab(i, j  ) = (unsigned short)(65535*L);
                Lab(i, j+1) = (unsigned short)(32767*a + 32768);
                Lab(i, j+2) = (unsigned short)(32767*b + 32768);
            }
        }
    }
}

//Converts gamma-curved sRGB D50 to linear, short int to float.
void sRGB_linearize(matrix<unsigned short> &in,
                    matrix<float> &out)
{
    int nRows = in.nr();
    int nCols = in.nc();

    out.set_size(nRows, nCols);

#pragma omp parallel shared(in, out) firstprivate(nRows, nCols)
    {
#pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < nRows; i++)
        {
            for (int j = 0; j < nCols; j += 3)
            {
                out(i, j  ) = sRGB_inverse_gamma(float(in(i, j  ))/65535.0);
                out(i, j+1) = sRGB_inverse_gamma(float(in(i, j+1))/65535.0);
                out(i, j+2) = sRGB_inverse_gamma(float(in(i, j+2))/65535.0);
            }
        }
    }
}

//Converts linear float sRGB D50 to gamma-curved, float to short int.
//Reference: http://www.brucelindbloom.com/index.html?Eqn_RGB_to_XYZ.html
void sRGB_gammacurve(matrix<float> &in,
                     matrix<unsigned short> &out)
{
    int nRows = in.nr();
    int nCols = in.nc();

    out.set_size(nRows,nCols);

#pragma omp parallel shared(in, out) firstprivate(nRows, nCols)
    {
#pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < nRows; i++)
        {
            for (int j = 0; j < nCols; j += 3)
            {
                //First, linearize the sRGB.
                out(i, j  ) = (unsigned short)(65535*sRGB_forward_gamma(in(i, j  )));
                out(i, j+1) = (unsigned short)(65535*sRGB_forward_gamma(in(i, j+1)));
                out(i, j+2) = (unsigned short)(65535*sRGB_forward_gamma(in(i, j+2)));
            }
        }
    }
}
