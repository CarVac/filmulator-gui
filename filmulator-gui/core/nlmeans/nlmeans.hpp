#ifndef NLMEANS_H
#define NLMEANS_H

#include <vector>
#include <array>
#include <optional>

#include "imagePipeline.h"

constexpr ptrdiff_t blockSize = 64;
constexpr ptrdiff_t S = 8;
constexpr ptrdiff_t expandedBlockSize = blockSize + 2*S;
constexpr int radius = 1;
constexpr int patchSize = (2 * radius + 1) * (2 * radius + 1);
constexpr int numChannels = 3;
constexpr int numPoints = 500;

std::vector<float> bisecting_kmeans(float* const X, const int maxNumClusters, const float threshold);

std::vector<float> calcC1ChanT(std::vector<float> centers, int numClusters, const float h);

void calcW(float* const Aguide_ptr, float* const centers_ptr,
           ptrdiff_t rangeDims, ptrdiff_t numClusters, ptrdiff_t expandedBlockSize,
           float const h, float* W_ptr);

void expandDims(float* const I, const int sizeX, const int sizeY, float* output);

void highDimBoxFilter(float* const A, float* const W, float* const C1chanT, ptrdiff_t const numClusters,
                      float* output);

bool kMeansNLMApprox(float* const I, const int maxNumClusters, const float clusterThreshold, const float h, const int sizeX, const int sizeY, float* output, ParameterManager* paramManager);

std::tuple<std::vector<float>,std::vector<bool>,std::array<double,2>,std::array<int,2>> splitCluster(float* const X, const int numPoints);

#endif // NLMEANS_H
