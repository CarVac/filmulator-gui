////////////////////////////////////////////////////////////////
//
//          CFA denoise by wavelet transform, FT filtering
//
//  copyright (c) 2008-2012  Emil Martinec <ejmartin@uchicago.edu>
//
//
//  code dated: March 9, 2012
//  heavily modified by Carlo Vaccari 2012-06-12
//
//  FTblockDN.cc is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////

#include <cmath>
#include <memory>

#include "../matrix.hpp"
#include "../../ui/parameterManager.h"

//#include "array2D.h"
#include "boxblur.h"
#include "cplx_wavelet_dec.h"
//#include "color.h"
//#include "curves.h"
#include "helpersse2.h"
//#include "iccmatrices.h"
//#include "iccstore.h"
//#include "imagefloat.h"
//#include "improcfun.h"
#include "labimage.h"
//#include "LUT.h"
//#include "mytime.h"
#include "opthelper.h"
//#include "procparams.h"
#include "rt_math.h"
#include "sleef.h"
#include "sleefsseavx.h"
//#include "../rtgui/threadutils.h"
//#include "../rtgui/options.h"

//#include <omp.h>

//#define BENCHMARK
//#include "StopWatch.h"

#define TS 64       // Tile size
#define offset 25   // shift between tiles
#define blkrad 1    // radius of block averaging

#define epsilon 0.001f/(TS*TS) //tolerance


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
/*
 Structure of the algorithm:

 1. Compute an initial denoise of the image via undecimated wavelet transform
 and universal thresholding modulated by user input.
 2. Decompose the residual image into TSxTS size tiles, shifting by 'offset' each step
 (so roughly each pixel is in (TS/offset)^2 tiles); Discrete Cosine transform the tiles.
 3. Filter the DCT data to pick out patterns missed by the wavelet denoise
 4. Inverse DCT the denoised tile data and combine the tiles into a denoised output image.

 */

void Tile_calc(int tilesize, int overlap, int kall, int imwidth, int imheight, int &numtiles_W, int &numtiles_H, int &tilewidth, int &tileheight, int &tileWskip, int &tileHskip)

{
    if (kall == 2) {

        if (imwidth < tilesize) {
            numtiles_W = 1;
            tileWskip = imwidth;
            tilewidth = imwidth;
        } else {
            numtiles_W = ceil((static_cast<float>(imwidth)) / (tilesize - overlap));
            tilewidth  = ceil((static_cast<float>(imwidth)) / (numtiles_W)) + overlap;
            tilewidth += (tilewidth & 1);
            tileWskip = tilewidth - overlap;
        }

        if (imheight < tilesize) {
            numtiles_H = 1;
            tileHskip = imheight;
            tileheight = imheight;
        } else {
            numtiles_H = ceil((static_cast<float>(imheight)) / (tilesize - overlap));
            tileheight = ceil((static_cast<float>(imheight)) / (numtiles_H)) + overlap;
            tileheight += (tileheight & 1);
            tileHskip = tileheight - overlap;
        }
    }

    if (kall == 0) {
        numtiles_W = 1;
        tileWskip = imwidth;
        tilewidth = imwidth;
        numtiles_H = 1;
        tileHskip = imheight;
        tileheight = imheight;
    }

    //  printf("Nw=%d NH=%d tileW=%d tileH=%d\n",numtiles_W,numtiles_H,tileWskip,tileHskip);
}

enum nrquality {QUALITY_STANDARD, QUALITY_HIGH};

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


/*
#undef TS
#undef fTS
#undef offset
#undef epsilon
*/

float Mad(const float * DataList, const int datalen)
{
    if (datalen <= 1) { // Avoid possible buffer underrun
        return 0;
    }

    //computes Median Absolute Deviation
    //DataList values should mostly have abs val < 256 because we are in Lab mode (32768)
    int histo[32768] ALIGNED64 = {0};

    //calculate histogram of absolute values of wavelet coeffs
    for (int i = 0; i < datalen; ++i) {
        histo[static_cast<int>(rtengine::min(32768.f, fabsf(DataList[i])))]++;
    }

    //find median of histogram
    int lmedian = 0, count = 0;

    while (count < datalen / 2) {
        count += histo[lmedian];
        ++lmedian;
    }

    int count_ = count - histo[lmedian - 1];

    // interpolate
    return ((lmedian - 1) + (datalen / 2 - count_) / (static_cast<float>(count - count_))) / 0.6745f;
}

float MadRgb(const float * DataList, const int datalen)
{
    if (datalen <= 1) { // Avoid possible buffer underrun
        return 0;
    }

    //computes Median Absolute Deviation
    //DataList values should mostly have abs val < 65536 because we are in RGB mode
    int * histo = new int[65536];

    for (int i = 0; i < 65536; ++i) {
        histo[i] = 0;
    }

    //calculate histogram of absolute values of wavelet coeffs
    for (int i = 0; i < datalen; ++i) {
        histo[static_cast<int>(rtengine::min(65535.f, fabsf(DataList[i])))]++;
    }

    //find median of histogram
    int lmedian = 0, count = 0;

    while (count < datalen / 2) {
        count += histo[lmedian];
        ++lmedian;
    }

    int count_ = count - histo[lmedian - 1];

    // interpolate
    delete[] histo;
    return ((lmedian - 1) + (datalen / 2 - count_) / (static_cast<float>(count - count_))) / 0.6745f;
}

void Noise_residualAB(const wavelet_decomposition &WaveletCoeffs_ab, float &chresid, float &chmaxresid, bool denoiseMethodRgb, int denoiseNestedLevels)
{

    float resid = 0.f;
    float maxresid = 0.f;

#ifdef _OPENMP
    #pragma omp parallel for schedule(dynamic) collapse(2) reduction(+:resid) reduction(max:maxresid) num_threads(denoiseNestedLevels) if (denoiseNestedLevels>1)
#endif
    for (int lvl = 0; lvl < WaveletCoeffs_ab.maxlevel(); ++lvl) {
        // compute median absolute deviation (MAD) of detail coefficients as robust noise estimator
        for (int dir = 1; dir < 4; ++dir) {
            const int Wlvl_ab = WaveletCoeffs_ab.level_W(lvl);
            const int Hlvl_ab = WaveletCoeffs_ab.level_H(lvl);

            const float* const* WavCoeffs_ab = WaveletCoeffs_ab.level_coeffs(lvl);
            const float madC = rtengine::SQR(denoiseMethodRgb ? MadRgb(WavCoeffs_ab[dir], Wlvl_ab * Hlvl_ab) : Mad(WavCoeffs_ab[dir], Wlvl_ab * Hlvl_ab));

            resid += madC;

            if (madC > maxresid) {
                maxresid = madC;
            }
        }
    }

    chresid = resid;
    chmaxresid = maxresid;
}

void ShrinkAllAB(wavelet_decomposition& WaveletCoeffs_L, wavelet_decomposition& WaveletCoeffs_ab, float **buffer, int level, int dir,
        float * noisevarchrom, float noisevar_ab, bool autoch,
        bool denoiseMethodRgb, float * madL,  float * madaab,  bool madCalculated)

{
    //simple wavelet shrinkage
    const float eps = 0.01f;

    if (autoch && noisevar_ab <= 0.001f) {
        noisevar_ab = 0.02f;
    }

    float * sfaveab = buffer[0] + 32;
    float * sfaveabd = buffer[1] + 64;
 //   float * blurBuffer = buffer[2] + 96;

    int W_ab = WaveletCoeffs_ab.level_W(level);
    int H_ab = WaveletCoeffs_ab.level_H(level);

    float* const* WavCoeffs_L = WaveletCoeffs_L.level_coeffs(level);
    float* const* WavCoeffs_ab = WaveletCoeffs_ab.level_coeffs(level);

    float madab;
    float mad_L = madL[dir - 1];

    if (madCalculated) {
        madab = madaab[dir - 1];
    } else {
        if (!denoiseMethodRgb) {
            madab = rtengine::SQR(Mad(WavCoeffs_ab[dir], W_ab * H_ab));
        } else {
            madab = rtengine::SQR(MadRgb(WavCoeffs_ab[dir], W_ab * H_ab));
        }
    }
    float noisevarfc;

    float *nvc = nullptr;
    nvc = new float[ H_ab * W_ab];

    noisevarfc = noisevar_ab;

    for (int p = 0; p < H_ab * W_ab; p++) {
        nvc[p] = noisevarchrom[p];
    }


 // printf("varfc=%f nvc0=%f nvc1=%f nvc2=%f\n",  noisevarfc, nvc[10], nvc[H_ab * W_ab /3], nvc[H_ab * W_ab /2]);
    if (noisevarfc > 0.001f) {//noisevar_ab
        madab = madab * noisevarfc;
#ifdef __SSE2__
        vfloat onev = F2V(1.f);
        vfloat mad_abrv = F2V(madab);

        vfloat rmadLm9v = onev / F2V(mad_L * 9.f);
        vfloat mad_abv ;
        vfloat mag_Lv, mag_abv;

        int coeffloc_ab;

        for (coeffloc_ab = 0; coeffloc_ab < H_ab * W_ab - 3; coeffloc_ab += 4) {
            mad_abv = LVFU(nvc[coeffloc_ab]) * mad_abrv;

            mag_Lv = LVFU(WavCoeffs_L[dir][coeffloc_ab]);
            mag_abv = SQRV(LVFU(WavCoeffs_ab[dir][coeffloc_ab]));
            mag_Lv = (SQRV(mag_Lv)) * rmadLm9v;
            STVFU(sfaveab[coeffloc_ab], (onev - xexpf(-(mag_abv / mad_abv) - (mag_Lv))));
        }

        // few remaining pixels
        for (; coeffloc_ab < H_ab * W_ab; ++coeffloc_ab) {
            float mag_L = rtengine::SQR(WavCoeffs_L[dir][coeffloc_ab]);
            float mag_ab = rtengine::SQR(WavCoeffs_ab[dir][coeffloc_ab]);
            sfaveab[coeffloc_ab] = (1.f - xexpf(-(mag_ab / (nvc[coeffloc_ab] * madab)) - (mag_L / (9.f * mad_L))));
        }//now chrominance coefficients are denoised

#else

        for (int i = 0; i < H_ab; ++i) {
            for (int j = 0; j < W_ab; ++j) {
                int coeffloc_ab = i * W_ab + j;
                float mag_L = rtengine::SQR(WavCoeffs_L[dir][coeffloc_ab]);
                float mag_ab = rtengine::SQR(WavCoeffs_ab[dir][coeffloc_ab]);
                sfaveab[coeffloc_ab] = (1.f - xexpf(-(mag_ab / (nvc[coeffloc_ab] * madab)) - (mag_L / (9.f * mad_L))));
            }
        }//now chrominance coefficients are denoised

#endif
        boxblur(sfaveab, sfaveabd, level + 2, W_ab, H_ab, false); //increase smoothness by locally averaging shrinkage

//        boxblur(sfaveab, sfaveabd, blurBuffer, level + 2, level + 2, W_ab, H_ab); //increase smoothness by locally averaging shrinkage
#ifdef __SSE2__
        vfloat epsv = F2V(eps);
        vfloat sfabv;
        vfloat sfaveabv;

        for (coeffloc_ab = 0; coeffloc_ab < H_ab * W_ab - 3; coeffloc_ab += 4) {
            sfabv = LVFU(sfaveab[coeffloc_ab]);
            sfaveabv = LVFU(sfaveabd[coeffloc_ab]);

            //use smoothed shrinkage unless local shrinkage is much less
            STVFU(WavCoeffs_ab[dir][coeffloc_ab], LVFU(WavCoeffs_ab[dir][coeffloc_ab]) * (SQRV(sfaveabv) + SQRV(sfabv)) / (sfaveabv + sfabv + epsv));
        }

        // few remaining pixels
        for (; coeffloc_ab < H_ab * W_ab; ++coeffloc_ab) {
            //modification Jacques feb 2013
            float sfab = sfaveab[coeffloc_ab];

            //use smoothed shrinkage unless local shrinkage is much less
            WavCoeffs_ab[dir][coeffloc_ab] *= (rtengine::SQR(sfaveabd[coeffloc_ab]) + rtengine::SQR(sfab)) / (sfaveabd[coeffloc_ab] + sfab + eps);
        }//now chrominance coefficients are denoised

#else

        for (int i = 0; i < H_ab; ++i) {
            for (int j = 0; j < W_ab; ++j) {
                int coeffloc_ab = i * W_ab + j;
                float sfab = sfaveab[coeffloc_ab];

                //use smoothed shrinkage unless local shrinkage is much less
                WavCoeffs_ab[dir][coeffloc_ab] *= (rtengine::SQR(sfaveabd[coeffloc_ab]) + rtengine::SQR(sfab)) / (sfaveabd[coeffloc_ab] + sfab + eps);
            }//now chrominance coefficients are denoised
        }

#endif
    }

    delete [] nvc;
}

bool WaveletDenoiseAll_BiShrinkAB(wavelet_decomposition& WaveletCoeffs_L, wavelet_decomposition& WaveletCoeffs_ab, float *noisevarchrom, float madL[8][3], int local, float noisevar_ab, bool autoch, bool denoiseMethodRgb, int denoiseNestedLevels)
{
    int maxlvl = WaveletCoeffs_L.maxlevel();
    printf("Ftblockdn ab bishrink\n");

    if (local == 1) {
        maxlvl = 6;    //for local denoise
    }


    if (local == 2) {
        maxlvl = 7;    //for local denoise
    }

    if (local == 3) {
        maxlvl = 4;    //for shape detection
    }

    if (autoch && noisevar_ab <= 0.001f) {
        noisevar_ab = 0.02f;
    }

    float madab[8][3];

    int maxWL = 0, maxHL = 0;

    for (int lvl = 0; lvl < maxlvl; ++lvl) {
        if (WaveletCoeffs_L.level_W(lvl) > maxWL) {
            maxWL = WaveletCoeffs_L.level_W(lvl);
        }

        if (WaveletCoeffs_L.level_H(lvl) > maxHL) {
            maxHL = WaveletCoeffs_L.level_H(lvl);
        }
    }

    bool memoryAllocationFailed = false;
#ifdef _OPENMP
    #pragma omp parallel num_threads(denoiseNestedLevels) if (denoiseNestedLevels>1)
#endif
    {
        float *buffer[3];
        buffer[0] = new (std::nothrow) float[maxWL * maxHL + 32];
        buffer[1] = new (std::nothrow) float[maxWL * maxHL + 64];
        buffer[2] = new (std::nothrow) float[maxWL * maxHL + 96];

        if (buffer[0] == nullptr || buffer[1] == nullptr || buffer[2] == nullptr) {
            memoryAllocationFailed = true;
        }

        if (!memoryAllocationFailed) {


#ifdef _OPENMP
            #pragma omp for schedule(dynamic) collapse(2)
#endif

            for (int lvl = 0; lvl < maxlvl; ++lvl) {
                for (int dir = 1; dir < 4; ++dir) {
                    // compute median absolute deviation (MAD) of detail coefficients as robust noise estimator
                    int Wlvl_ab = WaveletCoeffs_ab.level_W(lvl);
                    int Hlvl_ab = WaveletCoeffs_ab.level_H(lvl);
                    const float* const* WavCoeffs_ab = WaveletCoeffs_ab.level_coeffs(lvl);

                    if (!denoiseMethodRgb) {
                        madab[lvl][dir - 1] = rtengine::SQR(Mad(WavCoeffs_ab[dir], Wlvl_ab * Hlvl_ab));
                    } else {
                        madab[lvl][dir - 1] = rtengine::SQR(MadRgb(WavCoeffs_ab[dir], Wlvl_ab * Hlvl_ab));
                    }
                }
            }

#ifdef _OPENMP
            #pragma omp for schedule(dynamic) collapse(2)
#endif

            for (int lvl = maxlvl - 1; lvl >= 0; lvl--) { //for levels less than max, use level diff to make edge mask
                for (int dir = 1; dir < 4; ++dir) {
                    int Wlvl_ab = WaveletCoeffs_ab.level_W(lvl);
                    int Hlvl_ab = WaveletCoeffs_ab.level_H(lvl);

                    float* const* WavCoeffs_L = WaveletCoeffs_L.level_coeffs(lvl);
                    float* const* WavCoeffs_ab = WaveletCoeffs_ab.level_coeffs(lvl);

                    if (lvl == maxlvl - 1) {
                        //printf("Shrink ab bis\n");
                        ShrinkAllAB(WaveletCoeffs_L, WaveletCoeffs_ab, buffer, lvl, dir, noisevarchrom, noisevar_ab, autoch, denoiseMethodRgb, madL[lvl], madab[lvl], true);
                    } else {
                        //simple wavelet shrinkage
                        float noisevarfc;

                        float mad_Lr = madL[lvl][dir - 1];
                        float *nvc = nullptr;
                        nvc = new float[Hlvl_ab * Wlvl_ab];

                        noisevarfc = noisevar_ab;

                        for (int p = 0; p < Hlvl_ab * Wlvl_ab; p++) {
                            nvc[p] = noisevarchrom[p];
                        }

                        float mad_abr = rtengine::SQR(noisevarfc) * madab[lvl][dir - 1];

                        if (noisevarfc > 0.001f) {

#ifdef __SSE2__
                            vfloat onev = F2V(1.f);
                            vfloat mad_abrv = F2V(mad_abr);
                            vfloat rmad_Lm9v = onev / F2V(mad_Lr * 9.f);
                            vfloat mad_abv;
                            vfloat mag_Lv, mag_abv;
                            vfloat tempabv;
                            int coeffloc_ab;

                            for (coeffloc_ab = 0; coeffloc_ab < Hlvl_ab * Wlvl_ab - 3; coeffloc_ab += 4) {
                                mad_abv = LVFU(nvc[coeffloc_ab]) * mad_abrv;

                                tempabv = LVFU(WavCoeffs_ab[dir][coeffloc_ab]);
                                mag_Lv = LVFU(WavCoeffs_L[dir][coeffloc_ab]);
                                mag_abv = SQRV(tempabv);
                                mag_Lv = SQRV(mag_Lv) * rmad_Lm9v;
                                STVFU(WavCoeffs_ab[dir][coeffloc_ab], tempabv * SQRV((onev - xexpf(-(mag_abv / mad_abv) - (mag_Lv)))));
                            }

                            // few remaining pixels
                            for (; coeffloc_ab < Hlvl_ab * Wlvl_ab; ++coeffloc_ab) {
                                float mag_L = rtengine::SQR(WavCoeffs_L[dir][coeffloc_ab ]);
                                float mag_ab = rtengine::SQR(WavCoeffs_ab[dir][coeffloc_ab]);
                                WavCoeffs_ab[dir][coeffloc_ab] *= rtengine::SQR(1.f - xexpf(-(mag_ab / (nvc[coeffloc_ab] * mad_abr)) - (mag_L / (9.f * mad_Lr)))/*satfactor_a*/);
                            }//now chrominance coefficients are denoised

#else

                            for (int i = 0; i < Hlvl_ab; ++i) {
                                for (int j = 0; j < Wlvl_ab; ++j) {
                                    int coeffloc_ab = i * Wlvl_ab + j;

                                    float mag_L = rtengine::SQR(WavCoeffs_L[dir][coeffloc_ab ]);
                                    float mag_ab = rtengine::SQR(WavCoeffs_ab[dir][coeffloc_ab]);

                                    WavCoeffs_ab[dir][coeffloc_ab] *= rtengine::SQR(1.f - xexpf(-(mag_ab / (nvc[coeffloc_ab] * mad_abr)) - (mag_L / (9.f * mad_Lr)))/*satfactor_a*/);

                                }
                            }//now chrominance coefficients are denoised

#endif
                        }

                        delete [] nvc;

                    }
                }
            }

        }

        for (int i = 2; i >= 0; i--) {
            delete[] buffer[i];
        }

    }
    return (!memoryAllocationFailed);
}

bool WaveletDenoiseAllAB(wavelet_decomposition& WaveletCoeffs_L, wavelet_decomposition& WaveletCoeffs_ab,
        float *noisevarchrom, float madL[8][3], int local, float noisevar_ab, bool autoch, bool denoiseMethodRgb, int denoiseNestedLevels)//mod JD

{
    int maxlvl = WaveletCoeffs_L.maxlevel();

    if (local == 1) {
        maxlvl = 6;    //for local denoise
    }


    if (local == 2) {
        maxlvl = 7;    //for local denoise
    }

    if (local == 3) {
        maxlvl = 4;    //for shape detection
    }

    int maxWL = 0, maxHL = 0;

    for (int lvl = 0; lvl < maxlvl; ++lvl) {
        if (WaveletCoeffs_L.level_W(lvl) > maxWL) {
            maxWL = WaveletCoeffs_L.level_W(lvl);
        }

        if (WaveletCoeffs_L.level_H(lvl) > maxHL) {
            maxHL = WaveletCoeffs_L.level_H(lvl);
        }
    }

    bool memoryAllocationFailed = false;
#ifdef _OPENMP
    #pragma omp parallel num_threads(denoiseNestedLevels) if (denoiseNestedLevels>1)
#endif
    {
        float *buffer[3];
        buffer[0] = new (std::nothrow) float[maxWL * maxHL + 32];
        buffer[1] = new (std::nothrow) float[maxWL * maxHL + 64];
        buffer[2] = new (std::nothrow) float[maxWL * maxHL + 96];

        if (buffer[0] == nullptr || buffer[1] == nullptr || buffer[2] == nullptr) {
            memoryAllocationFailed = true;
        }

        if (!memoryAllocationFailed) {
#ifdef _OPENMP
            #pragma omp for schedule(dynamic) collapse(2) nowait
#endif

            for (int lvl = 0; lvl < maxlvl; ++lvl) {
                for (int dir = 1; dir < 4; ++dir) {
                    ShrinkAllAB(WaveletCoeffs_L, WaveletCoeffs_ab, buffer, lvl, dir, noisevarchrom, noisevar_ab, autoch, denoiseMethodRgb, madL[lvl], nullptr, 0);
                }
            }
        }

        for (int i = 2; i >= 0; i--) {
            if (buffer[i] != nullptr) {
                delete[] buffer[i];
            }
        }
    }
    return (!memoryAllocationFailed);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void RGB_denoise(int kall,
                 matrix<float> &src,
                 matrix<float> &dst,
                 const float chroma,
                 const float redchro,
                 const float bluechro,
                 ParameterManager* paramManager)
{
//BENCHFUN
//    MyTime t1e, t2e;
//    t1e.set();

    if (chroma == 0) {
        //nothing to do; copy src to dst or do nothing in case src == dst
        dst = src;

        return;
    }

    const bool autoch = false;

    const bool denoiseMethodRgb = false;

    const int imheight = src.nr(), imwidth = src.nc()/3;

    if (chroma != 0) {

        matrix<float> tilemask_out(TS, TS);

        int tilesize = 0;
        int overlap = 0;

        //if (settings->leveldnti == 0) {
            tilesize = 1024;
            overlap = 128;
        //}

        //if (settings->leveldnti == 1) {
        //    tilesize = 768;
        //    overlap = 96;
        //}

        int numTries = 0;

        bool memoryAllocationFailed = false;

        do {
            ++numTries;

            if (numTries == 2) {
                printf("1st denoise pass failed due to insufficient memory, starting 2nd (tiled) pass now...\n");
            }

            int numtiles_W, numtiles_H, tilewidth, tileheight, tileWskip, tileHskip;

            Tile_calc(tilesize, overlap, (numTries == 1 ? 0 : 2), imwidth, imheight, numtiles_W, numtiles_H, tilewidth, tileheight, tileWskip, tileHskip);
            memoryAllocationFailed = false;
            const int numtiles = numtiles_W * numtiles_H;

            //output buffer
            //Imagefloat * dsttmp;
            matrix<float> dsttmp(imheight, imwidth*3);

#ifdef _OPENMP
            #pragma omp parallel for
#endif

            for (int i = 0; i < imheight; ++i) {
                for (int j = 0; j < imwidth; ++j) {
                    dsttmp(i, j*3 + 0) = 0.f;
                    dsttmp(i, j*3 + 1) = 0.f;
                    dsttmp(i, j*3 + 2) = 0.f;
                }
            }

            //now we have tile dimensions, overlaps
            //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

#ifndef _OPENMP
            int numthreads = 1;
#else
            // Calculate number of tiles. If less than omp_get_max_threads(), then limit num_threads to number of tiles
            int numthreads = min(numtiles, omp_get_max_threads());

#ifdef _OPENMP
            int denoiseNestedLevels = omp_get_max_threads() / numthreads;
            bool oldNested = omp_get_nested();

            if (denoiseNestedLevels < 2) {
                denoiseNestedLevels = 1;
            } else {
                omp_set_nested(true);
            }

#endif

            printf("RGB_denoise uses %d main thread(s) and up to %d nested thread(s) for each main thread\n", numthreads, denoiseNestedLevels);

#endif
            const std::size_t blox_array_size = denoiseNestedLevels * numthreads;

            // begin tile processing of image
#ifdef _OPENMP
            #pragma omp parallel num_threads(numthreads) if (numthreads>1)
#endif
            {
                int pos;
                float* noisevarchrom;

                noisevarchrom = new float[((tileheight + 1) / 2) * ((tilewidth + 1) / 2)];

#ifdef _OPENMP
                #pragma omp for schedule(dynamic) collapse(2)
#endif

                for (int tiletop = 0; tiletop < imheight; tiletop += tileHskip) {
                    for (int tileleft = 0; tileleft < imwidth ; tileleft += tileWskip) {
                        //printf("titop=%d tileft=%d\n",tiletop/tileHskip, tileleft/tileWskip);
                        pos = (tiletop / tileHskip) * numtiles_W + tileleft / tileWskip ;
                        int tileright = min(imwidth, tileleft + tilewidth);
                        int tilebottom = min(imheight, tiletop + tileheight);
                        int width  = tileright - tileleft;
                        int height = tilebottom - tiletop;
                        int width2 = (width + 1) / 2;
                        float realred, realblue;
                        float interm_med = chroma / 10.0;
                        float intermred, intermblue;

                        if (redchro > 0.) {
                            intermred = redchro / 10.0;
                        } else {
                            intermred = redchro / 7.0;     //increase slower than linear for more sensit
                        }

                        if (bluechro > 0.) {
                            intermblue = bluechro / 10.0;
                        } else {
                            intermblue = bluechro / 7.0;     //increase slower than linear for more sensit
                        }

                        realred = interm_med + intermred;

                        if (realred <= 0.f) {
                            realred = 0.001f;
                        }

                        realblue = interm_med + intermblue;

                        if (realblue <= 0.f) {
                            realblue = 0.001f;
                        }

                        const float noisevarab_r = rtengine::SQR(realred);
                        const float noisevarab_b = rtengine::SQR(realblue);

                        //input L channel
                        //wavelet denoised image
                        LabImage * labdn = new LabImage(width, height);


                        //We are going to convert to lab externally, so this is just going to be copying from the input to labdn.

#ifdef _OPENMP
                        #pragma omp parallel for schedule(dynamic,16) num_threads(denoiseNestedLevels) if (denoiseNestedLevels>1)
#endif

                        for (int i = tiletop; i < tilebottom; ++i) {
                            const int i1 = i - tiletop;

                            for (int j = tileleft; j < tileright; ++j) {
                                const int j1 = j - tileleft;

                                labdn->L[i1][j1] = src(i, j*3 + 0);
                                labdn->a[i1][j1] = src(i, j*3 + 1);
                                labdn->b[i1][j1] = src(i, j*3 + 2);

                                if (((i1 | j1) & 1) == 0) {
                                    noisevarchrom[(i1 >> 1) * width2 + (j1 >> 1)] = 1.f;
                                }

                                //end chroma
                            }
                        }

                        //now perform basic wavelet denoise
                        //arguments 4 and 5 of wavelet decomposition are max number of wavelet decomposition levels;
                        //and whether to subsample the image after wavelet filtering.  Subsampling is coded as
                        //binary 1 or 0 for each level, eg subsampling = 0 means no subsampling, 1 means subsample
                        //the first level only, 7 means subsample the first three levels, etc.
                        //actual implementation only works with subsampling set to 1
                        bool execwavelet = true;
                        if (execwavelet) {//gain time if user choose only median  sliders L <=1  slider chrom master < 1
                            int levwav = 5;
                            float maxreal = max(realred, realblue);

                            //increase the level of wavelet if user increase much or very much sliders
                            if (maxreal < 8.f) {
                                levwav = 5;
                            } else if (maxreal < 10.f) {
                                levwav = 6;
                            } else if (maxreal < 15.f) {
                                levwav = 7;
                            } else {
                                levwav = 8;    //maximum ==> I have increase Maxlevel in cplx_wavelet_dec.h from 8 to 9
                            }

                            if (levwav > 8) {
                                levwav = 8;
                            }

                            int minsizetile = min(tilewidth, tileheight);
                            int maxlev2 = 8;

                            if (minsizetile < 256) {
                                maxlev2 = 7;
                            }

                            if (minsizetile < 128) {
                                maxlev2 = 6;
                            }

                            if (minsizetile < 64) {
                                maxlev2 = 5;
                            }

                            levwav = min(maxlev2, levwav);

                            //  if (settings->verbose) printf("levwavelet=%i  noisevarA=%f noisevarB=%f \n",levwav, noisevarab_r, noisevarab_b);
                            const std::unique_ptr<wavelet_decomposition> Ldecomp(new wavelet_decomposition(labdn->L[0], labdn->W, labdn->H, levwav, 1, 1, max(1, denoiseNestedLevels)));

                            if (Ldecomp->memory_allocation_failed()) {
                                memoryAllocationFailed = true;
                            }

                            float madL[8][3];

                            if (!memoryAllocationFailed) {
                                // precalculate madL, because it's used in adecomp and bdecomp
                                int maxlvl = Ldecomp->maxlevel();
#ifdef _OPENMP
                                #pragma omp parallel for schedule(dynamic) collapse(2) num_threads(denoiseNestedLevels) if (denoiseNestedLevels>1)
#endif

                                for (int lvl = 0; lvl < maxlvl; ++lvl) {
                                    for (int dir = 1; dir < 4; ++dir) {
                                        // compute median absolute deviation (MAD) of detail coefficients as robust noise estimator
                                        int Wlvl_L = Ldecomp->level_W(lvl);
                                        int Hlvl_L = Ldecomp->level_H(lvl);

                                        const float* const* WavCoeffs_L = Ldecomp->level_coeffs(lvl);

                                        if (!denoiseMethodRgb) {
                                            madL[lvl][dir - 1] = rtengine::SQR(Mad(WavCoeffs_L[dir], Wlvl_L * Hlvl_L));
                                        } else {
                                            madL[lvl][dir - 1] = rtengine::SQR(MadRgb(WavCoeffs_L[dir], Wlvl_L * Hlvl_L));
                                        }

                                    }
                                }
                            }

                            float chresid = 0.f;
                            float chresidtemp = 0.f;
                            float chmaxresid = 0.f;
                            float chmaxresidtemp = 0.f;

                            std::unique_ptr<wavelet_decomposition> adecomp(new wavelet_decomposition(labdn->a[0], labdn->W, labdn->H, levwav, 1, 1, max(1, denoiseNestedLevels)));

                            if (adecomp->memory_allocation_failed()) {
                                memoryAllocationFailed = true;
                            }

                            if (!memoryAllocationFailed) {
                                if (!WaveletDenoiseAllAB(*Ldecomp, *adecomp, noisevarchrom, madL, 0, noisevarab_r, autoch, denoiseMethodRgb, denoiseNestedLevels)) { //enhance mode
                                    memoryAllocationFailed = true;
                                }
                            }

                            if (!memoryAllocationFailed) {
                                if (kall == 0) {
                                    Noise_residualAB(*adecomp, chresid, chmaxresid, denoiseMethodRgb, denoiseNestedLevels);
                                    chresidtemp = chresid;
                                    chmaxresidtemp = chmaxresid;
                                }

                                adecomp->reconstruct(labdn->a[0]);
                            }

                            adecomp.reset();

                            if (!memoryAllocationFailed) {
                                std::unique_ptr<wavelet_decomposition> bdecomp(new wavelet_decomposition(labdn->b[0], labdn->W, labdn->H, levwav, 1, 1, max(1, denoiseNestedLevels)));

                                if (bdecomp->memory_allocation_failed()) {
                                    memoryAllocationFailed = true;
                                }

                                if (!memoryAllocationFailed) {
                                    if (!WaveletDenoiseAllAB(*Ldecomp, *bdecomp, noisevarchrom, madL, 0, noisevarab_b, autoch, denoiseMethodRgb, denoiseNestedLevels)) { //enhance mode
                                        memoryAllocationFailed = true;
                                    }
                                }

                                if (!memoryAllocationFailed) {
                                    if (kall == 0) {
                                        Noise_residualAB(*bdecomp, chresid, chmaxresid, denoiseMethodRgb, denoiseNestedLevels);
                                        chresid += chresidtemp;
                                        chmaxresid += chmaxresidtemp;
                                        chresid = sqrt(chresid / (6 * (levwav)));
                                    }

                                    bdecomp->reconstruct(labdn->b[0]);
                                }

                                bdecomp.reset();
                            }
                        }

                        if (!memoryAllocationFailed) {


                            // end of tiling calc

                            //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                            // Main detail recovery algorithm: Block loop
                            //DCT block data storage


                            //convert back to RGB and write to destination array
                            //except we're not converting back to RGB, we're leaving it Lab
                            //do copy labdn to dsttmp
#ifdef _OPENMP
                            #pragma omp parallel for schedule(dynamic,16) num_threads(denoiseNestedLevels)
#endif

                            for (int i = tiletop; i < tilebottom; ++i) {
                                int i1 = i - tiletop;
                                for (int j = tileleft; j < tileright; ++j) {
                                    int j1 = j - tileleft;
                                    dsttmp(i, j*3 + 0) = labdn->L[i1][j1];
                                    dsttmp(i, j*3 + 1) = labdn->a[i1][j1];
                                    dsttmp(i, j*3 + 2) = labdn->b[i1][j1];
                                }
                            }

                        }
                        delete labdn;

                    }//end of tile row
                }//end of tile loop

                delete[] noisevarchrom;

            }

#ifdef _OPENMP
            omp_set_nested(oldNested);
#endif

            //copy denoised image to output
            dst.swap(dsttmp);
            dsttmp.set_size(0,0);

        } while (memoryAllocationFailed && numTries < 2);

        if (memoryAllocationFailed) {
            printf("tiled denoise failed due to isufficient memory. Output is not denoised!\n");
            dst.set_size(0,0);
        }

    }

//    if (settings->verbose) {
//        t2e.set();
//        printf("Denoise performed in %d usec:\n", t2e.etime(t1e));
//    }
}//end of main RGB_denoise
