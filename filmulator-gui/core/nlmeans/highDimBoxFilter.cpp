#include <cstddef>
#include <algorithm>
#include <cmath>
#include "nlmeans.hpp"

//A is the input image- called f in the paper, W is called c_k, numClusters is K
//S is the radius of the neighborhood over which we are averaging (neighborhood size (2S+1)^2)
//We're going to column major order right now to match MATLAB, TODO: switch to row major
//TODO: fixup int vs ptrdiff_t indexing.
void highDimBoxFilter(float* __restrict const A, float* __restrict const W, float* __restrict const C1chanT, ptrdiff_t const numClusters,
                      float* __restrict output){


    //Calculate the weighting (Wt) in this block across all clusters to preserve locallity. TODO: test moving this into the per-cluster for loop
    std::vector<float> Wt(blockSize*blockSize*numClusters, 0.0f);
    for (ptrdiff_t xIdx = 0; xIdx < blockSize; xIdx++){
        for (ptrdiff_t destClusterIdx = 0; destClusterIdx < numClusters; destClusterIdx++){
            for(ptrdiff_t sourceClusterIdx = 0; sourceClusterIdx < numClusters; sourceClusterIdx++){
                for(ptrdiff_t yIdx = 0; yIdx < blockSize; yIdx++){
                    ptrdiff_t Wt_idx = yIdx + xIdx*blockSize + destClusterIdx*blockSize*blockSize;
                    ptrdiff_t C1chanT_idx = sourceClusterIdx + destClusterIdx*numClusters;
                    ptrdiff_t W_idx = (yIdx+S) + (xIdx+S)*expandedBlockSize + sourceClusterIdx*expandedBlockSize*expandedBlockSize;

                    Wt[Wt_idx] += W[W_idx]*C1chanT[C1chanT_idx];
                }
            }
        }
            
    }

    //Calculate sum (across clusters) of Wt*boxFilter(W) and Wt*boxFilter(W*A), yield Wb and B
    std::vector<float> W_summedAreaTable(blockSize*expandedBlockSize, 0.0f);
    std::vector<float> WA_summedAreaTable(blockSize*expandedBlockSize*3, 0.0f);
    std::vector<float> Wb(blockSize*blockSize, 0.0f);
    std::vector<float> B(blockSize*blockSize*3, 0.0f);
    for (ptrdiff_t clusterIdx = 0; clusterIdx < numClusters; clusterIdx++){

        //We need to blur in both the x and y directions, but only the x direction is vectorizeable
        //Therefore, we first do a blur in the x direction and store it in a temporary array.
        //Then we calculate the cumulative sum (summed area table) in the y direction using scalars
        //Lastly, when we read out the values, we will do the appropriate subtractions to calculate the blur in the y direction

        //First create temporary summed area table for a blur in the x direction
        std::fill(W_summedAreaTable.begin(), W_summedAreaTable.end(), 0);//TODO: I probably only need to zero out the first column
        std::fill(WA_summedAreaTable.begin(), WA_summedAreaTable.end(), 0); //TODO: might want to set to epsilon instead of 0 

            
        //Preamble: generate the first column of the summed area table in the x direction.
        //We are reading from an expandedBlockSize x expandedBlockSize image tile and storing a blurred version in
        //an expandedBlockSize x blockSize summed area table (which, for this dimension is simply a temporary array)
        //TODO: When I make this code row major, change comment to say first row.
        for (ptrdiff_t xReadIdx = 0; xReadIdx < (1+2*S); xReadIdx++){
            for (ptrdiff_t c = 0; c < 3; c++){
                for (ptrdiff_t y = 0; y < expandedBlockSize; y++){
                    ptrdiff_t xWriteIdx = 0;

                    ptrdiff_t W_sat_idx = y + xWriteIdx*expandedBlockSize;
                    ptrdiff_t W_idx = y + xReadIdx*expandedBlockSize + clusterIdx*expandedBlockSize*expandedBlockSize;
                    if (c == 0){ //W is a single channel, so we only have to compute it once
                        W_summedAreaTable[W_sat_idx] += W[W_idx];
                    }

                    ptrdiff_t A_idx = y + xReadIdx*expandedBlockSize + c*expandedBlockSize*expandedBlockSize;
                    ptrdiff_t WA_sat_idx = W_sat_idx + c*blockSize*expandedBlockSize;
                    WA_summedAreaTable[WA_sat_idx] += W[W_idx]*A[A_idx];
                }
            }
        }

        //Main: generate the rest of the summed area table in the x direction.
        //TODO: When I make this code row major, change comment to say first row.
        for (ptrdiff_t xWriteIdx = 1; xWriteIdx < blockSize; xWriteIdx++){
            for (ptrdiff_t c = 0; c < 3; c++){
                for (ptrdiff_t y = 0; y < expandedBlockSize; y++){

                    ptrdiff_t xSATReadCenter = xWriteIdx - 1; //read from previous column
                    ptrdiff_t xImageReadAdd = xWriteIdx + 2*S;
                    ptrdiff_t xImageReadSub = xWriteIdx - 1;

                    ptrdiff_t W_sat_idx_write = y + xWriteIdx*expandedBlockSize;
                    ptrdiff_t W_sat_idx_read = y + xSATReadCenter*expandedBlockSize; 
                    ptrdiff_t W_idx_add = y + xImageReadAdd*expandedBlockSize + clusterIdx*expandedBlockSize*expandedBlockSize;
                    ptrdiff_t W_idx_sub = y + xImageReadSub*expandedBlockSize + clusterIdx*expandedBlockSize*expandedBlockSize;

                    ptrdiff_t WA_sat_idx_write = W_sat_idx_write + c*blockSize*expandedBlockSize; //WA has same index as W, but shifted by the color
                    ptrdiff_t WA_sat_idx_read = W_sat_idx_read + c*blockSize*expandedBlockSize;
                    ptrdiff_t A_idx_add = y + xImageReadAdd*expandedBlockSize + c*expandedBlockSize*expandedBlockSize;
                    ptrdiff_t A_idx_sub = y + xImageReadSub*expandedBlockSize + c*expandedBlockSize*expandedBlockSize;

                    if (c == 0){
                        W_summedAreaTable[W_sat_idx_write] = W_summedAreaTable[W_sat_idx_read] + W[W_idx_add] - W[W_idx_sub];
                    }
                    WA_summedAreaTable[WA_sat_idx_write] = WA_summedAreaTable[WA_sat_idx_read] + W[W_idx_add]*A[A_idx_add] - W[W_idx_sub]*A[A_idx_sub];
                }     
            }       
        }
            

        //Cumulative sum in the vertical direction. TODO: change this comment when flipping order
        for (ptrdiff_t c = 0; c < 3; c++){
            for (ptrdiff_t yWriteIdx = 1; yWriteIdx < expandedBlockSize; yWriteIdx++){ // the cumsum of the first row is equal to the first row, so skip it. TODO: change this comment when filpping order 
                for (ptrdiff_t xIdx = 0; xIdx < blockSize; xIdx++){            
                        
                    ptrdiff_t W_sat_idx = yWriteIdx + xIdx*expandedBlockSize;
                    if (c == 0){ //W is a single channel, so we only have to compute it once
                        W_summedAreaTable[W_sat_idx] += W_summedAreaTable[W_sat_idx - 1];
                    }

                    ptrdiff_t WA_sat_idx = W_sat_idx + c*blockSize*expandedBlockSize;
                    WA_summedAreaTable[WA_sat_idx] += WA_summedAreaTable[WA_sat_idx - 1];
                }
            }
        }


        //Weight by Wt and write out to Wb and B
        for (ptrdiff_t c = 0; c < 3; c++){
            for (ptrdiff_t xIdx = 0; xIdx < blockSize; xIdx++){
                //We're pulling the first yIdx iteration out of the loop to help the compiler (needed in GCC 10.2)
                //The first iteration is different because we don't have anything to subtract
                ptrdiff_t yIdx = 0;
                ptrdiff_t Wt_idx = yIdx + xIdx*blockSize + clusterIdx*blockSize*blockSize;
                ptrdiff_t Wb_idx = yIdx + xIdx*blockSize;
                ptrdiff_t B_idx = yIdx + xIdx*blockSize + c*blockSize*blockSize;

                ptrdiff_t W_sat_idx = yIdx+S + xIdx*expandedBlockSize;
                ptrdiff_t WA_sat_idx = yIdx+S + xIdx*expandedBlockSize + c*blockSize*expandedBlockSize;
                if (yIdx == 0){
                    if (c == 0){
                        Wb[Wb_idx] += Wt[Wt_idx]*W_summedAreaTable[W_sat_idx+S];
                    }
                    B[B_idx] += Wt[Wt_idx]*WA_summedAreaTable[WA_sat_idx+S];
                }

                for (ptrdiff_t yIdx = 1; yIdx < blockSize; yIdx++){
                    ptrdiff_t Wt_idx = yIdx + xIdx*blockSize + clusterIdx*blockSize*blockSize;
                    ptrdiff_t Wb_idx = yIdx + xIdx*blockSize;
                    ptrdiff_t B_idx = yIdx + xIdx*blockSize + c*blockSize*blockSize;

                    ptrdiff_t W_sat_idx = yIdx+S + xIdx*expandedBlockSize;
                    ptrdiff_t WA_sat_idx = yIdx+S + xIdx*expandedBlockSize + c*blockSize*expandedBlockSize;

                    if (c == 0){
                        Wb[Wb_idx] += Wt[Wt_idx]*(W_summedAreaTable[W_sat_idx+S] - W_summedAreaTable[W_sat_idx-S-1]); //We read out a blurred value using the summed area table. blur = cumsum(x+r) - cumsum(x-r)
                    }
                    B[B_idx] += Wt[Wt_idx]*(WA_summedAreaTable[WA_sat_idx+S] - WA_summedAreaTable[WA_sat_idx-S-1]);                            
                }
            }
        }
    }


    //Divide B by Wb
    for (ptrdiff_t chanIdx = 0; chanIdx < 3; chanIdx++){
        for (ptrdiff_t xIdx = 0; xIdx < blockSize; xIdx++){
            for(ptrdiff_t yIdx = 0; yIdx < blockSize; yIdx++){
                ptrdiff_t output_idx = yIdx + xIdx*blockSize + chanIdx*blockSize*blockSize;
                ptrdiff_t Wb_idx = yIdx + xIdx*blockSize;
                ptrdiff_t B_idx = Wb_idx + chanIdx*blockSize*blockSize;
                ptrdiff_t A_idx = yIdx + S + (xIdx+S)*expandedBlockSize + chanIdx*expandedBlockSize*expandedBlockSize;

                // Small values of Wb are unreliable. If ln(abs(Wb)) < -3, then disregard just fall back to the original value.
                const float confidence = std::clamp(std::log(std::abs(Wb[Wb_idx]))+3 , 0.0f, 1.0f);
                const float eps = std::numeric_limits<float>::epsilon();
                output[output_idx] =  confidence*(B[B_idx] / (Wb[Wb_idx] + eps)) + (1-confidence)*A[A_idx];
            }
        }
    }

}
