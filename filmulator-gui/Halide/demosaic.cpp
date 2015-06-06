//Compile with:
//g++ demosaic.cpp -g -I include/ -L bin/ -lHalide `libpng-config --cflags --ldflags` -lpthread -ldl -o demosaic -std=c++11
//
//Run with:
//LD_LIBRARY_PATH=bin ./demosaic
//For debug
//LD_LIBRARY_PATH=bin HL_DEBUG_CODEGEN=[#] ./demosaic
#include <Halide.h>
#include <stdio.h>
#include <iostream>
#include <sys/time.h>
using Halide::Image;
#include <image_io.h>
//#include <RDom.h>

using namespace Halide;

Halide::Func bayerize(Func in)
{
    Func out;
    Var x,y,c;
    out(x,y,c) = 0.0f;
    out(x,y,0) = in(2*x,   2*y,   1);//green in red rows
    out(x,y,1) = in(2*x+1, 2*y,   0); //red
    out(x,y,2) = in(2*x,   2*y+1, 2); //blue
    out(x,y,3) = in(2*x+1, 2*y+1, 1); //green in blue rows

    // G R G R
    // B G B G
    // G R G R
    // B G B G

    return out;
}

Halide::Func blurRatio_v(Func vert)
{
    Var x,y;
    //Low pass filter (sigma=2 L=4)
    Expr h0, h1, h2, h3, h4, hsum;
    h0 = 1.0f;
    h1 = exp( -1.0f/8.0f);
    h2 = exp( -4.0f/8.0f);
    h3 = exp( -9.0f/8.0f);
    h4 = exp(-16.0f/8.0f);
    hsum = h0 + 2.0f*(h1 + h2 + h3 + h4);
    h0 /= hsum;
    h1 /= hsum;
    h2 /= hsum;
    h3 /= hsum;
    h4 /= hsum;

    Func out;
    out(x,y) = h0 * vert(x,y) +
        h1 * (vert(x,y-1) + vert(x,y+1)) +
        h2 * (vert(x,y-2) + vert(x,y+2)) +
        h3 * (vert(x,y-3) + vert(x,y+3)) +
        h4 * (vert(x,y-4) + vert(x,y+4));
    return out;
}

Halide::Func blurRatio_h(Func hor)
{
    Var x,y;
    //Low pass filter (sigma=2 L=4)
    Expr h0, h1, h2, h3, h4, hsum;
    h0 = 1.0f;
    h1 = exp( -1.0f/8.0f);
    h2 = exp( -4.0f/8.0f);
    h3 = exp( -9.0f/8.0f);
    h4 = exp(-16.0f/8.0f);
    hsum = h0 + 2.0f*(h1 + h2 + h3 + h4);
    h0 /= hsum;
    h1 /= hsum;
    h2 /= hsum;
    h3 /= hsum;
    h4 /= hsum;

    Func out;
    out(x,y) = h0 * hor(x,y) +
        h1 * (hor(x-1,y) + hor(x+1,y)) +
        h2 * (hor(x-2,y) + hor(x+2,y)) +
        h3 * (hor(x-3,y) + hor(x+3,y)) +
        h4 * (hor(x-4,y) + hor(x+4,y));
    return out;
}

Halide::Func demosaic(Func deinterleaved)
{
    Func output;
    //A large part of the algorithm is spent processing vertical and horizontal separately.
    //This means that we can duplicate the original data into vertical and horizontal
    //And run the horizontal actually vertically in memory
    //Then we can vectorize very easily.

    Var x, y, c;

    //Group the pixels into fours.
    Func r_r, g_gr, g_gb, b_b;
    g_gr(x, y) = deinterleaved(x,y,0);
    r_r(x, y)  = deinterleaved(x,y,1);
    b_b(x, y)  = deinterleaved(x,y,2);
    g_gb(x, y) = deinterleaved(x,y,3);


    //Initial demosaic:
    //We need to make this bilinear, and sharpen the estimated colors at the end.
    //
    //The paper uses an implementation that sharpens the off colors first, but that
    //violates their assumption of smooth color transitions and worsens performance.


    //First calculate the green at the red and blue pixels.
    
    //Red pixels
    Func gAtR_v, gAtR_h;
    gAtR_h(x,y) = (g_gr(x,y) + g_gr(x+1,y))/2;// + (2*r_r(x,y) - r_r(x-1,y) - r_r(x+1,y))/4;
    gAtR_v(x,y) = (g_gb(x,y-1) + g_gb(x,y))/2;// + (2*r_r(x,y) - r_r(x,y-1) - r_r(x,y+1))/4;
    //Blue pixels
    Func gAtB_v, gAtB_h;
    gAtB_h(x,y) = (g_gb(x-1,y) + g_gb(x,y))/2;// + (2*b_b(x,y) - b_b(x-1,y) - b_b(x+1,y))/4;
    gAtB_v(x,y) = (g_gr(x,y) + g_gr(x,y+1))/2;// + (2*b_b(x,y) - b_b(x,y-1) - b_b(x,y+1))/4;

    //Next, calculate the red and blue at the green pixels.

    //Red rows
    Func rAtGR_h, bAtGR_v;
    rAtGR_h(x,y) = (r_r(x-1,y) + r_r(x,y))/2;// + (2*g_gr(x,y) - g_gr(x-1,y) - g_gr(x+1,y))/4;
    bAtGR_v(x,y) = (b_b(x,y-1) + b_b(x,y))/2;// + (2*g_gr(x,y) - g_gr(x,y-1) - g_gr(x,y+1))/4;
    //Blue rows
    Func bAtGB_h, rAtGB_v;
    bAtGB_h(x,y) = (b_b(x,y) + b_b(x+1,y))/2;// + (2*g_gb(x,y) - g_gb(x-1,y) - g_gb(x+1,y))/4;
    rAtGB_v(x,y) = (r_r(x,y) + r_r(x,y+1))/2;// + (2*g_gb(x,y) - g_gb(x,y-1) - g_gb(x,y+1))/4;


    //Get the logs of the color ratios
    
    //On red pixels
    Func grRatioAtR_h, grRatioAtR_v;
    grRatioAtR_h(x,y) = gAtR_h(x,y)/r_r(x,y);
    grRatioAtR_v(x,y) = gAtR_v(x,y)/r_r(x,y);
    //On blue pixels
    Func gbRatioAtB_h, gbRatioAtB_v;
    gbRatioAtB_h(x,y) = gAtB_h(x,y)/b_b(x,y);
    gbRatioAtB_v(x,y) = gAtB_v(x,y)/b_b(x,y);
    //On green pixels in red rows
    Func grRatioAtGR_h, gbRatioAtGR_v;
    grRatioAtGR_h(x,y) = g_gr(x,y)/rAtGR_h(x,y);
    gbRatioAtGR_v(x,y) = g_gr(x,y)/bAtGR_v(x,y);
    //On green pixels in blue rows
    Func gbRatioAtGB_h, grRatioAtGB_v;
    gbRatioAtGB_h(x,y) = g_gb(x,y)/bAtGB_h(x,y);
    grRatioAtGB_v(x,y) = g_gb(x,y)/rAtGB_v(x,y);

    //Blur the color ratios, to estimate the actual color ratio.
    //These are 1d blurs.
    //Vertical is blurred vertically.
    //Horizontal is blurred horizontally.
    //It just happens to work out.

    //Combined color ratios
    Func colorRatios_h, colorRatios_v;
    colorRatios_h(x,y) = select(y%2 == 0,//Rows
            select(x%2 == 0,//Red row
                grRatioAtGR_h(x/2, y/2),//Green in red row
                grRatioAtR_h(x/2, y/2)),//Red
            select(x%2 == 1,//Blue row
                gbRatioAtB_h(x/2, y/2),//Blue
                gbRatioAtGB_h(x/2,y/2)));//Green in blue row
    colorRatios_v(x,y) = select(y%2 == 0,//Rows
            select(x%2 == 0,//Red row
                gbRatioAtGR_v(x/2, y/2),//Green in red row
                grRatioAtR_v(x/2, y/2)),//Red
            select(x%2 == 1,//Blue row
                gbRatioAtB_v(x/2, y/2),//Blue
                grRatioAtGB_v(x/2,y/2)));//Green in blue row

    //Blurred combined matrices
    Func blurredRatios_h, blurredRatios_v;
    blurredRatios_h = blurRatio_h(colorRatios_h);
    blurredRatios_v = blurRatio_v(colorRatios_v);

    //Now we take the logs of them.

    //First is the log of the color ratios.
    //This is the Observed Color Difference Y.
    Func Y_h, Y_v;
    Y_h(x,y) = log(colorRatios_h(x,y));
    Y_v(x,y) = log(colorRatios_v(x,y));

    //Next is the log of the blurred color ratios.
    //This is the Estimated True Color Difference Ys ~~= X
    Func Ys_h, Ys_v;
    Ys_h(x,y) = log(blurredRatios_h(x,y));
    Ys_v(x,y) = log(blurredRatios_v(x,y));

    
    //To get a Linear Minimum Mean Square Error (LMMSE) estimate,
    //we want
    //xhat = E[x] + cov(x,y)/var(y) * (y - E[y])
    //
    //Empirically, since we don't all of these, it becomes (locally
    //xhat = mean(x) + var(x)/(var(x) + var(%nu)) * (y - mean(x))
    //
    //x is approximated by Ys; it's low-pass.
    //nu, the error, is approximated by (Ys - Y); it's high-pass.
    Func X_h, X_v;
    X_h(x,y) = Ys_h(x,y);
    X_v(x,y) = Ys_v(x,y);

    Func NU_h, NU_v;
    NU_h(x,y) = Ys_h(x,y) - Y_h(x,y);
    NU_v(x,y) = Ys_v(x,y) - Y_v(x,y);

    //Neighborhood mean of X
    Func MUx_h, MUx_v;
    RDom r(-4, 9);
    MUx_h(x,y) = 0.0f;
    MUx_h(x,y) += X_h(r+x,y) / 9.0f;
    MUx_v(x,y) = 0.0f;
    MUx_v(x,y) += X_v(x,r+y) / 9.0f;

    //Neighborhood variance of X
    Func SIGMAx_h, SIGMAx_v;
    SIGMAx_h(x,y) = 0.0f;
    SIGMAx_h(x,y) += pow(X_h(r+x,y) - MUx_h(r+x,y),2) / 9.0f;
    SIGMAx_v(x,y) = 0.0f;
    SIGMAx_v(x,y) += pow(X_v(x,r+y) - MUx_v(x,r+y),2) / 9.0f;

    //Neighborhood variance of nu
    Func SIGMAnu_h, SIGMAnu_v;
    SIGMAnu_h(x,y) = 0.0f;
    SIGMAnu_h(x,y) += pow(X_h(r+x,y) - Y_h(r+x,y),2) / 9.0f;
    SIGMAnu_v(x,y) = 0.0f;
    SIGMAnu_v(x,y) += pow(X_v(x,r+y) - Y_v(x,r+y),2) / 9.0f;

    //LMMSE estimation in each direction
    Func Xlmmse_h, Xlmmse_v;
    Xlmmse_h(x,y) = MUx_h(x,y) +
        (Y_h(x,y) - MUx_h(x,y)) * SIGMAx_h(x,y) /
        (SIGMAx_h(x,y) + SIGMAnu_h(x,y) + 1e-7f);
    Xlmmse_v(x,y) = MUx_v(x,y) +
        (Y_v(x,y) - MUx_v(x,y)) * SIGMAx_v(x,y) /
        (SIGMAx_v(x,y) + SIGMAnu_h(x,y) + 1e-7f);

    //The expected estimation error is Xerror = X - Xlmmse
    //We don't use it.

    //The variance of the estimation error, we do use for the weighting.
    Func SIGMAer_h, SIGMAer_v;
    SIGMAer_h(x,y) = SIGMAx_h(x,y) - SIGMAx_h(x,y)/(SIGMAx_h(x,y) + SIGMAnu_h(x,y));
    SIGMAer_v(x,y) = SIGMAx_v(x,y) - SIGMAx_v(x,y)/(SIGMAx_v(x,y) + SIGMAnu_v(x,y));

    //Weight of estimate
    Func W_h, W_v;
    W_h(x,y) = SIGMAer_v(x,y) / (SIGMAer_h(x,y) + SIGMAer_v(x,y));
    W_v(x,y) = SIGMAer_h(x,y) / (SIGMAer_v(x,y) + SIGMAer_h(x,y));

    //Combine to get the final log of the color ratio we'll use
    Func X;
    X(x,y) = W_h(x,y)*Xlmmse_h(x,y) + W_v(x,y)*Xlmmse_v(x,y);

    //Separate the green/color ratios back out.
    //They're only valid on red and blue pixels.
    //Reminder: these are logs, not actually the ratios yet.
    Func grLogRatioAtR, gbLogRatioAtB;
    grLogRatioAtR(x,y) = X(2*x+1, 2*y+0);
    gbLogRatioAtB(x,y) = X(2*x+0, 2*y+1);
    
    //Compute the green/color ratios at the opposite colors.
    //It's just the mean of the color ratios of the proper colors nearby.
    //Once more, these are still logs.
    Func gbLogRatioAtR, grLogRatioAtB;
    gbLogRatioAtR(x,y) = (gbLogRatioAtB(x,y) + gbLogRatioAtB(x,y-1) +
            gbLogRatioAtB(x+1,y-1) + gbLogRatioAtB(x+1,y)) / 4.0f;
    grLogRatioAtB(x,y) = (grLogRatioAtR(x,y) + grLogRatioAtR(x,y+1) +
            grLogRatioAtR(x-1,y+1) + grLogRatioAtR(x-1,y)) / 4.0f;

    //Compute the color ratios at green
    //Again, still logs.
    Func grLogRatioAtGR, gbLogRatioAtGR, grLogRatioAtGB, gbLogRatioAtGB;
    grLogRatioAtGR(x,y) = (grLogRatioAtR(x-1,y) + grLogRatioAtR(x,y) +
            grLogRatioAtB(x,y-1) + grLogRatioAtB(x,y)) / 4.0f;
    grLogRatioAtGB(x,y) = (grLogRatioAtR(x,y) + grLogRatioAtR(x,y+1) +
            grLogRatioAtB(x,y) + grLogRatioAtB(x+1,y)) / 4.0f;
    gbLogRatioAtGR(x,y) = (gbLogRatioAtR(x-1,y) + gbLogRatioAtR(x,y) +
            gbLogRatioAtB(x,y-1) + gbLogRatioAtB(x,y)) / 4.0f;
    gbLogRatioAtGB(x,y) = (gbLogRatioAtR(x,y) + gbLogRatioAtR(x,y+1) +
            gbLogRatioAtB(x,y) + gbLogRatioAtB(x+1,y)) / 4.0f;

    //The libraw lmmse does a median filter...why?

    //Turn the logs back into ratios.
    Func grRatioAtR,  gbRatioAtR,  gbRatioAtB,  grRatioAtB;
    Func grRatioAtGR, gbRatioAtGR, gbRatioAtGB, grRatioAtGB;
    grRatioAtR(x,y)  = exp(grLogRatioAtR(x,y)) + 1e-7f;
    gbRatioAtR(x,y)  = exp(gbLogRatioAtR(x,y)) + 1e-7f;
    gbRatioAtB(x,y)  = exp(gbLogRatioAtB(x,y)) + 1e-7f;
    grRatioAtB(x,y)  = exp(grLogRatioAtB(x,y)) + 1e-7f;
    grRatioAtGR(x,y) = exp(grLogRatioAtGR(x,y))+ 1e-7f;
    gbRatioAtGR(x,y) = exp(gbLogRatioAtGR(x,y))+ 1e-7f;
    gbRatioAtGB(x,y) = exp(gbLogRatioAtGB(x,y))+ 1e-7f;
    grRatioAtGB(x,y) = exp(grLogRatioAtGB(x,y))+ 1e-7f;

    //Now output the values we want.
    output(x,y,c) = select(c == 0,
            //Red channel
            select(y%2 == 0,
                select(x%2 == 0,
                    //Green pixel in red row
                    g_gr(x/2,y/2) / grRatioAtGR(x/2,y/2),
                    //Red pixel
                    r_r(x/2,y/2)),
                select(x%2 == 0,
                    //Blue pixel
                    b_b(x/2,y/2) * gbRatioAtB(x/2,y/2) / grRatioAtB(x/2,y/2),
                    //Green pixel in blue row
                    g_gb(x/2,y/2) / grRatioAtGB(x/2,y/2))),
            select(c == 1,
                //Green channel
                select(y%2 == 0,
                    select(x%2 == 0,
                        //Green pixel in red row
                        g_gr(x/2,y/2),
                        //Red pixel
                        r_r(x/2,y/2) * grRatioAtR(x/2,y/2)),
                    select(x%2 == 0,
                        //Blue pixel
                        b_b(x/2,y/2) * gbRatioAtB(x/2,y/2),
                        //Green pixel in blue row
                        g_gb(x/2,y/2))),
                //Blue channel
                select(y%2 == 0,
                    select(x%2 == 0,
                        //Green pixel in red row
                        g_gr(x/2,y/2) / grRatioAtGR(x/2,y/2),
                        //Red pixel
                        r_r(x/2,y/2) * grRatioAtR(x/2,y/2) / gbRatioAtR(x/2,y/2)),
                    select(x%2 == 0,
                        //Blue pixel
                        b_b(x/2,y/2),
                        //Green pixel in blue row
                        g_gb(x/2,y/2) / gbRatioAtGB(x/2,y/2)))));

    return output;
}

int main(int argc, char **argv)
{
    Var x, y, c;
    Halide::Image<uint8_t> input = load<uint8_t>("P1040567.png");
    timeval t1, t2;
    gettimeofday(&t1, NULL);
    Func toFloat, toBayer, toDemosaic, toInt;
    toFloat(x,y,c) = cast<float>(input(x,y,c))/255.0f;
    toBayer = bayerize(toFloat);
    toDemosaic = demosaic(toBayer);
    toInt(x,y,c) = cast<uint8_t>(toDemosaic(x,y,c)*255.0f);
    Halide::Image<uint8_t> output  = toInt.realize(input.width(),input.height(),input.channels());
    gettimeofday(&t2, NULL);
    save(output,"demosaiced.png");
    std::cout<<float(t2.tv_sec - t1.tv_sec) + float(t2.tv_usec - t1.tv_usec)/1000000.0f << std::endl;
    return 0;
}
/*
int main(int argc, char **argv)
{
    Var x, y, c;
    //Halide::Image<uint8_t> input = load<uint8_t>("P1040567.png");
    ImageParam input(type_of<uint8_t>(), 3);
    Func toFloat, toBayer, toDemosaic, toInt;
    toFloat(x,y,c) = cast<float>(input(x,y,c))/255.0f;
    toBayer = bayerize(toFloat);
    toDemosaic = demosaic(toBayer);
    toInt(x,y,c) = cast<uint8_t>(toDemosaic(x,y,c)*255.0f);
    //Halide::Image<uint8_t> output  = toInt.realize(input.width(),input.height(),input.channels());
    std::vector<Argument> args(1);
    args[0] = input;
    toInt.compile_to_file("demosaic-lmmse", args);
    return 0;
}*/
