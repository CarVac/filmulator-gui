#include "filmSim.hpp"
#include <cmath>
#include <math.h>

template <typename T>
void downscaleDivisible1D(matrix<T> input, matrix<T> &output, int scaleFactor, bool interleaved);
template <typename T>
void downscaleBilinear1D(matrix<T> input, matrix<T> &output, int start, int end, double scaleFactor, bool interleaved);



//Scales the input to the output to fit within the output sizes.
void downscale_and_crop(matrix<float> input, matrix<float> &output, int inputStartX, int inputStartY, int inputEndX, int inputEndY, int outputXSize, int outputYSize)
{
    int inputXSize = inputEndX - inputStartX + 1;
    int inputYSize = inputEndY - inputStartY + 1;

    //Determine the split of the scaling between integer and bilinear scaling.
    //Integer is much faster, but we can only do integer multiples.
    //Bilinear can only do shrinks between 1 and 2.
    double overallScaleFactor = max(double(inputXSize)/double(outputXSize),double(inputYSize)/double(outputYSize));
    int integerScaleFactor = floor(overallScaleFactor);
    double bilinearScaleFactor = overallScaleFactor/double(integerScaleFactor);

    //Downscale in one direction
    matrix<float> bilinearX;
    matrix<float> bothX;
    downscaleBilinear1D(input,bilinearX,inputStartX,inputEndX,overallScaleFactor,true);
    downscaleDivisible1D(bilinearX,bothX,integerScaleFactor,true);

    //Then transpose
    matrix<float> bothXTransposed;
    bothXTransposed.set_size(bothX.nc(),bothX.nr());
    bothX.transpose_to(bothXTransposed);

    //Then downscale in the other direction.
    matrix<float> bothXTransposedBilinearY;
    matrix<float> bothXTransposedBothY;
    downscaleBilinear1D(bothXTransposed,bothXTransposedBilinearY,inputStartY,inputEndY,overallScaleFactor,false);
    downscaleDivisible1D(bothXTransposedBilinearY,bothXTransposedBothY,integerScaleFactor,false);

    //Then transpose to the output.
    output.set_size(bothXTransposedBothY.nc(),bothXTransposedBothY.nr());
    bothXTransposedBothY.transpose_to(output);
    return;
}

//Scales the image such that the number of columns is redeuced by integer factor scaleFactor.
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
    }
    else
    {
        outputNumCols = ceil(inputNumCols/double(scaleFactor));
    }
    output.set_size(outputNumRows,outputNumCols);

    if (interleaved)
    {
        #pragma omp parallel for shared(input, output)
        for (int i = 0; i < outputNumRows; i++)
            for (int j = 0; j < outputNumCols; j = j+3)
            {
                double sumR = 0;
                double sumG = 0;
                double sumB = 0;
                for (int k = j*scaleFactor; k < (j+3)*scaleFactor; k = k + 3 )
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
        for (int i = 0; i < outputNumRows; i++)
            for (int j = 0; j < outputNumCols; j++)
            {
                double sum = 0;
                for (int k = j*scaleFactor; k < (j+1)*scaleFactor; k++)
                {
                    sum += input(i,k);
                }
                output(i,j) = sum/scaleFactor;
            }
    }
    return;
}

//Scales the image such that the number of columns is reduced by a scaling factor between 1 and 2.
//The scaling factor is computed from the overall scale factor such that it ends up an integer multiple
// of the desired final size.
template <typename T>
void downscaleBilinear1D(matrix<T> input, matrix<T> &output, int start, int end, double overallScaleFactor, bool interleaved)
{
    int inputNumRows = input.nr();
    int inputNumCols = end - start + 1;

    int outputNumRows = inputNumRows;

    int endNumCols = round(double(inputNumCols)/overallScaleFactor);
    //We need to make sure that it ends up being a whole multiple of the final size.
    float newOverallScaleFactor = inputNumCols/double(endNumCols);
    //We assume that floor of newOverallScaleFactor is the same as the original.
    int intScaleFactor = floor(newOverallScaleFactor);
    //This is the scale factor used for bilinear.
    float scaleFactor = newOverallScaleFactor/intScaleFactor;

    int outputNumCols = endNumCols * intScaleFactor;

    if(interleaved)
        output.set_size(outputNumRows,outputNumCols*3);
    else
        output.set_size(outputNumRows,outputNumCols);

    #pragma omp parallel for shared(input, output)
    for (int i = 0; i < outputNumRows; i++)
    {
        for (int j = 0; j < outputNumCols-1; j++)
        {
            double inputPoint = (double(j) + 0.5)*scaleFactor -0.5 + double(start);
            int inputStart = floor(inputPoint);
            int inputEnd = ceil(inputPoint);
            double notUsed;
            double endWeight = modf(inputPoint, &notUsed);
            double startWeight = 1 - endWeight;
            if (interleaved)
                for (int c = 0; c < 3; c++)
                {
                    output(i,3*j + c) = startWeight*double(input(i,3*inputStart + c)) + endWeight*double(input(i,3*inputEnd + c));
                }
            else
                output(i,j) = startWeight*double(input(i,inputStart)) + endWeight*double(input(i,inputEnd));
        }
    }

}
