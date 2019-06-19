#include "filmSim.hpp"
#include <cmath>
#include <math.h>
#include <iostream>

using std::cout;
using std::endl;

template <typename T>
void downscaleDivisible1D(const matrix<T> input,
                          matrix<T> &output,
                          const int scaleFactor,
                          const bool interleaved);

template <typename T>
void downscaleBilinear1D(const matrix<T> input,
                         matrix<T> &output,
                         const int start,
                         const int end,
                         const double scaleFactor,
                         const bool interleaved);

template <typename T>
void upscaleBilinear1D(const matrix<T> input,
                       matrix<T> &output,
                       const int outNumCols,
                       const bool interleaved);


//Scales the input to the output to fit within the output sizes.
void downscale_and_crop(const matrix<float> input,
                        matrix<float> &output,
                        const int inputStartX,
                        const int inputStartY,
                        const int inputEndX,
                        const int inputEndY,
                        const int outputXSizeLimit,
                        const int outputYSizeLimit)
{
    const int inputXSize = inputEndX - inputStartX + 1;
    const int inputYSize = inputEndY - inputStartY + 1;

    //If the output size limit is bigger than the image, shrink it to fit.
    const int outputXSize = min(inputXSize, outputXSizeLimit);
    const int outputYSize = min(inputYSize, outputYSizeLimit);

    //Determine the split of the scaling between integer and bilinear scaling.
    //Integer is much faster, but we can only do integer multiples.
    //Bilinear can only do shrinks between 1 and 2.
    const double overallScaleFactor = max(double(inputXSize)/double(outputXSize),double(inputYSize)/double(outputYSize));
    if (overallScaleFactor == 1)
    {
        if ((outputXSize == input.nc()/3) && (outputYSize == input.nr()))
        {
            // no scale and no crop
            output = input;
            return;
        } else {
            // crop only, no scale
            output.set_size(outputYSize, outputXSize*3);
            #pragma omp parallel for shared(output)
            for (int i = 0; i < outputYSize; i++)
            {
                int iin = inputStartX + i;
                for (int j = 0; j < outputXSize*3; j += 3)
                {
                    int jin = 3*inputStartY + j;
                    output(i, j  ) = input(iin, jin  );
                    output(i, j+1) = input(iin, jin+1);
                    output(i, j+2) = input(iin, jin+2);
                }
            }
            return;
        }
    }
    const int integerScaleFactor = floor(overallScaleFactor);

    //Downscale in one direction
    matrix<float> bilinearX;
    matrix<float> bothX;
    downscaleBilinear1D(input,bilinearX,inputStartX,inputEndX,overallScaleFactor,true);
    if (integerScaleFactor != 1)
    {
        downscaleDivisible1D(bilinearX,bothX,integerScaleFactor,true);
    }
    matrix<float> &temp = integerScaleFactor == 1 ? bilinearX : bothX;

    //Then transpose
    matrix<float> bothXTransposed;
    bothXTransposed.set_size(temp.nc(),temp.nr());
    temp.transpose_to(bothXTransposed);

    //Then downscale in the other direction.
    matrix<float> bothXTransposedBilinearY;
    matrix<float> bothXTransposedBothY;
    downscaleBilinear1D(bothXTransposed,bothXTransposedBilinearY,inputStartY,inputEndY,overallScaleFactor,false);
    if (integerScaleFactor != 1)
    {
        downscaleDivisible1D(bothXTransposedBilinearY,bothXTransposedBothY,integerScaleFactor,false);
    }
    matrix<float> &result = integerScaleFactor == 1 ? bothXTransposedBilinearY : bothXTransposedBothY;

    //Then transpose to the output.
    output.set_size(result.nc(),result.nr());
    result.transpose_to(output);
    return;
}

//Scales the image such that the number of columns is redeuced by integer factor scaleFactor.
template <typename T>
void downscaleDivisible1D(const matrix<T> input,
                          matrix<T> &output,
                          const int scaleFactor,
                          const bool interleaved)
{
    const int inputNumRows = input.nr();
    const int inputNumCols = input.nc();
    const int outputNumRows = inputNumRows;
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
        #pragma omp parallel for shared(output)
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
        #pragma omp parallel for shared(output)
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
void downscaleBilinear1D(const matrix<T> input,
                         matrix<T> &output,
                         const int start,
                         const int end,
                         const double overallScaleFactor,
                         const bool interleaved)
{
    const int inputNumRows = input.nr();
    const int inputNumCols = end - start + 1;

    const int outputNumRows = inputNumRows;

    const int endNumCols = max(1.0,round(double(inputNumCols)/overallScaleFactor));
    //We need to make sure that it ends up being a whole multiple of the final size.
    const float newOverallScaleFactor = inputNumCols/double(endNumCols);
    //We assume that floor of newOverallScaleFactor is the same as the original.
    const int intScaleFactor = max(1.0f,floor(newOverallScaleFactor));
    //This is the scale factor used for bilinear.
    const float scaleFactor = newOverallScaleFactor/intScaleFactor;

    const int outputNumCols = endNumCols * intScaleFactor;

    if(interleaved)
        output.set_size(outputNumRows,outputNumCols*3);
    else
        output.set_size(outputNumRows,outputNumCols);

    #pragma omp parallel for shared(output)
    for (int i = 0; i < outputNumRows; i++)
    {
        for (int j = 0; j < outputNumCols; j++)
        {
            const double inputPoint = (double(j) + 0.5)*scaleFactor -0.5 + double(start);
            const int inputStart = floor(inputPoint);
            const int inputEnd = ceil(inputPoint);
            double notUsed;
            const double endWeight = modf(inputPoint, &notUsed);
            const double startWeight = 1 - endWeight;
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

//Scales the image up so that the number of columns is increased to the desired number.
//outputNumCols should be for the un-interleaved image.
//TODO: COMPLETE THIS
template <typename T>
void upscaleBilinear1D(const matrix<T> input,
                       matrix<T> &output,
                       const int outNumCols,
                       const bool interleaved)
{
    const int inputNumRows = input.nr();
    const int inputNumCols = input.nc();

    if (outNumCols <= inputNumCols)
    {
        output.set_size(0,0);
        return;
    }

    if (interleaved)
    {
        output.set_size(inputNumRows, outNumCols*3);
    }
    else
    {
        output.set_size(inputNumRows, outNumCols);
    }


}
