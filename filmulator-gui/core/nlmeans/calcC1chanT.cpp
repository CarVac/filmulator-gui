#include <iostream>
#include <cmath>
#include "nlmeans.hpp"

template <class MatT>
Eigen::Matrix<typename MatT::Scalar, MatT::ColsAtCompileTime, MatT::RowsAtCompileTime>
pseudoinverse(const MatT& mat, typename MatT::Scalar tolerance) // choose appropriately
{
    typedef typename MatT::Scalar Scalar;
    auto svd = mat.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV);
    const auto& singularValues = svd.singularValues();
    Eigen::Matrix<Scalar, MatT::ColsAtCompileTime, MatT::RowsAtCompileTime> singularValuesInv(mat.cols(), mat.rows());
    singularValuesInv.setZero();
    for (unsigned int i = 0; i < singularValues.size(); ++i) {
        if (singularValues(i) > tolerance)
        {
            singularValuesInv(i, i) = Scalar{ 1 } / singularValues(i);
        }
        else
        {
            singularValuesInv(i, i) = Scalar{ 0 };
        }
    }
    return svd.matrixV() * singularValuesInv * svd.matrixU().adjoint();
}

//centers is in cluster major order
std::vector<float> calcC1ChanT(std::vector<float> centers, int numClusters, const float h) {

    int numDimensions = centers.size() / numClusters;
    Eigen::MatrixXf C1(numClusters, numClusters);
    for (int row = 0; row < numClusters; row++) {
        for (int col = 0; col < numClusters; col++) {
            float squaredDistance = 0;
            for (int dimIdx = 0; dimIdx < numDimensions; dimIdx++) {
                float dist = centers[row + dimIdx * numClusters] - centers[col + dimIdx * numClusters];
                squaredDistance += dist * dist;
            }
            C1(col, row) = std::exp(-squaredDistance / (2 * h * h)); //We're assigning to (col,row) in order to do the transpose
        }
    }
    auto C1Chan = pseudoinverse<Eigen::MatrixXf>(C1);
    float* matPtr = C1Chan.data();
    return std::vector<float>(matPtr, matPtr + C1Chan.size());
}
