#include "filmSim.hpp"
#include <cmath>
#include <math.h>

template <typename T>
void downscaleDivisible1D(matrix<T> input, matrix<T> &output, int scaleFactor, bool interleaved);
template <typename T>
void downscaleBilinear1D(matrix<T> input, matrix<T> &output, int start, int end, double scaleFactor, bool interleaved);



void downscale_and_crop(matrix<float> input, matrix<float> &output, int inputStartX, int inputStartY, int inputEndX, int inputEndY, int outputXSize, int outputYSize)
{
    int inputXSize = inputEndX - inputStartX + 1;
    int inputYSize = inputEndY - inputStartY + 1;
    //std::cout << "downscale before computation" << std::endl;

    double overallScaleFactor = max(double(inputXSize)/double(outputXSize),double(inputYSize)/double(outputYSize));
    int integerScaleFactor = floor(overallScaleFactor);
    double bilinearScaleFactor = overallScaleFactor/double(integerScaleFactor);

    //std::cout << "downscale before bilinear x" << std::endl;
    //Downscale in one direction
    matrix<float> bilinearX;
    matrix<float> bothX;
    downscaleBilinear1D(input,bilinearX,inputStartX,inputEndX,bilinearScaleFactor,true);
    //std::cout << "downscale before divisible x" << std::endl;
    downscaleDivisible1D(bilinearX,bothX,integerScaleFactor,true);
    matrix<float> bothXTransposed;
    //std::cout << "downscale before transpose" << std::endl;
    //std::cout << bothX.nc() << std::endl;
    //std::cout << bothX.nr() << std::endl;
    bothXTransposed.set_size(bothX.nc(),bothX.nr());
    //std::cout << "downscale before transpose" << std::endl;
    bothX.transpose_to(bothXTransposed);
    matrix<float> bothXTransposedBilinearY;
    matrix<float> bothXTransposedBothY;
    //std::cout << "downscale before bilinear y" << std::endl;
    downscaleBilinear1D(bothXTransposed,bothXTransposedBilinearY,inputStartY,inputEndY,bilinearScaleFactor,false);
    //std::cout << "downscale before divisible y" << std::endl;
    downscaleDivisible1D(bothXTransposedBilinearY,bothXTransposedBothY,integerScaleFactor,false);

    //Then transpose to the output.
    //std::cout << "downscale before transpose 2" << std::endl;
    output.set_size(bothXTransposedBothY.nc(),bothXTransposedBothY.nr());
    bothXTransposedBothY.transpose_to(output);
    //std::cout << "downscale done" << std::endl;
    return;
}

template <typename T>
void downscaleDivisible1D(matrix<T> input, matrix<T> &output, int scaleFactor, bool interleaved)
{
    int inputNumRows = input.nr();
    int inputNumCols = input.nc();
    int outputNumRows = inputNumRows;
    //We must use the ceiling here in order to not undersize the output matrix.
    int outputNumCols;
    if (interleaved)
    {
        outputNumCols = 3*ceil(inputNumCols/(3*double(scaleFactor)));
    } else {
        outputNumCols = ceil(inputNumCols/double(scaleFactor));
    }
    output.set_size(outputNumRows,outputNumCols);

    if (interleaved)
    {
        #pragma omp parallel for shared(input, output)
        for(int i = 0; i < outputNumRows; i++)
            for(int j = 0; j < outputNumCols; j = j+3)
            {
                double sumR = 0;
                double sumG = 0;
                double sumB = 0;
                for (int k = j*scaleFactor; k < j*scaleFactor; k = k + 3 )
                {
                    sumR += input(i,k);
                    sumG += input(i,k+1);
                    sumB += input(i,k+2);
                }
                output(i,j)   = sumR/scaleFactor;
                output(i,j+1) = sumG/scaleFactor;
                output(i,j+2) = sumB/scaleFactor;
            }
    }
    else
    {
        #pragma omp parallel for shared(input, output)
        for(int i = 0; i < outputNumRows; i++)
            for(int j = 0; j < outputNumCols; j++)
            {
                double sum = 0;
                for(int k = j*scaleFactor; k < (j+1)*scaleFactor; k++)
                {
                    sum += input(i,k);
                }
                output(i,j) = sum/scaleFactor;
            }
    }
    return;
}

template <typename T>
void downscaleBilinear1D(matrix<T> input, matrix<T> &output, int start, int end, double scaleFactor, bool interleaved)
{
    int inputNumRows = input.nr();
    int inputNumCols = end - start + 1;

    int outputNumRows = inputNumRows;
    int outputNumCols = round(double(inputNumCols)/scaleFactor);

    if(interleaved)
        output.set_size(outputNumRows,outputNumCols*3);
    else
        output.set_size(outputNumRows,outputNumCols);

    #pragma omp parallel for shared(input, output)
    for(int i = 0; i < outputNumRows; i++)
    {
        for(int j = 0; j < outputNumCols-1; j++)
        {
            double inputPoint = (double(j) + 0.5)*scaleFactor -0.5 + double(start);
            int inputStart = floor(inputPoint);
            int inputEnd = ceil(inputPoint);
            double notUsed;
            double endWeight = modf(inputPoint, &notUsed);
            double startWeight = 1 - endWeight;
            if(interleaved)
                for(int c = 0; c < 3; c++)
                {
                    output(i,3*j + c) = startWeight*double(input(i,3*inputStart + c)) + endWeight*double(input(i,3*inputEnd + c));
                }
            else
                output(i,j) = startWeight*double(input(i,inputStart)) + endWeight*double(input(i,inputEnd));
        }
    }

}
