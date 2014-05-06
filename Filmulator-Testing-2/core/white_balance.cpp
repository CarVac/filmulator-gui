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

void matrixVectorMult( double rr, double gr, double br,
                       double &x, double &y, double &z,
                       double mat[3][3] )
{
    x = mat[0][0]*rr + mat[0][1]*gr + mat[0][2]*br;
    y = mat[1][0]*rr + mat[1][1]*gr + mat[1][2]*br;
    z = mat[2][0]*rr + mat[2][1]*gr + mat[2][2]*br;
}

void inverse( double in[3][3], double (&out)[3][3] )
{
    double det = in[0][0] * ( in[1][1]*in[2][2] - in[2][1]*in[1][2] ) -
                 in[0][1] * ( in[1][0]*in[2][2] - in[1][2]*in[2][0] ) +
                 in[0][2] * ( in[1][0]*in[2][1] - in[1][1]*in[2][0] );
    double invdet = 1 / det;

    out[0][0] = ( in[1][1]*in[2][2] - in[2][1]*in[1][2] ) * invdet;
    out[0][1] = ( in[0][2]*in[2][1] - in[0][1]*in[2][2] ) * invdet;
    out[0][2] = ( in[0][1]*in[1][2] - in[0][2]*in[1][1] ) * invdet;
    out[1][0] = ( in[1][2]*in[2][0] - in[1][0]*in[2][2] ) * invdet;
    out[1][1] = ( in[0][0]*in[2][2] - in[0][2]*in[2][0] ) * invdet;
    out[1][2] = ( in[1][0]*in[0][2] - in[0][0]*in[1][2] ) * invdet;
    out[2][0] = ( in[1][0]*in[2][1] - in[2][0]*in[1][1] ) * invdet;
    out[2][1] = ( in[2][0]*in[0][1] - in[0][0]*in[2][1] ) * invdet;
    out[2][2] = ( in[0][0]*in[1][1] - in[1][0]*in[0][1] ) * invdet;
}

void matrixMatrixMult( double left[3][3], double right[3][3], double (&output)[3][3])
{
    for ( int i = 0; i < 3; i++ )
    {
        for ( int j = 0; j < 3; j++ )
        {
            output[i][j] = 0;
            for ( int k = 0; k < 3; k++ )
            {
                output[i][j] += left[i][k] * right[k][j];
            }
        }
    }
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
    //double BASE_TEMP =  6005.973;
    double BASE_TEMP = 6655.9928;
    double BASE_TINT = 0.97214088;

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

    double rBaseMult, gBaseMult, bBaseMult;
    //Grab the existing white balance data from the raw file.
    LibRaw imageProcessor;
#define COLOR imageProcessor.imgdata.color
#define PARAM imageProcessor.imgdata.params

    const char *cstr = inputFilename.c_str();
    if ( 0 == imageProcessor.open_file( cstr ) )
    {
        //Set the white balance arguments based on what libraw did.

        //First we need to set up the transformation from the camera's
        // raw color space to sRGB.

        //Grab the xyz2cam matrix.
        double xyzToCam[3][3];
        for ( int i = 0; i < 3; i++ )
        {
            for ( int j = 0; j < 3; j++ )
            {
                xyzToCam[i][j] = COLOR.cam_xyz[i][j];
            }
        }
        double rgbToXyz[3][3] = {
            { 0.4124, 0.3576, 0.1805 },
            { 0.2126, 0.7152, 0.0722 },
            { 0.0193, 0.1192, 0.9502 } };

        double rgbToCam[3][3];

        //Determine sRGB -> camera space matrix
        matrixMatrixMult(xyzToCam, rgbToXyz, rgbToCam);

        //Normalize it so that rgbToCam*transponse([1 1 1]) is [1 1 1]
        double sum;
        for ( int i = 0; i < 3; i++ )
        {
            sum = 0;
            for ( int j = 0; j < 3; j++ )
            {
                sum += rgbToCam[i][j];
            }
            for ( int j = 0; j < 3; j++ )
            {
                rgbToCam[i][j] /= sum;
            }
        }

        double camToRgb[3][3];
        //Take the inverse so that we have camera space -> sRGB.
        inverse( rgbToCam, camToRgb );

        //Now we divide the daylight multipliers by the camera multipliers.
        double rrBaseMult = COLOR.pre_mul[ 0 ] / COLOR.cam_mul[ 0 ];
        double grBaseMult = COLOR.pre_mul[ 1 ] / COLOR.cam_mul[ 1 ];
        double brBaseMult = COLOR.pre_mul[ 2 ] / COLOR.cam_mul[ 2 ];
        //And then we convert them from camera space to sRGB.
        matrixVectorMult( rrBaseMult, grBaseMult, brBaseMult,
                          rBaseMult,  gBaseMult,  bBaseMult,
                          camToRgb);
    }
    else //it couldn't read the file, or it wasn't raw. Either way, fallback to 1
    {
        rBaseMult = 1;
        gBaseMult = 1;
        bBaseMult = 1;
    }

    //Multiply our desired WB by the base offsets.
    rMult *= rBaseMult;
    gMult *= gBaseMult;
    bMult *= bBaseMult;

    //Normalize
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
    LibRaw imageProcessor;
    const char *cstr = inputFilename.c_str();
    if( 0 == imageProcessor.open_file( cstr ) )
    {
        double rNorm = COLOR.cam_mul[ 0 ] / COLOR.pre_mul[ 0 ];
        double gNorm = COLOR.cam_mul[ 1 ] / COLOR.pre_mul[ 1 ];
        double bNorm = COLOR.cam_mul[ 2 ] / COLOR.pre_mul[ 2 ];
        double cMin = min( min( rNorm, gNorm ), bNorm );
        rNorm /= cMin;
        gNorm /= cMin;
        bNorm /= cMin;
        rMult -= rNorm;
        gMult -= gNorm;
        bMult -= bNorm;
    }
    else
    {
        rMult -= 1;
        gMult -= 1;
        bMult -= 1;
    }
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

    double low, mid, hi, oldLow;
    low = wbDistance( file, lowCoord );
    mid = wbDistance( file, midCoord );
    hi  = wbDistance( file, hiCoord  );
    double refl, exp, cont;

#define TOLERANCE 0.000001
#define ITER_LIMIT 10000
#define REPEAT_LIMIT 5

    int iterations = 0;
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
