#include "filmsim.hpp"
#include <algorithm>

void temp_tint_to_xy(double const temp, double const tint,
                     double &tempTintX, double &tempTintY)
{
    double tempX;

    if(temp < 4000)
    {
        tempX = -0.2661239 * pow(10,9) * pow(temp,-3) +
                -0.2343589 * pow(10,6) * pow(temp,-2) +
                 0.8776956 * pow(10,3) * pow(temp,-1) +
                 0.179910;
    }
    else
    {
        tempX = -3.0258469 * pow(10,9) * pow(temp,-3) +
                 2.1070379 * pow(10,6) * pow(temp,-2) +
                 0.2226347 * pow(10,3) * pow(temp,-1) +
                 0.24039;
    }

    double tempY;
    double dYdX;;

    if(temp < 2222)
    {
        tempY = -1.1063814  * pow(tempX,3) +
                -1.34811020 * pow(tempX,2) +
                 2.18555832 * tempX +
                -0.20219683;

        dYdX = 3*-1.1063814  * pow(tempX,2) +
               2*-1.34811020 * tempX +
                  2.18555832;
    }
    else if(temp < 4000)
    {
        tempY = -0.9549476  * pow(tempX,3) +
                -1.37418593 * pow(tempX,2) +
                 2.09137015 * tempX +
                -0.16748867;

        dYdX = 3*-0.9549476  * pow(tempX,2) +
               2*-1.37418593 * tempX +
                  2.09137015;
    }
    else
    {
        tempY =  3.0817580  * pow(tempX,3) +
                -5.8733867 * pow(tempX,2) +
                 3.75112997 * tempX +
                -0.37001483;

        dYdX = 3* 3.0817580  * pow(tempX,2) +
               2*-5.8733867 * tempX +
                 -0.37001483;
    }

    if(fabs(dYdX) > 0.01)
    {
        double normal = -1.0/dYdX;

        //solution of x^2 + y^2 = tint^2; y=normal*x
        double tintX = sqrt( pow(tint,2)/(1.0+ pow(normal,2) ) );
        if (tint < 0)
            tintX = -tintX;
        double tintY = normal*tintX;

        tempTintX = tempX + tintX;
        tempTintY = tempY + tintY;
    }
    else
    {
        tempTintX = tempX;
        if(dYdX > 0)
            tempTintY = tempY - tint;
        else
            tempTintY = tempY + tint;
    }

    return;
}

void rgb_to_xyz(double  r, double  g, double  b,
                double &x, double &y, double &z)
{
    x = 0.4124*r + 0.3576*g + 0.1805*b;
    y = 0.2126*r + 0.7152*g + 0.0722*b;
    z = 0.0193*r + 0.1192*g + 0.9502*b;
}

void xyz2rgb( double  x, double  y, double  z,
              double &r, double &g, double &b)
{
    r =  3.2406*x - 1.5372*y - 0.4989*z;
    g = -0.9689*x + 1.8758*y + 0.0415*z;
    b =  0.0557*x - 0.2041*y + 1.0573*z;
}

void white_balance ( matrix<float> &input, matrix<float> &output,
                     double temp, double tone )
{
    double tempTintX, tempTintY;
    temp_tint_to_xy(temp,tone,tempTintX,tempTintY);
    double xShift = (1.0/3.0)/tempTintX;
    double yShift = (1.0/3.0)/tempTintY;

    int nrows = input.nr();
    int ncols = input.nc();

    output.set_size(nrows,ncols);
#pragma omp parallel shared(output, input) firstprivate(nrows,ncols,xShift,yShift)
        {
#pragma omp for schedule(dynamic) nowait
    for(int i = 0; i < nrows; i++)
        for(int j = 0; j < ncols; j = j+3)
        {
            double inputX, inputY, inputZ;
            rgb_to_xyz(input(i,j),input(i,j+1),input(i,j+2),
                       inputX    ,inputY      ,inputZ);
            double magnitude = inputX + inputY + inputZ;
            double newX = inputX*xShift;
            double newY = inputY*yShift;
            double newZ = magnitude - newX - newY;

            double newR, newG, newB;
            xyz2rgb(newX, newY, newZ, newR, newG, newB);

            output(i,j  ) = max(newR,0.0);
            output(i,j+1) = max(newG,0.0);
            output(i,j+2) = max(newB,0.0);
        }
    }
}

void whiteBalanceMults( double temperature, double tint,
                         double &rMult, double &gMult, double &bMult )
{
    //Value of the desired illuminant in the xyz space.
    double xyzXIllum, xyzYIllum, xyzZIllum;

    //Value of the base illuminant in the xyz space.
    double xyzXBase, xyzYBase, xyzZBase;

    //In order to get physically relevant temperatures, we trust dcraw
    // and by proxy libraw to give consistent WB in the
    // "daylight multipliers" (dcraw) and "pre_mul" (libraw)
    // fields.
    //
    //The following values are our baseline estimate of what this temperature
    // and tint is.
    double BASE_TEMP =  5830.523;
    double BASE_TINT = -0.00184056;

    //Now we compute the coordinates.
    temp_tint_to_xy( temperature, tint, xyzXIllum, xyzYIllum );
    xyzZIllum = 1 - xyzXIllum - xyzYIllum;
    temp_tint_to_xy( BASE_TEMP, BASE_TINT, xyzXBase, xyzYBase );
    xyzZBase = 1 - xyzXBase - xyzYBase;

    //Calculate the multiplier to convert from one illuminant to the other.
    //
    //The first constants are rgb2xyz(1,1,1): when the illuminants match for
    // the baseline value and the user's set temperature, we want the
    // RGB multipliers to all equal 1.
    //
    //Then, we divide the baseline by
    double xMult = 0.9505 * xyzXBase / xyzXIllum;
    double yMult = 1.0000 * xyzYBase / xyzYIllum;
    double zMult = 1.0887 * xyzZBase / xyzZIllum;

    //Convert the xyz value of the illuminant compensation to rgb.
    xyz2rgb( xMult, yMult, zMult,
             rMult, gMult, bMult );

    //Check that they don't go negative.
    rMult = max( rMult, 0.0 );
    gMult = max( gMult, 0.0 );
    bMult = max( bMult, 0.0 );
}

void whiteBalance( matrix<float> &input, matrix<float> &output,
                   double temperature, double tint,
                   std::string inputFilename )
                   /*double rBaseMult, double gBaseMult, double bBaseMult,
                   double rCamMult, double gCamMult, double bCamMult )*/
{

    //The white balance flow from LibRaw goes like this:
    // raw ---> srgb primaries --------------> [variable] camera WB.
    //The camera WB is variable, so we can't base anything off those
    // multipliers and have the temperature values be still physical.
    //Fortunately, LibRaw exposes another value for us, the "pre_mul" field,
    // which is fixed for each camera.
    //Our new white balance flow is:
    // raw ---> sRGB primaries --------------> camera WB
    //          sRGB primaries <------------------
    //                 --------> pre_mul ------> user set WB
    //So, we apply the reciprocal of the camera multipliers,
    // and simply multiply by the base and user multipliers.
    //The reason we don't ask libRaw for the raw color or sRGB primaries
    // is because demosaicing performs much better when the white balance
    // is approximately correct.
    // So we trust the camera's value to be close enough.


    //Grab the white balance data from the raw file.
    LibRaw imageProcessor;
#define COLOR imageProcessor.imgdata.color

    const char *cstr = inputFilename.c_str();
    if ( 0 != imageProcessor.open_file( cstr ) )
    {
        cerr << "Could not read input file!" << endl;
        return;
    }

    //Set the white balance arguments.

    //First is the fixed (per-camera) daylight multipliers.
    double rBaseMult = COLOR.pre_mul[ 0 ];
    double gBaseMult = COLOR.pre_mul[ 1 ];
    double bBaseMult = COLOR.pre_mul[ 2 ];
    //Next is the white balance coefficients as shot.
    double rCamMult = COLOR.cam_mul[ 0 ];
    double gCamMult = COLOR.cam_mul[ 1 ];
    double bCamMult = COLOR.cam_mul[ 2 ];


    //Computing user multipliers
    double rMult, bMult, gMult;
    whiteBalanceMults( temperature, tint,
                       rMult,
                       gMult,
                       bMult );
    cout << "white_balance user multipliers:" << endl;
    cout << rMult << endl << gMult << endl << bMult << endl;
    rMult *= rBaseMult / rCamMult;
    gMult *= gBaseMult / gCamMult;
    bMult *= bBaseMult / bCamMult;
    cout << "white_balance end multipliers:" << endl;
    double multMin = min( min( rMult, gMult ), bMult );
    rMult /= multMin;
    gMult /= multMin;
    bMult /= multMin;
    cout << rMult << endl << gMult << endl << bMult << endl;

    int nRows = input.nr();
    int nCols = input.nc();

    output.set_size( nRows, nCols );

#pragma omp parallel shared( output, input ) firstprivate( nRows, nCols, rMult, gMult, bMult )
    {
#pragma omp for schedule( dynamic ) nowait
        for ( int i = 0; i < nRows; i++ )
        {
            for ( int j = 0; j < nCols; j += 3 )
            {
                output( i, j   ) = rMult*input( i, j   );
                output( i, j+1 ) = gMult*input( i, j+1 );
                output( i, j+2 ) = bMult*input( i, j+2 );
            }
        }
    }
}
