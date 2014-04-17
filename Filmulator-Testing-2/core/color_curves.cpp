#include "filmsim.hpp"

void colorCurves(matrix<unsigned short> &input, matrix<unsigned short> &output,
                  LUT lutR, LUT lutG, LUT lutB)
{

    //Check for null inputs
    if( lutR.isUnity() && lutG.isUnity() && lutB.isUnity() )
    {
        output = input;
    }
    else
    {
        int nrows = input.nr();
        int ncols = input.nc();
        output.set_size( nrows, ncols );
#pragma omp parallel shared( output, input ) firstprivate( nrows, ncols )
        {
#pragma omp for schedule( dynamic ) nowait
        for ( int i = 0; i < nrows; i++ )
            for ( int j = 0; j < ncols; j = j + 3 )
            {
                    output( i, j  ) = lutR[ input( i, j   ) ];
                    output( i, j+1) = lutG[ input( i, j+1 ) ];
                    output( i, j+2) = lutB[ input( i, j+2 ) ];
            }
        }
    }
    return;
}
