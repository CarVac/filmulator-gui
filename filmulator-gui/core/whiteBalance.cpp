#include "filmSim.hpp"
#include <utility>

//Generates the illuminant in XYZ for the given temperature temp.
void temp_to_XYZ(float const temp, float &X, float &Y, float &Z)
{
    //The temporary variables are in the xyY color space.
    //x and y are chromaticity.
    float temp_x;
    float temp_y;

    if(temp < 4000)
    {
        temp_x = -0.2661239 * pow(10,9) * pow(temp,-3) +
                 -0.2343589 * pow(10,6) * pow(temp,-2) +
                  0.8776956 * pow(10,3) * pow(temp,-1) +
                  0.179910;
    }
    else
    {
        temp_x = -3.0258469 * pow(10,9) * pow(temp,-3) +
                  2.1070379 * pow(10,6) * pow(temp,-2) +
                  0.2226347 * pow(10,3) * pow(temp,-1) +
                  0.24039;
    }

    if(temp < 2222)
    {
        temp_y = -1.1063814  * pow(temp_x,3) +
                 -1.34811020 * pow(temp_x,2) +
                  2.18555832 * temp_x +
                 -0.20219683;
    }
    else if(temp < 4000)
    {
        temp_y = -0.9549476  * pow(temp_x,3) +
                 -1.37418593 * pow(temp_x,2) +
                  2.09137015 * temp_x +
                 -0.16748867;
    }
    else
    {
        temp_y =  3.0817580  * pow(temp_x,3) +
                 -5.8733867 * pow(temp_x,2) +
                  3.75112997 * temp_x +
                 -0.37001483;
    }

    //Then we convert from xyY to XYZ.
    //Technically, it's like this:
    //X = xY/y
    //Y = Y
    //Z = (1-x-y)Y/y
    //However, if we set the xyY.Y equal to xyY.y, then xyY.Y/xyY.y in X and Z disappear.
    X = temp_x;
    Y = temp_y;
    Z = 1 - temp_x - temp_y;
    return;
}

//Right-multiplies the 3x3 matrix mat by column vector [r g b]'
void matrixVectorMult(float r,  float g,  float b,
                      float &x, float &y, float &z,
                      float mat[3][3])
{
    x = mat[0][0]*r + mat[0][1]*g + mat[0][2]*b;
    y = mat[1][0]*r + mat[1][1]*g + mat[1][2]*b;
    z = mat[2][0]*r + mat[2][1]*g + mat[2][2]*b;
}

//Self-explanatory.
void inverse(float in[3][3], float (&out)[3][3])
{
    float det = in[0][0] * (in[1][1]*in[2][2] - in[2][1]*in[1][2]) -
                 in[0][1] * (in[1][0]*in[2][2] - in[1][2]*in[2][0]) +
                 in[0][2] * (in[1][0]*in[2][1] - in[1][1]*in[2][0]);
    float invdet = 1 / det;

    out[0][0] = (in[1][1]*in[2][2] - in[2][1]*in[1][2]) * invdet;
    out[0][1] = (in[0][2]*in[2][1] - in[0][1]*in[2][2]) * invdet;
    out[0][2] = (in[0][1]*in[1][2] - in[0][2]*in[1][1]) * invdet;
    out[1][0] = (in[1][2]*in[2][0] - in[1][0]*in[2][2]) * invdet;
    out[1][1] = (in[0][0]*in[2][2] - in[0][2]*in[2][0]) * invdet;
    out[1][2] = (in[1][0]*in[0][2] - in[0][0]*in[1][2]) * invdet;
    out[2][0] = (in[1][0]*in[2][1] - in[2][0]*in[1][1]) * invdet;
    out[2][1] = (in[2][0]*in[0][1] - in[0][0]*in[2][1]) * invdet;
    out[2][2] = (in[0][0]*in[1][1] - in[1][0]*in[0][1]) * invdet;
}

//Self-explanatory.
void matrixMatrixMult(float left[3][3], float right[3][3], float (&output)[3][3])
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            output[i][j] = 0;
            for (int k = 0; k < 3; k++)
            {
                output[i][j] += left[i][k] * right[k][j];
            }
        }
    }
}

//Computes the white balance multipliers, given that libraw has already applied the camera's set WB.
//If we match the camera's WB, then the multipliers should be 1,1,1.
//We don't actually know what that is, so later on this function gets optimized with the goal of 1,1,1
// as the default value on importing an image.
void whiteBalanceMults(float temperature, float tint, std::string inputFilename,
                       float &rMult, float &gMult, float &bMult)
{
    //To compute the white balance, we have to reference the undo what the thingy did.
    //In order to get physically relevant temperatures, we trust dcraw
    // and by proxy libraw to give consistent WB in the
    // "daylight multipliers" (dcraw) and "pre_mul" (libraw)
    // fields.
    //
    //The following values are our baseline estimate of what this temperature
    // and tint is.
    float BASE_TEMP = 6594.9982;
    float BASE_TINT = 0.9864318;

    float rBaseMult, gBaseMult, bBaseMult;
    //Grab the existing white balance data from the raw file.
    LibRaw imageProcessor;
#define COLOR imageProcessor.imgdata.color
#define PARAM imageProcessor.imgdata.params

    const char *cstr = inputFilename.c_str();
    if (0 == imageProcessor.open_file(cstr))
    {
        //Set the white balance arguments based on what libraw did.

        //First we need to set up the transformation from the camera's
        // raw color space to sRGB.

        //Grab the xyz2cam matrix.
        float xyzToCam[3][3];
        float camToRgb[3][3];
//        cout << "white_balance: camToRgb" << endl;
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                xyzToCam[i][j] = COLOR.cam_xyz[i][j];
                camToRgb[i][j] = COLOR.rgb_cam[i][j];
//                cout << COLOR.rgb_cam[i][j] << " ";
            }
//            cout << endl;
        }
        //Now we divide the daylight multipliers by the camera multipliers.
        float rrBaseMult = COLOR.pre_mul[0] / COLOR.cam_mul[0];
        float grBaseMult = COLOR.pre_mul[1] / COLOR.cam_mul[1];
        float brBaseMult = COLOR.pre_mul[2] / COLOR.cam_mul[2];
        float rawMultMin = min(min(rrBaseMult, grBaseMult), brBaseMult);
        rrBaseMult /= rawMultMin;
        grBaseMult /= rawMultMin;
        brBaseMult /= rawMultMin;
//        cout << "white_balance raw pre_muls" << endl;
//        cout << rrBaseMult << " ";
//        cout << grBaseMult << " ";
//        cout << brBaseMult << endl;
        //And then we convert them from camera space to sRGB.
        matrixVectorMult(rrBaseMult, grBaseMult, brBaseMult,
                          rBaseMult,  gBaseMult,  bBaseMult,
                          camToRgb);
//        cout << "white_balance sRGB base_mults" << endl;
//        cout << rBaseMult << " ";
//        cout << gBaseMult << " ";
//        cout << bBaseMult << endl;
        if ((1.0f == camToRgb[0][0] && 1.0f == camToRgb[1][1] && 1.0f == camToRgb[2][2])
             || (1.0f == COLOR.pre_mul[0] && 1.0f == COLOR.pre_mul[1] && 1.0f == COLOR.pre_mul[2]))
        {
            cout << "Unity camera matrix or base multipliers. BORK" << endl;
            rBaseMult = 1;
            gBaseMult = 1;
            bBaseMult = 1;
            BASE_TEMP = 5200;
            BASE_TINT = 1;
        }
    }
    else //it couldn't read the file, or it wasn't raw. Either way, fallback to 1
    {
        rBaseMult = 1;
        gBaseMult = 1;
        bBaseMult = 1;
        BASE_TEMP = 5200;
        BASE_TINT = 1;
    }
    //The result of this is the BaseMultipliers in sRGB, which we use later.


    //Here we compute the ratio of the desired to the reference (kinda daylight) illuminant.
    //Value of the desired illuminant in XYZ coordinates.
    float XIllum, YIllum, ZIllum;
    //Value of the base illuminant in XYZ coordinates.
    float XBase, YBase, ZBase;

    //Now we compute the coordinates.
    temp_to_XYZ(temperature, XIllum, YIllum, ZIllum);
    temp_to_XYZ(BASE_TEMP, XBase, YBase, ZBase);

    //Next, we convert them to sRGB.
    float rIllum, gIllum, bIllum;
    float rBase, gBase, bBase;
    XYZ_to_sRGB(XIllum, YIllum, ZIllum,
                rIllum, gIllum, bIllum);
    XYZ_to_sRGB(XBase, YBase, ZBase,
                rBase, gBase, bBase);

    //Calculate the multipliers needed to convert from one illuminant to the base.
    gIllum /= tint;
    gBase /= BASE_TINT;
    rMult = rBase / rIllum;
    gMult = gBase / gIllum;
    bMult = bBase / bIllum;

    //cout << "white_balance: non-offset multipliers" << endl;
    //cout << rMult << endl << gMult << endl << bMult << endl;

    //Clip negative values.
    rMult = max(rMult, 0.0f);
    gMult = max(gMult, 0.0f);
    bMult = max(bMult, 0.0f);

    //Multiply our desired WB by the base offsets to compensate for
    // libraw already having applied them.
    rMult *= rBaseMult;
    gMult *= gBaseMult;
    bMult *= bBaseMult;

    //Normalize so that no component shrinks ever. (It should never go to below zero.)
    float multMin = min(min(rMult, gMult), bMult)+0.00001;
    rMult /= multMin;
    gMult /= multMin;
    bMult /= multMin;
}

//Computes the Eulerian distance from the WB coefficients to (1,1,1). Also adds the temp to it.
float wbDistance(std::string inputFilename, array<float,2> tempTint)
{
    float rMult, gMult, bMult;
    whiteBalanceMults(tempTint[0], tempTint[1], inputFilename,
                      rMult, gMult, bMult);
    rMult -= 1;
    gMult -= 1;
    bMult -= 1;

    float output;
    output = sqrt(rMult*rMult + gMult*gMult + bMult*bMult);
    return output;
}

//Run a Nelder-Mead simplex optimization on wbDistance.
void optimizeWBMults(std::string file,
                     float &temperature, float &tint)
{
    //This is nelder-mead in 2d, so we have 3 points.
    array<float,2> lowCoord, midCoord, hiCoord;
    //Some temporary coordinates for use in optimizing.
    array<float,2> meanCoord, reflCoord, expCoord, contCoord;
    //Temperature
    lowCoord[0] = 4000.0;
    midCoord[0] = 5200.0;
    hiCoord[0]  = 6300.0;
    //Tint
    lowCoord[1] = 1.0;
    midCoord[1] = 1.05;
    hiCoord[1]  = 1.0;

    float low, mid, hi, oldLow;
    low = wbDistance(file, lowCoord);
    mid = wbDistance(file, midCoord);
    hi  = wbDistance(file, hiCoord);
    float refl, exp, cont;

#define TOLERANCE 0.000000001
#define ITER_LIMIT 10000
#define REPEAT_LIMIT 30

    int iterations = 0;
    int repeats = 0;
    oldLow = 10000;

    while (repeats < REPEAT_LIMIT)
    {
        iterations++;
        if (iterations > ITER_LIMIT)
        {
            cout << "Hit iteration limit" << endl;
            temperature = 5200.0;
            tint = 1;
            return;
        }

        //Remember the low value
        //Sort the coordinates.
        if (mid > hi)
        {
            midCoord.swap(hiCoord);
            swap(mid, hi);
        }
        if (low > mid)
        {
            lowCoord.swap(midCoord);
            swap(low, mid);
        }
        if (mid > hi)
        {
            midCoord.swap(hiCoord);
            swap(mid, hi);
        }//End sort
        if (oldLow - low < TOLERANCE) //if it hasn't improved enough
        {
            repeats++;
        }
        else //if it got over a hump
        {
            oldLow = low;
            repeats = 0;
        }

        //Centroid of all but the worst point.
        meanCoord[0] = 0.5 * (lowCoord[0]  + midCoord[0]);
        meanCoord[1] = 0.5 * (lowCoord[1]  + midCoord[1]);

        //Reflect the worst point about the centroid.
        reflCoord[0] = meanCoord[0] + 1 * (meanCoord[0] - hiCoord[0]);
        reflCoord[1] = meanCoord[1] + 1 * (meanCoord[1] - hiCoord[1]);
        refl = wbDistance(file, reflCoord);
        if (refl < mid) //Better than the second-worst point
        {
            if (refl > low) //but not better than the old best point
            { //Swap this with the worst point
                hiCoord.swap(reflCoord);
                swap(hi, refl);
            } //and go back to the beginning.
            else //It's the best point so far
            { //Try a point expanded farther away in the same direction.
                expCoord[0] = meanCoord[0] + 2 * (meanCoord[0] - hiCoord[0]);
                expCoord[1] = meanCoord[1] + 2 * (meanCoord[1] - hiCoord[1]);
                exp = wbDistance(file, expCoord);
                if (exp < refl) //It is the best so far
                { //Swap this with the worst point
                    hiCoord.swap(expCoord);
                    swap(hi, exp);
                } //and go back to the beginning.
                else //The reflected point was better
                { //Swap the reflected point with the best point
                    hiCoord.swap(reflCoord);
                    swap(hi,refl);
                } //and go back.
            }
        }
        else //The reflected point was not better than the second-worst point
        { //Compute a point contracted from the worst towards the centroid.
            contCoord[0] = meanCoord[0] - 0.5 * (meanCoord[0] - hiCoord[0]);
            contCoord[1] = meanCoord[1] - 0.5 * (meanCoord[1] - hiCoord[1]);
            cont = wbDistance(file, contCoord);
            if (cont < hi) //Better than the worst point
            { //Replace the worst point with this.
                hiCoord.swap(contCoord);
                swap(hi, cont);
            } //and go back.
            else //Everything we tried was terrible
            { //Contract everything towards the best point, not the centroid.
                hiCoord[0] = lowCoord[0] + 0.5 * (lowCoord[0] - hiCoord[0]);
                hiCoord[1] = lowCoord[1] + 0.5 * (lowCoord[1] - hiCoord[1]);
                hi = wbDistance(file, hiCoord);
                midCoord[0] = lowCoord[0] + 0.5 * (lowCoord[0] - midCoord[0]);
                midCoord[1] = lowCoord[1] + 0.5 * (lowCoord[1] - midCoord[1]);
                mid = wbDistance(file, midCoord);
            }
        }
    }
    temperature = lowCoord[0];
    tint = lowCoord[1];
}

//Actually apply a white balance to image data.
//It takes in the input and output matrices, the desired temperature and tint,
// and the filename where it looks up the camera matrix and daylight multipliers.
void whiteBalance(matrix<float> &input, matrix<float> &output,
                   float temperature, float tint,
                   std::string inputFilename)
{
    float rMult, gMult, bMult;
    whiteBalanceMults(temperature, tint, inputFilename,
                       rMult, gMult, bMult);

    int nRows = input.nr();
    int nCols = input.nc();

    output.set_size(nRows, nCols);

#pragma omp parallel shared(output, input) firstprivate(nRows, nCols, rMult, gMult, bMult)
    {
#pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < nRows; i++)
        {
            for (int j = 0; j < nCols; j += 3)
            {
                output(i, j  ) = rMult*input(i, j  );
                output(i, j+1) = gMult*input(i, j+1);
                output(i, j+2) = bMult*input(i, j+2);
            }
        }
    }
}
