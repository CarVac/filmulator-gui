#include <algorithm>
#include <limits>
#include <numeric>
#include <tuple>
#include "nlmeans.hpp"

//X is in points major, dimensions minor order
//Returns cluster centers (dimensions major), isInSecondCluster, summed square distances, number of points in each cluster.
std::tuple<std::vector<float>,std::vector<bool>,std::array<double,2>,std::array<int,2>> splitCluster(float* __restrict const X, const int numPoints, const int numDimensions){

    //Set initial cluster centers to the points with the min and max norm
    std::vector<float> norms(numPoints,0);
    for (int dIdx = 0; dIdx < numDimensions; dIdx++){
        for (int pIdx = 0; pIdx < numPoints; pIdx++){
            float val = X[pIdx + dIdx*numPoints];
            norms[pIdx] += val*val;
        }
    }

    int minIdx = 0;
    int maxIdx = 0;
    float minVal = std::numeric_limits<float>::max();
    float maxVal = std::numeric_limits<float>::min();
    for (int pIdx = 0; pIdx < numPoints; pIdx++){
        if (norms[pIdx] > maxVal){
            maxVal = norms[pIdx];
            maxIdx = pIdx;
        }
        if (norms[pIdx] < minVal){
            minVal = norms[pIdx];
            minIdx = pIdx;
        }
    }
 
    //clusterCenters is in dimensions major, cluster minor order
    std::vector<float> clusterCenters(2*numDimensions);
    for (int cIdx = 0; cIdx < 2; cIdx++){
        for (int dIdx = 0; dIdx <numDimensions; dIdx++){
            if (cIdx == 0) // use min
                clusterCenters[dIdx + cIdx*numDimensions] = X[minIdx + dIdx*numPoints];
            else
                clusterCenters[dIdx + cIdx*numDimensions] = X[maxIdx + dIdx*numPoints];
        }
    }

    std::vector<bool> isInSecondCluster(numPoints,false);
    std::vector<bool> wasInSecondCluster(numPoints,true);
    
    //points major
    std::vector<float> distanceToClusterCenters(numPoints*2);
    
    //dimensions major
    std::vector<float> dimensionSums(numDimensions*2);

    std::array<int,2> numPointsInClusters{};

    //While there are changes in cluster identities
    for (int iterNum = 0; !equal(isInSecondCluster.begin(),isInSecondCluster.end(),wasInSecondCluster.begin()) & (iterNum < 100); iterNum++){

        //Calculate distances from each point to its cluster center
        std::fill(distanceToClusterCenters.begin(),distanceToClusterCenters.end(),0);
        for(int cIdx = 0; cIdx < 2; cIdx++){
            for(int dIdx = 0; dIdx < numDimensions; dIdx++){
                for (int pIdx = 0; pIdx < numPoints; pIdx++){
                    int clusterIdx = dIdx + cIdx*numDimensions;
                    float difference = X[pIdx + dIdx*numPoints] - clusterCenters[clusterIdx];
                    distanceToClusterCenters[pIdx + cIdx*numPoints] += difference*difference;
                }
            }
        }

        //Update which cluster each point belongs to
        std::swap(isInSecondCluster,wasInSecondCluster);
        for(int pIdx = 0; pIdx < numPoints; pIdx++){
            isInSecondCluster[pIdx] = (distanceToClusterCenters[pIdx] > distanceToClusterCenters[pIdx + numPoints]);
        }

        //Move cluster centers to the center of points belonging to that cluster
        std::fill(dimensionSums.begin(),dimensionSums.end(),0);
        for (int pIdx = 0; pIdx < numPoints; pIdx++){
            for (int dIdx = 0; dIdx < numDimensions; dIdx++){
                int dimsumIdx = dIdx + isInSecondCluster[pIdx]*numDimensions;
                dimensionSums[dimsumIdx] += X[pIdx + dIdx*numPoints];
            }
        }

        int numPointsInSecondCluster = std::accumulate(isInSecondCluster.begin(),isInSecondCluster.end(),0);
        numPointsInClusters = {numPoints - numPointsInSecondCluster, numPointsInSecondCluster};
        for (int cIdx = 0; cIdx < 2; cIdx++){
            for (int dIdx = 0; dIdx < numDimensions; dIdx++){
                int idx = dIdx + cIdx*numDimensions;
                clusterCenters[idx] = dimensionSums[idx] / float(numPointsInClusters[cIdx]);
            }
        }

    }

    std::array<double,2> loss{};
    for (int pIdx = 0; pIdx < numPoints; pIdx++){
        loss[isInSecondCluster[pIdx]] += distanceToClusterCenters[pIdx + isInSecondCluster[pIdx]*numPoints];
    }

    return {clusterCenters,isInSecondCluster,loss,numPointsInClusters};

}
