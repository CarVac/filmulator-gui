#include <cstddef>
#include <algorithm>
#include <cmath>
#include "nlmeans.hpp"

//Aguide is called p in the paper, W is called c_k, centers is called mu_k 
void expandDims(float* const I, int radius, int sizeX, int sizeY, float* output) {
    constexpr int numChannels = 3;
    int patchWidth = (2 * radius + 1);


    std::vector<float> weightMat(patchWidth * patchWidth);
    for (int xPatch = -radius; xPatch < radius + 1; xPatch++) {
        for (int yPatch = -radius; yPatch < radius + 1; yPatch++) {
            int xPatchIdx = xPatch + radius;
            int yPatchIdx = yPatch + radius;
            float dist = float(xPatch * xPatch) + float(yPatch * yPatch);
            weightMat[yPatchIdx + xPatchIdx * patchWidth] = std::exp(-dist / radius);
        }

    }


    for (int xPatch = -radius; xPatch < radius + 1; xPatch++) {
        for (int yPatch = -radius; yPatch < radius + 1; yPatch++) {
            for (int c = 0; c < numChannels; c++) {
                for (int x = 0; x < sizeX; x++) {
                    for (int y = 0; y < sizeY; y++) {


                        int xPatchIdx = xPatch + radius;
                        int yPatchIdx = yPatch + radius;

                        int yRead = y + yPatch;
                        int xRead = x + xPatch;


                        float readVal = 0;
                        if (!((yRead < 0) | (yRead >= sizeY) | (xRead < 0) | (xRead >= sizeX))) {
                            readVal = I[yRead + xRead * sizeY + c * sizeY * sizeX];
                        }

                        float outVal = readVal * weightMat[yPatchIdx + xPatchIdx * patchWidth];

                        int outputChannel = c + yPatchIdx * numChannels + xPatchIdx * numChannels * patchWidth;
                        output[y + x * sizeY + outputChannel * sizeY * sizeX] = outVal;
                    }
                }
            }
        }
    }
}
