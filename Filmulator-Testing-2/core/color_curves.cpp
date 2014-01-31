#include "filmsim.hpp"

void color_curves(matrix<unsigned short> &input, matrix<unsigned short> &output,
                  LUT lutR, LUT lutG, LUT lutB)
{
    int nrows = input.nr();
    int ncols = input.nc();
    output.set_size(nrows,ncols);
    if(lutR.isUnity() && lutG.isUnity() && lutB.isUnity())
        output = input;
    else
#pragma omp parallel shared(output, input) firstprivate(nrows,ncols)
    {
#pragma omp for schedule(dynamic) nowait
    for(int i = 0; i < input.nr(); i++)
        for(int j = 0; j < input.nc(); j = j +3)
        {
                output(i,j) = lutR[input(i,j)];
                output(i,j+1) = lutG[input(i,j+1)];
                output(i,j+2) = lutB[input(i,j+2)];
        }
    }
    return;
}
