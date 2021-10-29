#include <random>
#include "nlmeans.hpp"


bool kMeansNLMApprox(float* __restrict const I, const int maxNumClusters, const float clusterThreshold, const float h, const int sizeX, const int sizeY, float* __restrict output, ParameterManager* paramManager) {

	ptrdiff_t numBlocksX = std::ceil(float(sizeX) / float(blockSize));
	ptrdiff_t numBlocksY = std::ceil((float(sizeY) / float(blockSize)));
	ptrdiff_t numBlocks = numBlocksX * numBlocksY;
	bool canceled = false;
	#pragma omp parallel for 
	for (ptrdiff_t blockIdx = 0; blockIdx < numBlocks; blockIdx++) {

		AbortStatus abort;
        abort = paramManager->claimNlmeansNRAbort();
		if(abort == AbortStatus::restart)
		{
			canceled = true;
			#pragma omp cancel for
		}

		//We're using column major order across the blocks
		ptrdiff_t xBlockStart = blockSize * (blockIdx / numBlocksY);
		ptrdiff_t yBlockStart = blockSize * (blockIdx % numBlocksY);

		std::array<float, expandedBlockSize * expandedBlockSize * numChannels> IBlockCopy;
		for (ptrdiff_t c = 0; c < numChannels; c++) {
			for (ptrdiff_t xWriteIdx = 0; xWriteIdx < expandedBlockSize; xWriteIdx++) {
				for (ptrdiff_t yWriteIdx = 0; yWriteIdx < expandedBlockSize; yWriteIdx++) {
					ptrdiff_t xReadIdx = xWriteIdx + xBlockStart - S;
					ptrdiff_t yReadIdx = yWriteIdx + yBlockStart - S;

					float value = 0;
					if ((yReadIdx >= 0) & (yReadIdx < sizeY) & (xReadIdx >= 0) & (xReadIdx < sizeX)) {
                        value = I[yReadIdx*3 + 3*xReadIdx*sizeY + c];
					}

                    IBlockCopy[yWriteIdx + xWriteIdx*expandedBlockSize + c*expandedBlockSize*expandedBlockSize] = value;
				}
			}
		}


		std::array<float, expandedBlockSize * expandedBlockSize * numChannels * patchSize> expandedDimensions;
		expandDims(IBlockCopy.data(), expandedBlockSize, expandedBlockSize, expandedDimensions.data());


		//Todo: only sample within the image
		constexpr ptrdiff_t numPatchesToSample = numPoints;
		std::minstd_rand0 generator(0);
		std::uniform_int_distribution<> distribution(0, expandedBlockSize * expandedBlockSize - 1);
		std::array<ptrdiff_t, numPatchesToSample> sampledLocs;
		for (auto& sample : sampledLocs) {
			sample = distribution(generator);
		}

		std::array<float, numPatchesToSample * patchSize * numChannels> sampledPatches;
		for (ptrdiff_t dIdx = 0; dIdx < (patchSize * numChannels); dIdx++) {
			for (ptrdiff_t pIdx = 0; pIdx < numPatchesToSample; pIdx++) {
				sampledPatches[pIdx + dIdx*numPatchesToSample] = expandedDimensions[sampledLocs[pIdx] + dIdx*expandedBlockSize*expandedBlockSize];
			}
		}

		std::vector<float> clusterCenters = bisecting_kmeans(sampledPatches.data(), maxNumClusters, clusterThreshold);
		int numClusters = clusterCenters.size() / (patchSize * numChannels);

		//Todo: set W to 0 outside the image
		std::vector<float> W(expandedBlockSize * expandedBlockSize * numClusters, 0.0f);
        calcW(expandedDimensions.data(), clusterCenters.data(), patchSize * 3, numClusters, expandedBlockSize, h, W.data());

		std::vector<float> C1ChanT = calcC1ChanT(clusterCenters, numClusters, h);

		std::array<float, blockSize*blockSize*numChannels> tileOutput;
        highDimBoxFilter(IBlockCopy.data(), W.data(), C1ChanT.data(), numClusters, tileOutput.data());

		for (ptrdiff_t c = 0; c < numChannels; c++) {
			for (ptrdiff_t xReadIdx = 0; xReadIdx < blockSize; xReadIdx++) {
				for (ptrdiff_t yReadIdx = 0; yReadIdx < blockSize; yReadIdx++) {
					ptrdiff_t xWriteIdx = xReadIdx + xBlockStart;
					ptrdiff_t yWriteIdx = yReadIdx + yBlockStart;

					if ((yWriteIdx >= 0) & (yWriteIdx < sizeY) & (xWriteIdx >= 0) & (xWriteIdx < sizeX)) {
                        output[yWriteIdx*3 + 3*xWriteIdx*sizeY + c] = tileOutput[yReadIdx + xReadIdx*blockSize + c*blockSize*blockSize];
					}
				}
			}
		}
	}
	return canceled;
}
