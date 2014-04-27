#include "filmsim.hpp"
#include <utility>

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

void whiteBalanceMults( double temperature, double tint, std::string inputFilename,
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
    double BASE_TEMP =  6005.973;
    double BASE_TINT = 0.9712947;

    //Now we compute the coordinates.
    temp_tint_to_xy( temperature, 0, xyzXIllum, xyzYIllum );
    xyzZIllum = 1 - xyzXIllum - xyzYIllum;
    temp_tint_to_xy( BASE_TEMP, 0, xyzXBase, xyzYBase );
    xyzZBase = 1 - xyzXBase - xyzYBase;

    //Next, we convert them to RGB.
    double rIllum, gIllum, bIllum;
    double rBase, gBase, bBase;
    xyz2rgb( xyzXIllum, xyzYIllum, xyzZIllum,
             rIllum, gIllum, bIllum );
    xyz2rgb( xyzXBase, xyzYBase, xyzZBase,
             rBase, gBase, bBase );

    //Calculate the multipliers to convert from one illuminant to the base.
    gIllum /= tint;
    gBase /= BASE_TINT;
    rMult = rBase / rIllum;
    gMult = gBase / gIllum;
    bMult = bBase / bIllum;

    //cout << "white_balance: non-offset multipliers" << endl;
    //cout << rMult << endl << gMult << endl << bMult << endl;

    //Check that they don't go negative.
    rMult = max( rMult, 0.0 );
    gMult = max( gMult, 0.0 );
    bMult = max( bMult, 0.0 );

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


    double rBaseMult, gBaseMult, bBaseMult;
    double rCamMult, gCamMult, bCamMult;
    //Grab the existing white balance data from the raw file.
    LibRaw imageProcessor;
#define COLOR imageProcessor.imgdata.color

    const char *cstr = inputFilename.c_str();
    if ( 0 == imageProcessor.open_file( cstr ) )
    {
    //Set the white balance arguments based on what libraw did.

    //First is the fixed (per-camera) daylight multipliers.
    rBaseMult = COLOR.pre_mul[ 0 ];
    gBaseMult = COLOR.pre_mul[ 1 ];
    bBaseMult = COLOR.pre_mul[ 2 ];
    //Next is the white balance coefficients as shot.
    rCamMult = COLOR.cam_mul[ 0 ];
    gCamMult = COLOR.cam_mul[ 1 ];
    bCamMult = COLOR.cam_mul[ 2 ];
    }
    else //it couldn't read the file, or it wasn't raw. Either way, fallback to 1
    {
        rBaseMult = 1;
        gBaseMult = 1;
        bBaseMult = 1;
        rCamMult = 1;
        gCamMult = 1;
        bCamMult = 1;
    }


    rMult *= rBaseMult / rCamMult;
    gMult *= gBaseMult / gCamMult;
    bMult *= bBaseMult / bCamMult;
    double multMin = min( min( rMult, gMult ), bMult );
    rMult /= multMin;
    gMult /= multMin;
    bMult /= multMin;
}

//Computes the Eulerian distance from the WB coefficients to (1,1,1). Also adds the temp to it.
double wbDistance( std::string inputFilename, array<double,2> tempTint )
{
    double rMult, gMult, bMult;
    whiteBalanceMults( tempTint[ 0 ], tempTint[ 1 ], inputFilename,
                       rMult, gMult, bMult );
    rMult -= 1;
    gMult -= 1;
    bMult -= 1;
    return sqrt( rMult*rMult + gMult*gMult + bMult*bMult ) + sqrt( tempTint[ 1 ]*tempTint[ 1 ] );
}

void optimizeWBMults( std::string file,
                      double &temperature, double &tint )
{
    //This is nelder-mead in 2d, so we have 3 points.
    array<double,2> lowCoord, midCoord, hiCoord;
    //Some temporary coordinates for use in optimizing.
    array<double,2> meanCoord, reflCoord, expCoord, contCoord;
    //Temperature
    lowCoord[ 0 ] = 5000.0;
    midCoord[ 0 ] = 5200.0;
    hiCoord[ 0 ]  = 5400.0;
    //Tint
    lowCoord[ 1 ] = 1.0;
    midCoord[ 1 ] = 1.0001;
    hiCoord[ 1 ]  = 1.0;

    double low, mid, hi, oldLow, delta;
    low = wbDistance( file, lowCoord );
    mid = wbDistance( file, midCoord );
    hi  = wbDistance( file, hiCoord  );
    double refl, exp, cont;

#define TOLERANCE 0.000001
#define ITER_LIMIT 10000
#define REPEAT_LIMIT 5

    int iterations = 0;
    delta = 1;
    int repeats = 0;

    while ( repeats < REPEAT_LIMIT )
    {
        iterations++;
        if ( iterations > ITER_LIMIT )
        {
            temperature = 5200.0;
            tint = 1;
            return;
        }

        //Remember the low value
        oldLow = low;
        //Sort the coordinates.
        if ( mid > hi )
        {
            midCoord.swap( hiCoord );
            swap( mid, hi );
        }
        if ( low > mid )
        {
            lowCoord.swap( midCoord );
            swap( low, mid );
        }
        if ( mid > hi )
        {
            midCoord.swap( hiCoord );
            swap( mid, hi );
        }//End sort
        if ( oldLow - low < TOLERANCE ) //if it hasn't improved enough
        {
            repeats++;
        }
        else //if it got over a hump
        {
            repeats = 0;
        }

        //Centroid of all but the worst point.
        meanCoord[ 0 ] = 0.5 * ( lowCoord[ 0 ]  + midCoord[ 0 ] );
        meanCoord[ 1 ] = 0.5 * ( lowCoord[ 1 ]  + midCoord[ 1 ] );

        //Reflect the worst point about the centroid.
        reflCoord[ 0 ] = meanCoord[ 0 ] + 1 * ( meanCoord[ 0 ] - hiCoord[ 0 ] );
        reflCoord[ 1 ] = meanCoord[ 1 ] + 1 * ( meanCoord[ 1 ] - hiCoord[ 1 ] );
        refl = wbDistance( file, reflCoord );
        if ( refl < mid ) //Better than the second-worst point
        {
            if ( refl > low ) //but not better than the old best point
            { //Swap this with the worst point
                hiCoord.swap( reflCoord );
                swap( hi, refl );
            } //and go back to the beginning.
            else //It's the best point so far
            { //Try a point expanded farther away in the same direction.
                expCoord[ 0 ] = meanCoord[ 0 ] + 2 * (meanCoord[ 0 ] - hiCoord[ 0 ] );
                expCoord[ 1 ] = meanCoord[ 1 ] + 2 * (meanCoord[ 1 ] - hiCoord[ 1 ] );
                exp = wbDistance( file, expCoord );
                if ( exp < refl ) //It is the best so far
                { //Swap this with the worst point
                    hiCoord.swap( expCoord );
                    swap( hi, exp );
                } //and go back to the beginning.
                else //The reflected point was better
                { //Swap the reflected point with the best point
                    hiCoord.swap( reflCoord );
                    swap( hi,refl );
                } //and go back.
            }
        }
        else //The reflected point was not better than the second-worst point
        { //Compute a point contracted from the worst towards the centroid.
            contCoord[ 0 ] = meanCoord[ 0 ] - 0.5 * (meanCoord[ 0 ] - hiCoord[ 0 ] );
            contCoord[ 1 ] = meanCoord[ 1 ] - 0.5 * (meanCoord[ 1 ] - hiCoord[ 1 ] );
            cont = wbDistance( file, contCoord );
            if ( cont < hi ) //Better than the worst point
            { //Replace the worst point with this.
                hiCoord.swap( contCoord );
                swap( hi, cont );
            } //and go back.
            else //Everything we tried was terrible
            { //Contract everything towards the best point, not the centroid.
                hiCoord[ 0 ] = lowCoord[ 0 ] + 0.5 * (lowCoord[ 0 ] - hiCoord[ 0 ] );
                hiCoord[ 1 ] = lowCoord[ 1 ] + 0.5 * (lowCoord[ 1 ] - hiCoord[ 1 ] );
                hi = wbDistance( file, hiCoord );
                midCoord[ 0 ] = lowCoord[ 0 ] + 0.5 * (lowCoord[ 0 ] - midCoord[ 0 ] );
                midCoord[ 1 ] = lowCoord[ 1 ] + 0.5 * (lowCoord[ 1 ] - midCoord[ 1 ] );
                mid = wbDistance( file, midCoord );
            }
        }
    }
    temperature = lowCoord[ 0 ];
    tint = lowCoord[ 1 ];
}

void whiteBalance( matrix<float> &input, matrix<float> &output,
                   double temperature, double tint,
                   std::string inputFilename )
{
    double rMult, gMult, bMult;
    whiteBalanceMults( temperature, tint, inputFilename,
                       rMult, gMult, bMult );

    cout << "white_balance: settings" << endl;
    cout << temperature << endl << tint << endl;
    cout << "white_balance: multipliers" << endl;
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
