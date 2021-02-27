#include <algorithm>
#include <limits>
#include <numeric>
#include <tuple>
#include <optional>
#include "nlmeans.hpp"

struct clusterInfo{
    std::vector<float> center;
    std::vector<int> members;
    double summedSquareDistances;
};

//X is in points major, dimensions minor order
//Returns center locations (clusters major). Will always return at least two clusters
std::vector<float> bisecting_kmeans(float* __restrict const X, const int numPoints, const int maxNumClusters, const std::optional<float> threshold){

    const int numDimensions = patchSize*numChannels;
    std::vector<clusterInfo> currentClusters;
    
    //Initialize first cluster to include all points. Don't worry about its center and SSD
    clusterInfo allPointsClustInfo{.members = std::vector<int>(numPoints), .summedSquareDistances = std::numeric_limits<float>::max()};
    std::iota(allPointsClustInfo.members.begin(),allPointsClustInfo.members.end(),0);
    currentClusters.push_back(allPointsClustInfo);

    double totalSSD = std::numeric_limits<double>::max();

    while ((currentClusters.size() < maxNumClusters) & (threshold.has_value() && totalSSD > ((*threshold)*numPoints))){

        // Split the cluster with the highest summed squared distance
        int nextClusterToSplitIdx = -1;
        float highestSSD = 0;
        for (int clustIdx = 0; clustIdx < currentClusters.size(); clustIdx++){
            auto& thisCluster = currentClusters[clustIdx];
            if ((thisCluster.summedSquareDistances > highestSSD) & (thisCluster.members.size() > 2)){ //Don't split a cluster of only 2 points- it just has two outliers
                nextClusterToSplitIdx = clustIdx;
                highestSSD = thisCluster.summedSquareDistances;
            }
        }

        if (nextClusterToSplitIdx == -1) { //all clusters have only two points, so we are done.
            break;
        }

        auto clusterToSplit = currentClusters[nextClusterToSplitIdx];
        int numPointsToSplit = clusterToSplit.members.size();
        std::vector<float> clusterToSplitX(numPointsToSplit*numDimensions);
        for(int dimIdx = 0; dimIdx < numDimensions; dimIdx++){
            for(int pointIdx = 0; pointIdx < numPointsToSplit; pointIdx++){
                int sourcePointIdx = clusterToSplit.members[pointIdx];
                clusterToSplitX[pointIdx + dimIdx*numPointsToSplit] = X[sourcePointIdx + dimIdx*numPoints];
            }
        }

        auto [twoCenters, isInSecondCluster, SSDs, numMembers] = splitCluster(clusterToSplitX.data(), numPointsToSplit, numDimensions);
        
        //If either cluster only has one member, discard that cluster
        int discardCluster = std::distance(numMembers.begin(), std::find(numMembers.begin(), numMembers.end(), 1)); //takes on values 0,1 or 2 for none

        for (int clustIdx = 0, numInsertedClusters = 0; clustIdx < 2; clustIdx++){
            if (clustIdx == discardCluster) {
                continue;
            }
            auto centerRangeToCopyStart = twoCenters.begin() + clustIdx*numDimensions;
            auto centerRangeToCopyEnd = centerRangeToCopyStart + numDimensions;

            std::vector<int> clusterMembers;
            for (int memberIdx = 0; memberIdx < numPointsToSplit; memberIdx++){
                if (isInSecondCluster[memberIdx] ^ (clustIdx == 0)){
                    clusterMembers.push_back(clusterToSplit.members[memberIdx]);
                }
            }
            clusterInfo clustInfo{.center = std::vector<float>(centerRangeToCopyStart,centerRangeToCopyEnd),
                                  .members = clusterMembers,
                                  .summedSquareDistances = SSDs[clustIdx] };
            
            if (numInsertedClusters == 0) {
                currentClusters[nextClusterToSplitIdx] = clustInfo;
            }
            else {
                currentClusters.push_back(clustInfo);
            }
            numInsertedClusters++;
        }

        totalSSD = 0;
        for (int clustIdx = 0; clustIdx < currentClusters.size(); clustIdx++){
            totalSSD += currentClusters[clustIdx].summedSquareDistances;
        }
    }

    int numClusters = currentClusters.size();
    std::vector<float> clusterCenters(numClusters*numDimensions);
    for (int dimIdx = 0; dimIdx < numDimensions; dimIdx++){
        for (int clustIdx = 0; clustIdx < numClusters; clustIdx++){
            clusterCenters[clustIdx + dimIdx*numClusters] = currentClusters[clustIdx].center[dimIdx];
        }
    }

    return clusterCenters;

}
