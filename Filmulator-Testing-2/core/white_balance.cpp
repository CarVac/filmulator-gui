#include "filmsim.hpp"
#include <algorithm>

void temp_tone_to_xy(double const temp, double const tone,
                     double &tempToneX, double &tempToneY)
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
    double normal = -1.0/dYdX;

    //solution of x^2 + y^2 = tone^2; y=normal*x
    double toneX = sqrt( tone/(1.0+ pow(normal,2) ) );
    double toneY = normal*tempToneX;

    tempToneX = tempX + toneX;
    tempToneY = tempY + toneY;

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
    r =  3.2406*x + -1.5372*y + -0.4986*z;
    g = -0.9686*x +  1.8758*y +  0.0415*z;
    b =  0.0557*x + -0.2040*y +  1.0570*z;
}

void white_balance ( matrix<float> &input, matrix<float> &output,
                     double temp, double tone )
{
    double tempToneX, tempToneY;
    temp_tone_to_xy(temp,tone,tempToneX,tempToneY);
    double xShift = (1.0/3.0) - tempToneX;
    double yShift = (1.0/3.0) - tempToneY;
    xShift = 0;
    yShift = 0;

    int nrows = input.nr();
    int ncols = input.nc();

    output.set_size(nrows,ncols);
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
            output(i,j+1) = max(newB,0.0);
            output(i,j+2) = max(newG,0.0);
        }
}
