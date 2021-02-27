#ifndef NLMEANS_H
#define NLMEANS_H

#include <vector>
#include <array>

std::vector<float> bisecting_kmeans(float* const X, const int numPoints, const int numDimensions, const int k, const std::optional<float> threshold = std::nullopt);

std::vector<float> calcC1ChanT(std::vector<float> centers, int numClusters, const float h);

void calcW(float* const Aguide_ptr, float* const centers_ptr,
           ptrdiff_t rangeDims, ptrdiff_t numClusters, ptrdiff_t expandedBlockSize,
           float const h, float* W_ptr);

void expandDims(float* const I, int radius, int sizeX, int sizeY, float* output);

void highDimBoxFilter(float* const A, float* const W, float* const C1chanT, ptrdiff_t const numClusters,
                      ptrdiff_t blockSize, ptrdiff_t S, ptrdiff_t expandedBlockSize,
                      float* output);

void kMeansNLMApprox(float* const I, const int maxClusters, const float clusterThreshold, const float h, const int sizeX, const int sizeY, float* output);

std::tuple<std::vector<float>,std::vector<bool>,std::array<double,2>,std::array<int,2>> splitCluster(float* const X, const int numPoints, const int numDimensions);

#endif // NLMEANS_H
