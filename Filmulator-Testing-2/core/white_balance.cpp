#include "filmsim.hpp"
#include <algorithm>

void temp_tint_to_xy(double const temp, double const tint,
                     double &tempTintX, double &tempTintY)
{
    double tempX;

    if(temp < 4000)
    {
        tempX = -0.2661239 * pow(10,9) * pow(temp,-3) +
                -0.2343589 * pow(10,6) * pow(temp,-2) +
                 0.8776956 * pow(10,3) * pow(temp,-1) +
                 0.179910;
    }
    else
    {
        tempX = -3.0258469 * pow(10,9) * pow(temp,-3) +
                 2.1070379 * pow(10,6) * pow(temp,-2) +
                 0.2226347 * pow(10,3) * pow(temp,-1) +
                 0.24039;
    }

    double tempY;
    double dYdX;;

    if(temp < 2222)
    {
        tempY = -1.1063814  * pow(tempX,3) +
                -1.34811020 * pow(tempX,2) +
                 2.18555832 * tempX +
                -0.20219683;

        dYdX = 3*-1.1063814  * pow(tempX,2) +
               2*-1.34811020 * tempX +
                  2.18555832;
    }
    else if(temp < 4000)
    {
        tempY = -0.9549476  * pow(tempX,3) +
                -1.37418593 * pow(tempX,2) +
                 2.09137015 * tempX +
                -0.16748867;

        dYdX = 3*-0.9549476  * pow(tempX,2) +
               2*-1.37418593 * tempX +
                  2.09137015;
    }
    else
    {
        tempY =  3.0817580  * pow(tempX,3) +
                -5.8733867 * pow(tempX,2) +
                 3.75112997 * tempX +
                -0.37001483;

        dYdX = 3* 3.0817580  * pow(tempX,2) +
               2*-5.8733867 * tempX +
                 -0.37001483;
    }

    if(fabs(dYdX) > 0.01)
    {
        double normal = -1.0/dYdX;

        //solution of x^2 + y^2 = tint^2; y=normal*x
        double tintX = sqrt( pow(tint,2)/(1.0+ pow(normal,2) ) );
        if (tint < 0)
            tintX = -tintX;
        double tintY = normal*tintX;

        tempTintX = tempX + tintX;
        tempTintY = tempY + tintY;
    }
    else
    {
        tempTintX = tempX;
        if(dYdX > 0)
            tempTintY = tempY - tint;
        else
            tempTintY = tempY + tint;
    }

    return;
}

void rgb_to_xyz(double  r, double  g, double  b,
                double &x, double &y, double &z)
{
    x = 0.4124*r + 0.3576*g + 0.1805*b;
    y = 0.2126*r + 0.7152*g + 0.0722*b;
    z = 0.0193*r + 0.1192*g + 0.9502*b;
}

void xyz_to_rgb(double  x, double  y, double  z,
                double &r, double &g, double &b)
{
    r =  3.2406*x + -1.5372*y + -0.4989*z;
    g = -0.9689*x +  1.8758*y +  0.0415*z;
    b =  0.0557*x + -0.2041*y +  1.0573*z;
}

void white_balance ( matrix<float> &input, matrix<float> &output,
                     double temp, double tone )
{
    double tempTintX, tempTintY;
    temp_tint_to_xy(temp,tone,tempTintX,tempTintY);
    double xShift = (1.0/3.0) - tempTintX;
    double yShift = (1.0/3.0) - tempTintY;

    int nrows = input.nr();
    int ncols = input.nc();

    output.set_size(nrows,ncols);

#pragma omp parallel shared(output, input) firstprivate(nrows,ncols,xShift,yShift)
        {
#pragma omp for schedule(dynamic) nowait
    for(int i = 0; i < nrows; i++)
        for(int j = 0; j < ncols; j = j+3)
        {
            double inputX, inputY, inputZ;
            rgb_to_xyz(input(i,j),input(i,j+1),input(i,j+2),
                       inputX    ,inputY      ,inputZ);
            double magnitude = inputX + inputY + inputZ;
            double newX = magnitude*(inputX/magnitude + xShift);
            double newY = magnitude*(inputY/magnitude + yShift);
            double newZ = magnitude - newX - newY;

            double newR, newG, newB;
            xyz_to_rgb(newX, newY, newZ, newR, newG, newB);

            output(i,j  ) = max(newR,0.0);
            output(i,j+1) = max(newG,0.0);
            output(i,j+2) = max(newB,0.0);
        }
    }
}
