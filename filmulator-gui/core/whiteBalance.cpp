#include "filmSim.hpp"
#include <utility>
#include <iostream>
#include <omp.h>
#include <array>
#include <QString>

using std::cout;
using std::endl;

/*
 * cam_to_rgb, if right-multiplied by the column vector of a camera-space color, yields the sRGB colors
 *
 */

//Generates the illuminant in XYZ for the given temperature temp.
void temp_to_XYZ(float const temp, float &X, float &Y, float &Z)
{
    //The temporary variables are in the xyY color space.
    //x and y are chromaticity.
    float temp_x;
    float temp_y;

    if(temp < 4000)
    {
        temp_x = -0.2661239e9 * pow(temp,-3.0f) +
                 -0.2343589e6 * pow(temp,-2.0f) +
                  0.8776956e3 * pow(temp,-1.0f) +
                  0.179910f;
    }
    else
    {
        temp_x = -3.0258469e9 * pow(temp,-3.0f) +
                  2.1070379e6 * pow(temp,-2.0f) +
                  0.2226347e3 * pow(temp,-1.0f) +
                  0.24039f;
    }

    if(temp < 2222)
    {
        temp_y = -1.10638140f * pow(temp_x,3.0f) +
                 -1.34811020f * pow(temp_x,2.0f) +
                  2.18555832f * temp_x +
                 -0.20219683f;
    }
    else if(temp < 4000)
    {
        temp_y = -0.95494760f * pow(temp_x,3.0f) +
                 -1.37418593f * pow(temp_x,2.0f) +
                  2.09137015f * temp_x +
                 -0.16748867f;
    }
    else
    {
        temp_y =  3.08175800f * pow(temp_x,3.0f) +
                 -5.87338670f * pow(temp_x,2.0f) +
                  3.75112997f * temp_x +
                 -0.37001483f;
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
void matrixVectorMult(const float r, const float g, const float b,
                      float &x, float &y, float &z,
                      const float mat[3][3])
{
    x = mat[0][0]*r + mat[0][1]*g + mat[0][2]*b;
    y = mat[1][0]*r + mat[1][1]*g + mat[1][2]*b;
    z = mat[2][0]*r + mat[2][1]*g + mat[2][2]*b;
}

//Self-explanatory.
void inverse(const float in[3][3], float (&out)[3][3])
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
void matrixMatrixMult(const float left[3][3], const float right[3][3], float (&output)[3][3])
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

constexpr float srgb2xyz[3][3] = {
    {0.4124564, 0.3575761, 0.1804375},
    {0.2126729, 0.7151522, 0.0721750},
    {0.0193339, 0.1191920, 0.9503041}
};

//Computes the raw color space multipliers.
void whiteBalancePreMults(const float temperature, const float tint, const float cam_xyz[3][3],
                          float &rMult, float &gMult, float &bMult)
{
    //Temperature part
    float XIllum, YIllum, ZIllum;
    temp_to_XYZ(temperature, XIllum, YIllum, ZIllum);
    float rawRtemp, rawGtemp, rawBtemp;
    matrixVectorMult(XIllum, YIllum, ZIllum, rawRtemp, rawGtemp, rawBtemp, cam_xyz);

    //Tint part
    float srgb2raw[3][3];
    matrixMatrixMult(cam_xyz, srgb2xyz, srgb2raw);
    float rawRtint, rawGtint, rawBtint;
    float origRtint, origGtint, origBtint;
    //version 1: just convert 1:tint:1 from sRGB to raw color, and divide by 1:1:1 from sRGB to raw color
    /*
    matrixVectorMult(1, tint, 1, rawRtint, rawGtint, rawBtint, srgb2raw);
    matrixVectorMult(1, 1, 1, origRtint, origGtint, origBtint, srgb2raw);
    rawRtint /= origRtint;
    rawGtint /= origGtint;
    rawBtint /= origBtint;
    */

    rawRtemp = 1/rawRtemp;
    rawGtemp = 1/rawGtemp;
    rawBtemp = 1/rawBtemp;

    //version 2: apply a small change multiple times
    //((cam_xyz * sRGBd652xyz * [1; 1.1; 1;]) ./ (cam_xyz * sRGBd652xyz * [1; 1; 1;])) .^ (log(tint)/log(1.1))
    matrixVectorMult(1, 1.1, 1, rawRtint, rawGtint, rawBtint, srgb2raw);
    matrixVectorMult(1, 1, 1, origRtint, origGtint, origBtint, srgb2raw);
    rawRtint /= origRtint;
    rawGtint /= origGtint;
    rawBtint /= origBtint;
    rawRtint = pow(rawRtint, log(tint) / log(1.1));
    rawGtint = pow(rawGtint, log(tint) / log(1.1));
    rawBtint = pow(rawBtint, log(tint) / log(1.1));

    const float tempMin = min(min(rawRtemp, rawGtemp), rawBtemp);
    //Combine them
    rMult = rawRtemp * rawRtint;
    gMult = rawGtemp * rawGtint;
    bMult = rawBtemp * rawBtint;

    //Normalize
    rMult = max(rMult, 0.00001f);
    gMult = max(gMult, 0.00001f);
    bMult = max(bMult, 0.00001f);

    const float multMin = min(min(rMult, gMult), bMult);
    rMult /= multMin;
    gMult /= multMin;
    bMult /= multMin;
}

//Computes the sRGB white balance multipliers, given that libraw has already applied the camera's set WB.
//If we match the camera's WB, then the multipliers should be 1,1,1.
//We don't actually know what that is, so later on this function gets optimized with the goal of 1,1,1
// as the default value on importing an image.
void whiteBalancePostMults(float temperature, float tint, float camToRgb[3][3],
                           float rCamMul, float gCamMul, float bCamMul,
                           float rPreMul, float gPreMul, float bPreMul,
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
    float BASE_TEMP = 6594.9982f;
    float BASE_TINT = 0.9864318f;

    //Libraw's pre_mul values are obtained from cam_xyz and D65. Here's the octave code for it:
    //D65 = [.31271; .32902 .35827];
    //d65_in_cam = cam_xyz * D65;
    //pre_mul = max(d65_in_cam)./d65_in_cam;

    //float rBaseMult, gBaseMult, bBaseMult;
    //Set the white balance arguments based on what libraw did.
    cout << "whiteBalancePostMults rCamMul: " << rCamMul << endl;
    cout << "whiteBalancePostMults gCamMul: " << gCamMul << endl;
    cout << "whiteBalancePostMults bCamMul: " << bCamMul << endl;
    cout << "whiteBalancePostMults rPreMul: " << rPreMul << endl;
    cout << "whiteBalancePostMults gPreMul: " << gPreMul << endl;
    cout << "whiteBalancePostMults bPreMul: " << bPreMul << endl;
    cout << "whiteBalancePostMults cam mul harmonic mean: " << 2.0/(gCamMul/rCamMul + gCamMul/bCamMul) << endl;
    cout << "whiteBalancePostMults pre mul harmonic mean: " << 2.0/(gPreMul/rPreMul + gPreMul/bPreMul) << endl;

    //First we divide the daylight multipliers by the camera multipliers.
    //float rrBaseMult = rPreMul / rCamMul;
    //float grBaseMult = gPreMul / gCamMul;
    //float brBaseMult = bPreMul / bCamMul;
    //float rawMultMin = min(min(rrBaseMult, grBaseMult), brBaseMult);
    //rrBaseMult /= rawMultMin;
    //grBaseMult /= rawMultMin;
    //brBaseMult /= rawMultMin;
    //cout << "whiteBalancePostMults rrBaseMult: " << rrBaseMult << endl;
    //cout << "whiteBalancePostMults grBaseMult: " << grBaseMult << endl;
    //cout << "whiteBalancePostMults brBaseMult: " << brBaseMult << endl;
    for (int i = 0; i < 3; i++)
    {
        cout << "whiteBalancePostMults camToRGB: ";
        for (int j = 0; j < 3; j++)
        {
            cout << camToRgb[i][j] << " ";
        }
        cout << endl;
    }
    //And then we convert them from camera space to sRGB.
    //matrixVectorMult(rrBaseMult, grBaseMult, brBaseMult,
    //                 rBaseMult,  gBaseMult,  bBaseMult,
    //                 camToRgb);

    //if we are not handling a raw, set the base multipliers to 1 and use 5200/1 for temp/tint
    if ((1.0f == camToRgb[0][0] && 1.0f == camToRgb[1][1] && 1.0f == camToRgb[2][2])
         || (1.0f == rPreMul && 1.0f == gPreMul && 1.0f == bPreMul))
    {
    //    rBaseMult = 1;
    //    gBaseMult = 1;
    //    bBaseMult = 1;
        BASE_TEMP = 5200;
        BASE_TINT = 1;
    }
    //The result of this is the BaseMultipliers in sRGB, which we use later.
    //cout << "whiteBalancePostMults rBaseMult: " << rBaseMult << endl;
    //cout << "whiteBalancePostMults gBaseMult: " << gBaseMult << endl;
    //cout << "whiteBalancePostMults bBaseMult: " << bBaseMult << endl;


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

    //cout << "white_balance: non-offset multipliers" << endl;//===========================
    //cout << rMult << endl << gMult << endl << bMult << endl;//===========================

    //Clip negative values.
    rMult = max(rMult, 0.0f);
    gMult = max(gMult, 0.0f);
    bMult = max(bMult, 0.0f);

    //Multiply our desired WB by the base offsets to compensate for
    // already having applied them.
    //rMult *= rBaseMult;
    //gMult *= gBaseMult;
    //bMult *= bBaseMult;
    //rMult /= rBaseMult;
    //gMult /= gBaseMult;
    //bMult /= bBaseMult;

    //Normalize so that no component shrinks ever. (It should never go to below zero.)
    float multMin = min(min(rMult, gMult), bMult)+0.00001f;
    rMult /= multMin;
    gMult /= multMin;
    bMult /= multMin;
}

//Computes the Eulerian distance from the WB coefficients to (1,1,1). Also adds the temp to it.
/*
float wbDistance(array<float,2> tempTint, float camToRgb[3][3],
                 float rCamMul, float gCamMul, float bCamMul,
                 float rPreMul, float gPreMul, float bPreMul)
{
    float rMult, gMult, bMult;
    whiteBalancePostMults(tempTint[0], tempTint[1], camToRgb,
                          rCamMul, gCamMul, bCamMul,
                          rPreMul, gPreMul, bPreMul,
                          rMult, gMult, bMult);

    float rrVal, grVal, brVal; //raw color
    rrVal = rCamMul/rPreMul;
    grVal = gCamMul/gPreMul;
    brVal = bCamMul/bPreMul;
    float rVal, gVal, bVal; //sRGB
    matrixVectorMult(rrVal, grVal, brVal,
                     rVal, gVal, bVal,
                     camToRgb);

    float rFactor = rMult/rVal;
    float gFactor = gMult/gVal;
    float bFactor = bMult/bVal;

    rFactor -= 1;
    gFactor -= 1;
    bFactor -= 1;

    float output;
    output = sqrt(rFactor*rFactor + gFactor*gFactor + bFactor*bFactor);
    return output;
}
*/

//Computes the distance from the WB coefficients to the target white balance coefficients
float wbDistance(const array<float, 2> tempTint, const float cam_xyz[3][3],
                 const float rMulTarget, const float gMulTarget, const float bMulTarget)
{
    float rMult, gMult, bMult;
    whiteBalancePreMults(tempTint[0], tempTint[1], cam_xyz, rMult, gMult, bMult);
    const float rFactor = rMult/rMulTarget - 1;
    const float gFactor = gMult/gMulTarget - 1;
    const float bFactor = bMult/bMulTarget - 1;
    return sqrt(rFactor*rFactor + gFactor*gFactor + bFactor*bFactor);
}

//Run a Nelder-Mead simplex optimization on wbDistance.
void optimizeWBMults(std::string file,
                     float &temperature, float &tint,
                     const float rMul, const float gMul, const float bMul)//default to -1
{
    //Load wb params from the raw file
    std::unique_ptr<LibRaw> libraw = std::unique_ptr<LibRaw>(new LibRaw());

    //Open the file.
    int libraw_error;
#if (defined(_WIN32) || defined(__WIN32__))
    const QString tempFilename = QString::fromStdString(file);
    std::wstring wstr = tempFilename.toStdWString();
    libraw_error = libraw->open_file(wstr.c_str());
#else
    const char *cstr = file.c_str();
    libraw_error = libraw->open_file(cstr);
#endif
    if (libraw_error)
    {
        cout << "optimizeWBMults: Could not read input file!" << endl;
        cout << "libraw error text: " << libraw_strerror(libraw_error) << endl;
        temperature = 5200.0f;
        tint = 1.0f;
        return;
    }

    //float camToRGB[3][3];
    float cam_xyz[3][3];

    //get color matrix
    for (int i = 0; i < 3; i++)
    {
        //cout << "camToRGB: ";
        for (int j = 0; j < 3; j++)
        {
            //camToRGB[i][j] = libraw->imgdata.color.rgb_cam[i][j];
            cam_xyz[i][j] = libraw->imgdata.color.cam_xyz[i][j];
            //cout << camToRGB[i][j] << " ";
        }
        //cout << endl;
    }
    float rCamMul;
    float gCamMul;
    float bCamMul;
    if (rMul > 0 && gMul > 0 && bMul > 0)
    {
        rCamMul = rMul;
        gCamMul = gMul;
        bCamMul = bMul;
    } else {
        rCamMul = libraw->imgdata.color.cam_mul[0];
        gCamMul = libraw->imgdata.color.cam_mul[1];
        bCamMul = libraw->imgdata.color.cam_mul[2];
    }
    float minMult = min(min(rCamMul, gCamMul), bCamMul);
    rCamMul /= minMult;
    gCamMul /= minMult;
    bCamMul /= minMult;
    float rPreMul = libraw->imgdata.color.pre_mul[0];
    float gPreMul = libraw->imgdata.color.pre_mul[1];
    float bPreMul = libraw->imgdata.color.pre_mul[2];
    minMult = min(min(rPreMul, gPreMul), bPreMul);
    rPreMul /= minMult;
    gPreMul /= minMult;
    bPreMul /= minMult;

    //This is nelder-mead in 2d, so we have 3 points.
    array<float,2> lowCoord, midCoord, hiCoord;
    //Some temporary coordinates for use in optimizing.
    array<float,2> meanCoord, reflCoord, expCoord, contCoord;
    //Temperature
    lowCoord[0] = 4000.0f;
    midCoord[0] = 5200.0f;
    hiCoord[0]  = 6300.0f;
    //Tint
    lowCoord[1] = 1.0f;
    midCoord[1] = 1.05f;
    hiCoord[1]  = 1.0f;

    float low, mid, hi, oldLow;
    low = wbDistance(lowCoord, cam_xyz, rCamMul, gCamMul, bCamMul);
    mid = wbDistance(midCoord, cam_xyz, rCamMul, gCamMul, bCamMul);
    hi  = wbDistance(hiCoord,  cam_xyz, rCamMul, gCamMul, bCamMul);
    float refl, exp, cont;

#define TOLERANCE 0.000000001f
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
            temperature = 5200.0f;
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
        meanCoord[0] = 0.5f * (lowCoord[0]  + midCoord[0]);
        meanCoord[1] = 0.5f * (lowCoord[1]  + midCoord[1]);

        //Reflect the worst point about the centroid.
        reflCoord[0] = meanCoord[0] + 1 * (meanCoord[0] - hiCoord[0]);
        reflCoord[1] = meanCoord[1] + 1 * (meanCoord[1] - hiCoord[1]);
        refl = wbDistance(reflCoord, cam_xyz, rCamMul, gCamMul, bCamMul);
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
                exp = wbDistance(expCoord, cam_xyz, rCamMul, gCamMul, bCamMul);
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
            contCoord[0] = meanCoord[0] - 0.5f * (meanCoord[0] - hiCoord[0]);
            contCoord[1] = meanCoord[1] - 0.5f * (meanCoord[1] - hiCoord[1]);
            cont = wbDistance(contCoord, cam_xyz, rCamMul, gCamMul, bCamMul);
            if (cont < hi) //Better than the worst point
            { //Replace the worst point with this.
                hiCoord.swap(contCoord);
                swap(hi, cont);
            } //and go back.
            else //Everything we tried was terrible
            { //Contract everything towards the best point, not the centroid.
                hiCoord[0] = lowCoord[0] + 0.5f * (lowCoord[0] - hiCoord[0]);
                hiCoord[1] = lowCoord[1] + 0.5f * (lowCoord[1] - hiCoord[1]);
                hi = wbDistance(hiCoord, cam_xyz, rCamMul, gCamMul, bCamMul);
                midCoord[0] = lowCoord[0] + 0.5f * (lowCoord[0] - midCoord[0]);
                midCoord[1] = lowCoord[1] + 0.5f * (lowCoord[1] - midCoord[1]);
                mid = wbDistance(midCoord, cam_xyz, rCamMul, gCamMul, bCamMul);
            }
        }
    }
    temperature = lowCoord[0];
    tint = lowCoord[1];
}

//Undoes the camera WB which is applied before demosaicing, and then applies the user's WB
void rawWhiteBalance(const matrix<float> &input, matrix<float> &output,
                     const float temperature, const float tint, const float cam_xyz[3][3],
                     float rCamMul, float gCamMul, float bCamMul,
                     float & rUserMul, float & gUserMul, float & bUserMul)
{
    whiteBalancePreMults(temperature, tint, cam_xyz, rUserMul, gUserMul, bUserMul);
    if (rCamMul <= 0) {rCamMul = 1;}
    if (gCamMul <= 0) {gCamMul = 1;}
    if (bCamMul <= 0) {gCamMul = 1;}
    float temp_rUserMul = rUserMul/rCamMul;
    float temp_gUserMul = gUserMul/gCamMul;
    float temp_bUserMul = bUserMul/bCamMul;

    array<float, 2> tempTint;
    tempTint[0] = temperature;
    tempTint[1] = tint;
    float distance = wbDistance(tempTint, cam_xyz, rCamMul, gCamMul, bCamMul);
    cout << "rawWhiteBalance wbDistance: " << distance << endl;


    const int nRows = input.nr();
    const int nCols = input.nc();

    output.set_size(nRows, nCols);

#pragma omp parallel shared(output, input) firstprivate (nRows, nCols)
    {
#pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < nRows; i++)
        {
            for (int j = 0; j < nCols; j += 3)
            {
                output(i, j  ) = temp_rUserMul * input(i, j  );
                output(i, j+1) = temp_gUserMul * input(i, j+1);
                output(i, j+2) = temp_bUserMul * input(i, j+2);
            }
        }
    }
}

//Actually apply a white balance to image data.
//It takes in the input and output matrices, the desired temperature and tint,
// and the filename where it looks up the camera matrix and daylight multipliers.
//It simultaneously applies the camera matrix and exposure compensation while doing white balance.
//It also clips zeros at this point.
void whiteBalance(matrix<float> &input, matrix<float> &output,
                  float temperature, float tint, float cam2rgb[3][3],
                  float rCamMul, float gCamMul, float bCamMul,//what the camera asked for and is already applied
                  float rPreMul, float gPreMul, float bPreMul,//reference for camera's daylight wb
                  float expCompMult)
{
    float rMult, gMult, bMult;
    whiteBalancePostMults(temperature, tint, cam2rgb,
                          rCamMul, gCamMul, bCamMul,
                          rPreMul, gPreMul, bPreMul,
                          rMult, gMult, bMult);
    cout << "rmult: " << rMult << endl;
    cout << "gmult: " << gMult << endl;
    cout << "bmult: " << bMult << endl;
    cout << "rCamMul: " << rCamMul << endl;
    cout << "gCamMul: " << gCamMul << endl;
    cout << "bCamMul: " << bCamMul << endl;

    float transform[3][3];
    for (int i = 0; i < 3; i++)
    {
        transform[0][i] = expCompMult * rMult * cam2rgb[0][i];
        transform[1][i] = expCompMult * gMult * cam2rgb[1][i];
        transform[2][i] = expCompMult * bMult * cam2rgb[2][i];
    }

    int nRows = input.nr();
    int nCols = input.nc();

    output.set_size(nRows, nCols);

#pragma omp parallel shared(output, input) firstprivate(nRows, nCols)
    {
#pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < nRows; i++)
        {
            for (int j = 0; j < nCols; j += 3)
            {
                output(i, j  ) = max(0.0f, transform[0][0]*input(i, j) + transform[0][1]*input(i, j+1) + transform[0][2]*input(i, j+2));
                output(i, j+1) = max(0.0f, transform[1][0]*input(i, j) + transform[1][1]*input(i, j+1) + transform[1][2]*input(i, j+2));
                output(i, j+2) = max(0.0f, transform[2][0]*input(i, j) + transform[2][1]*input(i, j+1) + transform[2][2]*input(i, j+2));

            }
        }
    }
}

//Actually apply a white balance to image data.
//It takes in the input and output matrices, the desired temperature and tint,
// and the filename where it looks up the camera matrix and daylight multipliers.
//This one expects sRGB inputs to start.
//It does, however apply exposure compensation.
//It also clips zeros at this point.
void sRGBwhiteBalance(matrix<float> &input, matrix<float> &output,
                      float temperature, float tint, float cam2rgb[3][3],
                      float rCamMul, float gCamMul, float bCamMul,//what the camera asked for and is already applied
                      float rPreMul, float gPreMul, float bPreMul,//reference for camera's daylight wb
                      float expCompMult)
{
    float rMult, gMult, bMult;
    whiteBalancePostMults(temperature, tint, cam2rgb,
                          rCamMul, gCamMul, bCamMul,
                          rPreMul, gPreMul, bPreMul,
                          rMult, gMult, bMult);
    cout << "rmult: " << rMult << endl;
    cout << "gmult: " << gMult << endl;
    cout << "bmult: " << bMult << endl;
    cout << "rCamMul: " << rCamMul << endl;
    cout << "gCamMul: " << gCamMul << endl;
    cout << "bCamMul: " << bCamMul << endl;

    rMult = rMult * expCompMult;
    gMult = gMult * expCompMult;
    bMult = bMult * expCompMult;

    int nRows = input.nr();
    int nCols = input.nc();

    output.set_size(nRows, nCols);

#pragma omp parallel shared(output, input) firstprivate(nRows, nCols)
    {
#pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < nRows; i++)
        {
            for (int j = 0; j < nCols; j += 3)
            {
                output(i, j  ) = max(0.0f, rMult*input(i, j  ));
                output(i, j+1) = max(0.0f, gMult*input(i, j+1));
                output(i, j+2) = max(0.0f, bMult*input(i, j+2));

            }
        }
    }
}
