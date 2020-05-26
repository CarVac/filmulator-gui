#include <cstddef>
#include <algorithm>
#include <cmath>
#include "nlmeans.hpp"

//Aguide is called p in the paper, W is called c_k, centers is called mu_k
//requires W be set to 0
void calcW(float* __restrict const Aguide_ptr, float* __restrict const centers_ptr,
           ptrdiff_t rangeDims, ptrdiff_t numClusters, float const h,
           float* __restrict W_ptr){

    constexpr ptrdiff_t expandedBlockSize = 144;

    for (ptrdiff_t x = 0; x < expandedBlockSize; x++){
        for (ptrdiff_t c = 0; c < numClusters; c++) {
            for (ptrdiff_t pIdx = 0; pIdx < rangeDims; pIdx++) {
                for (ptrdiff_t y = 0; y < expandedBlockSize; y++){
                        
                    ptrdiff_t A_idx = y + x*expandedBlockSize + pIdx*expandedBlockSize*expandedBlockSize;
                    ptrdiff_t centers_idx = c + pIdx*numClusters;
                    ptrdiff_t W_idx = y + x*expandedBlockSize + c*expandedBlockSize*expandedBlockSize;

                    float centerDist = Aguide_ptr[A_idx] - centers_ptr[centers_idx];
                    W_ptr[W_idx] += centerDist*centerDist;
                }
            }

            for (ptrdiff_t y = 0; y < expandedBlockSize; y++) {
                ptrdiff_t W_idx = y + x*expandedBlockSize + c*expandedBlockSize*expandedBlockSize;
                W_ptr[W_idx] = std::exp(-W_ptr[W_idx] / (2 * h * h));
            }
        }
    }
}
