#include <cstddef>
#include <algorithm>
#include <cmath>
#include "nlmeans.hpp"

//Aguide is called p in the paper, W is called c_k, centers is called mu_k 
void expandDims(float* __restrict const I, float* __restrict outputPatches, float* __restrict patchMeans) {
    constexpr int patchWidth = (2 * radius + 1);


    std::array<float, patchWidth * patchWidth> weightMat;
    float weightSum = 0; 
    for (int xPatch = -radius; xPatch < radius + 1; xPatch++) {
        for (int yPatch = -radius; yPatch < radius + 1; yPatch++) {
            int xPatchIdx = xPatch + radius;
            int yPatchIdx = yPatch + radius;
            float dist = float(xPatch * xPatch) + float(yPatch * yPatch);
            float weight = std::exp(-dist / radius);
            weightMat[yPatchIdx + xPatchIdx * patchWidth] = weight;
            weightSum += weight;
        }

    }


    for (int xPatch = -radius; xPatch < radius + 1; xPatch++) {
        for (int yPatch = -radius; yPatch < radius + 1; yPatch++) {
            for (int c = 0; c < numChannels; c++) {
                for (int x = 0; x < expandedBlockSize; x++) {
                    for (int y = 0; y < expandedBlockSize; y++) {


                        int xPatchIdx = xPatch + radius;
                        int yPatchIdx = yPatch + radius;

                        int yRead = y + yPatch;
                        int xRead = x + xPatch;


                        float readVal = 0;
                        if (!((yRead < 0) | (yRead >= expandedBlockSize) | (xRead < 0) | (xRead >= expandedBlockSize))) {
                            readVal = I[yRead + xRead * expandedBlockSize + c * expandedBlockSize * expandedBlockSize];
                        }

                        float outVal = readVal * weightMat[yPatchIdx + xPatchIdx * patchWidth];

                        int outputChannel = c + yPatchIdx * numChannels + xPatchIdx * numChannels * patchWidth;
                        outputPatches[y + x * expandedBlockSize + outputChannel * expandedBlockSize * expandedBlockSize] = outVal;

                        patchMeans[y + x * expandedBlockSize + c * expandedBlockSize * expandedBlockSize] += outVal / weightSum;
                    }
                }
            }
        }
    }
}
