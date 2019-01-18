#include "filmSim.hpp"
#include <utility>
#include <iostream>
#include <omp.h>

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
        temp_x = -0.2661239f * pow(10.0f,9.0f) * pow(temp,-3.0f) +
                 -0.2343589f * pow(10.0f,6.0f) * pow(temp,-2.0f) +
                  0.8776956f * pow(10.0f,3.0f) * pow(temp,-1.0f) +
                  0.179910f;
    }
    else
    {
        temp_x = -3.0258469f * pow(10.0f,9.0f) * pow(temp,-3.0f) +
                  2.1070379f * pow(10.0f,6.0f) * pow(temp,-2.0f) +
                  0.2226347f * pow(10.0f,3.0f) * pow(temp,-1.0f) +
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

    float rBaseMult, gBaseMult, bBaseMult;
    //Set the white balance arguments based on what libraw did.

    //First we divide the daylight multipliers by the camera multipliers.
    float rrBaseMult = rPreMul / rCamMul;
    float grBaseMult = gPreMul / gCamMul;
    float brBaseMult = bPreMul / bCamMul;
    float rawMultMin = min(min(rrBaseMult, grBaseMult), brBaseMult);
    rrBaseMult /= rawMultMin;
    grBaseMult /= rawMultMin;
    brBaseMult /= rawMultMin;
    //And then we convert them from camera space to sRGB.
    matrixVectorMult(rrBaseMult, grBaseMult, brBaseMult,
                      rBaseMult,  gBaseMult,  bBaseMult,
                      camToRgb);
    if ((1.0f == camToRgb[0][0] && 1.0f == camToRgb[1][1] && 1.0f == camToRgb[2][2])
         || (1.0f == rPreMul && 1.0f == gPreMul && 1.0f == bPreMul))
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

    //cout << "white_balance: non-offset multipliers" << endl;//===========================
    //cout << rMult << endl << gMult << endl << bMult << endl;//===========================

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
    float multMin = min(min(rMult, gMult), bMult)+0.00001f;
    rMult /= multMin;
    gMult /= multMin;
    bMult /= multMin;
}

//Computes the Eulerian distance from the WB coefficients to (1,1,1). Also adds the temp to it.
float wbDistance(array<float,2> tempTint, float camToRgb[3][3],
                 float rCamMul, float gCamMul, float bCamMul,
                 float rPreMul, float gPreMul, float bPreMul)
{
    float rMult, gMult, bMult;
    whiteBalancePostMults(tempTint[0], tempTint[1], camToRgb,
                          rCamMul, gCamMul, bCamMul,
                          rPreMul, gPreMul, bPreMul,
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
    //Load wb params from the raw file
    std::unique_ptr<LibRaw> image_processor = std::unique_ptr<LibRaw>(new LibRaw());

    //Open the file.
    const char *cstr = file.c_str();
    if (0 != image_processor->open_file(cstr))
    {
        cout << "processImage: Could not read input file!" << endl;
    }

    float camToRGB[3][3];

    //get color matrix
    for (int i = 0; i < 3; i++)
    {
        cout << "camToRGB: ";
        for (int j = 0; j < 3; j++)
        {
            camToRGB[i][j] = image_processor->imgdata.color.rgb_cam[i][j];
            cout << camToRGB[i][j] << " ";
        }
        cout << endl;
    }
    float rCamMul = image_processor->imgdata.color.cam_mul[0];
    float gCamMul = image_processor->imgdata.color.cam_mul[1];
    float bCamMul = image_processor->imgdata.color.cam_mul[2];
    float minMult = min(min(rCamMul, gCamMul), bCamMul);
    rCamMul /= minMult;
    gCamMul /= minMult;
    bCamMul /= minMult;
    float rPreMul = image_processor->imgdata.color.pre_mul[0];
    float gPreMul = image_processor->imgdata.color.pre_mul[1];
    float bPreMul = image_processor->imgdata.color.pre_mul[2];
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
    low = wbDistance(lowCoord, camToRGB, rCamMul, gCamMul, bCamMul, rPreMul, gPreMul, bPreMul);
    mid = wbDistance(midCoord, camToRGB, rCamMul, gCamMul, bCamMul, rPreMul, gPreMul, bPreMul);
    hi  = wbDistance(hiCoord,  camToRGB, rCamMul, gCamMul, bCamMul, rPreMul, gPreMul, bPreMul);
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
        refl = wbDistance(reflCoord, camToRGB, rCamMul, gCamMul, bCamMul, rPreMul, gPreMul, bPreMul);
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
                exp = wbDistance(expCoord, camToRGB, rCamMul, gCamMul, bCamMul, rPreMul, gPreMul, bPreMul);
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
            cont = wbDistance(contCoord, camToRGB, rCamMul, gCamMul, bCamMul, rPreMul, gPreMul, bPreMul);
            if (cont < hi) //Better than the worst point
            { //Replace the worst point with this.
                hiCoord.swap(contCoord);
                swap(hi, cont);
            } //and go back.
            else //Everything we tried was terrible
            { //Contract everything towards the best point, not the centroid.
                hiCoord[0] = lowCoord[0] + 0.5f * (lowCoord[0] - hiCoord[0]);
                hiCoord[1] = lowCoord[1] + 0.5f * (lowCoord[1] - hiCoord[1]);
                hi = wbDistance(hiCoord, camToRGB, rCamMul, gCamMul, bCamMul, rPreMul, gPreMul, bPreMul);
                midCoord[0] = lowCoord[0] + 0.5f * (lowCoord[0] - midCoord[0]);
                midCoord[1] = lowCoord[1] + 0.5f * (lowCoord[1] - midCoord[1]);
                mid = wbDistance(midCoord, camToRGB, rCamMul, gCamMul, bCamMul, rPreMul, gPreMul, bPreMul);
            }
        }
    }
    temperature = lowCoord[0];
    tint = lowCoord[1];
}

//Actually apply a white balance to image data.
//It takes in the input and output matrices, the desired temperature and tint,
// and the filename where it looks up the camera matrix and daylight multipliers.
//It also takes in the camera matrix and the raw color space WB multipliers.
void whiteBalance(matrix<float> &input, matrix<float> &output,
                  float temperature, float tint, float cam2rgb[3][3],
                  float rCamMul, float gCamMul, float bCamMul,
                  float rPreMul, float gPreMul, float bPreMul,
                  float maxValue)
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
        transform[0][i] = rMult * cam2rgb[0][i];
        transform[1][i] = gMult * cam2rgb[1][i];
        transform[2][i] = bMult * cam2rgb[2][i];
    }
    //transform[0][0] = 1.0f;
    //transform[1][1] = 1.0f;
    //transform[2][2] = 1.0f;

    int nRows = input.nr();
    int nCols = input.nc();

    output.set_size(nRows, nCols);

    /*
    int case1 = 0;
    int case2 = 0;
    int case3 = 0;
    int case4 = 0;
    int case5 = 0;
    int case6 = 0;
    int case7 = 0;
    */

#pragma omp parallel shared(output, input) firstprivate(nRows, nCols)
    {
#pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < nRows; i++)
        {
            for (int j = 0; j < nCols; j += 3)
            {
                //highlight handling
                //If the channel with the lowest camera multipliers is clipped, then clip the other channels in that pixel so as to not drag down the brightness
                /*
                bool rClipped = (input(i,j  ) > maxValue);
                bool gClipped = (input(i,j+1) > maxValue);
                bool bClipped = (input(i,j+2) > maxValue);

                if (rClipped && !gClipped && !bClipped) //only red is clipped
                {
                    case1++;
                    input(i,j+1) = min(input(i,j+1),input(i,j+1)*rCamMul/gCamMul); //reduce green if necessary
                    input(i,j+2) = min(input(i,j+2),input(i,j+2)*rCamMul/bCamMul); //reduce blue if necessary
                }
                if (!rClipped && gClipped && !bClipped)
                {
                    case2++;
                    input(i,j  ) = min(input(i,j  ),input(i,j  )*gCamMul/rCamMul);
                    input(i,j+2) = min(input(i,j+2),input(i,j+2)*gCamMul/bCamMul);
                }
                if (!rClipped && !gClipped && bClipped)
                {
                    case3++;
                    input(i,j  ) = min(input(i,j  ),input(i,j  )*bCamMul/rCamMul);
                    input(i,j+1) = min(input(i,j+1),input(i,j+1)*bCamMul/gCamMul);
                }

                if (rClipped && gClipped && !bClipped) //red and green are both clipped;
                {
                    case4++;
                    input(i,j  ) = min(input(i,j  ),input(i,j  )*min(rCamMul,gCamMul)/rCamMul);//reduce relative to the clipped channel with the lowest multpilier
                    input(i,j+1) = min(input(i,j+1),input(i,j+1)*min(rCamMul,gCamMul)/gCamMul);
                    input(i,j+2) = min(input(i,j+2),input(i,j+2)*min(rCamMul,gCamMul)/bCamMul);
                }
                if (rClipped && !gClipped && bClipped)
                {
                    case5++;
                    input(i,j  ) = min(input(i,j  ),input(i,j  )*min(rCamMul,bCamMul)/rCamMul);
                    input(i,j+1) = min(input(i,j+1),input(i,j+1)*min(rCamMul,bCamMul)/gCamMul);
                    input(i,j+2) = min(input(i,j+2),input(i,j+2)*min(rCamMul,bCamMul)/bCamMul);
                }
                if (!rClipped && gClipped && bClipped)
                {
                    case6++;
                    input(i,j  ) = min(input(i,j  ),input(i,j  )*min(gCamMul,bCamMul)/rCamMul);
                    input(i,j+1) = min(input(i,j+1),input(i,j+1)*min(gCamMul,bCamMul)/gCamMul);
                    input(i,j+2) = min(input(i,j+2),input(i,j+2)*min(gCamMul,bCamMul)/bCamMul);
                }
                if (rClipped && gClipped && bClipped) //all channels are clipped
                {
                    case7++;
                    input(i,j  ) = min(input(i,j  ),input(i,j  )*min(min(rCamMul,gCamMul),bCamMul)/rCamMul);
                    input(i,j+1) = min(input(i,j+1),input(i,j+1)*min(min(rCamMul,gCamMul),bCamMul)/gCamMul);
                    input(i,j+2) = min(input(i,j+2),input(i,j+2)*min(min(rCamMul,gCamMul),bCamMul)/bCamMul);
                }
                */

                //Actually set output according to camera matrix and pre and post multipliers
                output(i, j  ) = max(0.0f, transform[0][0]*rCamMul*input(i, j) + transform[0][1]*gCamMul*input(i, j+1) + transform[0][2]*bCamMul*input(i, j+2));
                output(i, j+1) = max(0.0f, transform[1][0]*rCamMul*input(i, j) + transform[1][1]*gCamMul*input(i, j+1) + transform[1][2]*bCamMul*input(i, j+2));
                output(i, j+2) = max(0.0f, transform[2][0]*rCamMul*input(i, j) + transform[2][1]*gCamMul*input(i, j+1) + transform[2][2]*bCamMul*input(i, j+2));

                /*
                if (rClipped || gClipped || bClipped)
                {
                    if (i%2 == 0)
                    {
                        output(i,j+0) = 0.0f;
                        output(i,j+1) = 0.0f;
                        output(i,j+2) = 0.0f;
                    }
                }*/
            }
        }
    }

    /*
    cout << "case1: " << case1 << endl;
    cout << "case2: " << case2 << endl;
    cout << "case3: " << case3 << endl;
    cout << "case4: " << case4 << endl;
    cout << "case5: " << case5 << endl;
    cout << "case6: " << case6 << endl;
    cout << "case7: " << case7 << endl;
    */
}
