#include "filmsim.hpp"

void whitepoint_blackpoint(matrix<float> &input, matrix<unsigned short> &output,
                           float whitepoint, float blackpoint)
{
    int nrows = input.nr();
    int ncols = input.nc();
    output.set_size(nrows,ncols);
#pragma omp parallel shared(output, input) firstprivate(nrows,ncols)
    {
#pragma omp for schedule(dynamic) nowait
    for(int i = 0; i < input.nr(); i++)
        for(int j = 0; j < input.nc(); j++)
        {
            float subtracted = input(i,j)-blackpoint;
            float multiplied = subtracted*(65535/whitepoint);
            output(i,j) = (unsigned short) max(min(multiplied,float(65535)),float(0));
        }
    }
}
