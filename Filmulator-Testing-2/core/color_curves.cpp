#include "filmsim.hpp"

void color_curves(matrix<unsigned short> &input, matrix<unsigned short> &output,
                  LUT lutR, LUT lutG, LUT lutB)
{
    cout << "in color_curves" << endl;
    int nrows = input.nr();
    int ncols = input.nc();
    cout << "output.nr(): " << output.nr() << " output.nc(): " << output.nc() << endl;
    output.set_size(nrows,ncols);
    cout << "after set_size" << endl;
    if(lutR.isUnity() && lutG.isUnity() && lutB.isUnity())
    {
        cout << "using unity" << endl;
        output = input;
    }
    else
    {
        cout <<"not using unity" << endl;
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
    }
    return;
}
