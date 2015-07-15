//Compile with:
//g++ demosaic.cpp -g -I include/ -L bin/ -lHalide `libpng-config --cflags --ldflags` -lpthread -ldl -o demosaic -std=c++11
//
//Run with:
//LD_LIBRARY_PATH=bin ./demosaic
//For debug
//LD_LIBRARY_PATH=bin HL_DEBUG_CODEGEN=[#] ./demosaic
//
//g++ demosaic.cpp -g -I include/ -L bin/ -lHalide `libpng-config --cflags --ldflags` -lpthread -ldl -o demosaic -std=c++11 && LD_LIBRARY_PATH=bin HL_DEBUG_CODEGEN=0 ./demosaic
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
    
    out(x,y,c) = select(
            c < 2, select(
                c == 0,
                in(2*x+1,2*y+0,1),
                in(2*x+2,2*y+0,0)),
            select(
                c == 2,
                in(2*x+1,2*y+1,2),
                in(2*x+2,2*y+1,1)));
    /*
    out(x,y,c) = select(
            c < 2, select(
                c == 0,
                in(2*x+0,2*y+0,1),
                in(2*x+1,2*y+0,0)),
            select(
                c == 2,
                in(2*x+0,2*y+1,2),
                in(2*x+1,2*y+1,1)));
    */
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
    /*
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
    */
    h0 = .203125f;
    h1 = .1796875f;
    h2 = .1171875f;
    h3 = .0703125f;
    h4 = .03125f;


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
    /*    
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
    */
    h0 = .203125f;
    h1 = .1796875f;
    h2 = .1171875f;
    h3 = .0703125f;
    h4 = .03125f;

    Func out;
    out(x,y) = h0 * hor(x,y) +
        h1 * (hor(x-1,y) + hor(x+1,y)) +
        h2 * (hor(x-2,y) + hor(x+2,y)) +
        h3 * (hor(x-3,y) + hor(x+3,y)) +
        h4 * (hor(x-4,y) + hor(x+4,y));
    return out;
}

Tuple swap2(Expr a, Expr b) {
    return Tuple(min(a,b), max(a,b));
}

Tuple sort3(Expr a, Expr b, Expr c) {
    Tuple x = swap2(a,b);
    Tuple y = swap2(x[1],c);
    Tuple z = swap2(x[0],y[1]);
    return Tuple(z[0], z[1], y[1]);
}

Halide::Func demosaic(Func deinterleaved)
{
    Func output;
    //A large part of the algorithm is spent processing vertical and horizontal separately.
    //This means that we can duplicate the original data into vertical and horizontal
    //And run the horizontal actually vertically in memory
    //Then we can vectorize very easily.

    Var x, y, c;
    Var xo, yo, xi, yi, tile_index;
    //Optimization variables

    //Group the pixels into fours.
    Func r_r, g_gr, g_gb, b_b;

    g_gr(x, y) = deinterleaved(x,y,0) + 0.01f;
    r_r(x, y)  = deinterleaved(x,y,1)*1.2 + 0.01f;
    b_b(x, y)  = deinterleaved(x,y,2)*1.3 + 0.01f;
    g_gb(x, y) = deinterleaved(x,y,3) + 0.01f;

    //Initial demosaic:
    //We need to make this bilinear, and sharpen the estimated colors at the end.
    //
    //The paper uses an implementation that sharpens the off colors first, but that
    //violates their assumption of smooth color transitions and worsens performance.


    //First calculate the green at the red and blue pixels.

    //Red pixels
    Func gAtR_v, gAtR_h;
    gAtR_h(x,y) = (g_gr(x,y) + g_gr(x+1,y))/2.0f;
    gAtR_v(x,y) = (g_gb(x,y-1) + g_gb(x,y))/2.0f;
    //Blue pixels
    Func gAtB_v, gAtB_h;
    gAtB_h(x,y) = (g_gb(x-1,y) + g_gb(x,y))/2.0f;
    gAtB_v(x,y) = (g_gr(x,y) + g_gr(x,y+1))/2.0f;

    //Next, calculate the red and blue at the green pixels.

    //Red rows
    Func rAtGR_h, bAtGR_v;
    rAtGR_h(x,y) = (r_r(x-1,y) + r_r(x,y))/2.0f;
    bAtGR_v(x,y) = (b_b(x,y-1) + b_b(x,y))/2.0f;
    //Blue rows
    Func bAtGB_h, rAtGB_v;
    bAtGB_h(x,y) = (b_b(x,y) + b_b(x+1,y))/2.0f;
    rAtGB_v(x,y) = (r_r(x,y) + r_r(x,y+1))/2.0f;


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
            select(x%2 == 0,//Blue row
                gbRatioAtB_h(x/2, y/2),//Blue
                gbRatioAtGB_h(x/2,y/2)));//Green in blue row
    colorRatios_v(x,y) = select(y%2 == 0,//Rows
            select(x%2 == 0,//Red row
                gbRatioAtGR_v(x/2, y/2),//Green in red row
                grRatioAtR_v(x/2, y/2)),//Red
            select(x%2 == 0,//Blue row
                gbRatioAtB_v(x/2, y/2),//Blue
                grRatioAtGB_v(x/2,y/2)));//Green in blue row

    //Now we take the logs of them.

    //First is the log of the color ratios.
    //This is the Observed Color Difference Y.
    Func Y_h, Y_v;
    Y_h(x,y) = log(colorRatios_h(x,y));
    Y_v(x,y) = log(colorRatios_v(x,y));

    //Next is the blurred log of the color ratios.
    //This is the Estimated True Color Difference Ys ~~= X
    Func Ys_h, Ys_v;
    Ys_h = blurRatio_h(Y_h);
    Ys_v = blurRatio_v(Y_v);

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

    //Neighborhood mean of X
    Func MUx_h, MUx_v;
    Func momh, momv;
    RDom r1(-4, 9);
#define DOMDIV 9.0f
    momh(x,y) = 0.0f;
    momh(x,y) += X_h(r1+x,y);
    momv(x,y) = 0.0f;
    momv(x,y) += X_v(x,r1+y);

    MUx_h(x,y) = momh(x,y)/DOMDIV;
    MUx_v(x,y) = momv(x,y)/DOMDIV;

    //Confirmed to be exactly the same =========================================

    //Neighborhood variance of X
    Func SIGMAx_h, SIGMAx_v;
    Func ph,pv;//sums of squares
    RDom r2(-4,9);
    ph(x,y) = 0.0f;
    ph(x,y) += X_h(r2+x,y)*X_h(r2+x,y);
    pv(x,y) = 0.0f;
    pv(x,y) += X_v(x,r2+y)*X_v(x,r2+y);
    SIGMAx_h(x,y) = ph(x,y) / 8.0f - momh(x,y)*momh(x,y)/(8.0f*9.0f);
    SIGMAx_v(x,y) = pv(x,y) / 8.0f - momv(x,y)*momv(x,y)/(8.0f*9.0f);
    //Confirmed error < 2e-9; absolute values peak at around 1e-3

    //Neighborhood variance of nu
    Func SIGMAnu_h, SIGMAnu_v;
    RDom r3(-4,9);
    SIGMAnu_h(x,y) = 0.0f;
    SIGMAnu_h(x,y) += (X_h(r3+x,y) - Y_h(r3+x,y))*(X_h(r3+x,y) - Y_h(r3+x,y)) / DOMDIV;
    SIGMAnu_v(x,y) = 0.0f;
    SIGMAnu_v(x,y) += (X_v(x,r3+y) - Y_v(x,r3+y))*(X_v(x,r3+y) - Y_v(x,r3+y)) / DOMDIV;

    //LMMSE estimation in each direction
    Func Xlmmse_h, Xlmmse_v;
    Xlmmse_h(x,y) = MUx_h(x,y) +
        (Y_h(x,y) - MUx_h(x,y)) * SIGMAx_h(x,y) /
        (SIGMAx_h(x,y) + SIGMAnu_h(x,y) + 1e-7f);
    Xlmmse_v(x,y) = MUx_v(x,y) +
        (Y_v(x,y) - MUx_v(x,y)) * SIGMAx_v(x,y) /
        (SIGMAx_v(x,y) + SIGMAnu_v(x,y) + 1e-7f);

    //Confirmed to be correct? ======================================================

    //The expected estimation error is Xerror = X - Xlmmse
    //We don't use it.

    //The variance of the estimation error, we do use for the weighting.
    Func SIGMAer_h, SIGMAer_v;
    SIGMAer_h(x,y) = SIGMAx_h(x,y) - SIGMAx_h(x,y)*SIGMAx_h(x,y)/(SIGMAx_h(x,y) + SIGMAnu_h(x,y) + 1e-7f);
    SIGMAer_v(x,y) = SIGMAx_v(x,y) - SIGMAx_v(x,y)*SIGMAx_v(x,y)/(SIGMAx_v(x,y) + SIGMAnu_v(x,y) + 1e-7f);

    //Weight of estimate
    Func W_h, W_v;
    W_h(x,y) = SIGMAer_v(x,y) / (SIGMAer_h(x,y) + SIGMAer_v(x,y) + 1e-7f);
    W_v(x,y) = 1.0f - W_h(x,y);

    //slightly different =============================================

    //Combine to get the final log of the color ratio we'll use
    Func X;
    X(x,y) = W_h(x,y)*Xlmmse_h(x,y) + W_v(x,y)*Xlmmse_v(x,y);

    //Not Yet Scheduled
    //X.compute_root();

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
    //It seems like it may be necessary.
    //Let's try it.

    //First we must combine these ratios into one.
    Func grLogRatio, gbLogRatio;
    grLogRatio(x,y) = select(y%2 == 0,
            select(x%2 == 0,
                //Green pixel in red row
                grLogRatioAtGR(x/2,y/2),
                //Red pixel
                grLogRatioAtR(x/2,y/2)),
            select(x%2 == 0,
                //Blue pixel
                grLogRatioAtB(x/2,y/2),
                //Green pixel in blue row
                grLogRatioAtGB(x/2,y/2)));
    gbLogRatio(x,y) = select(y%2 == 0,
            select(x%2 == 0,
                //Green pixel in red row
                gbLogRatioAtGR(x/2,y/2),
                //Red pixel
                gbLogRatioAtR(x/2,y/2)),
            select(x%2 == 0,
                //Blue pixel
                gbLogRatioAtB(x/2,y/2),
                //Green pixel in blue row
                gbLogRatioAtGB(x/2,y/2)));


    //Turn the logs back into ratios, after the median filter
    Func grRatio, gbRatio;

    //No median, but do turn the logs back into ratios.
    grRatio(x,y) = exp(grLogRatio(x,y));
    gbRatio(x,y) = exp(gbLogRatio(x,y));



    //Output the values we want.
    output(x,y,c) = select(c == 0,
            //Red channel
            select(y%2 == 0,
                select(x%2 == 0,
                    //Green pixel in red row
                    g_gr(x/2,y/2) / grRatio(x,y),
                    //Red pixel
                    r_r(x/2,y/2)),
                select(x%2 == 0,
                    //Blue pixel
                    b_b(x/2,y/2) * gbRatio(x,y) / grRatio(x,y),
                    //Green pixel in blue row
                    g_gb(x/2,y/2) / grRatio(x,y))),
            select(c == 1,
                //Green channel
                select(y%2 == 0,
                    select(x%2 == 0,
                        //Green pixel in red row
                        g_gr(x/2,y/2),
                        //Red pixel
                        r_r(x/2,y/2) * grRatio(x,y)),
                    select(x%2 == 0,
                        //Blue pixel
                        b_b(x/2,y/2) * gbRatio(x,y),
                        //Green pixel in blue row
                        g_gb(x/2,y/2))),
                //Blue channel
                select(y%2 == 0,
                    select(x%2 == 0,
                        //Green pixel in red row
                        g_gr(x/2,y/2) / gbRatio(x,y),
                        //Red pixel
                        r_r(x/2,y/2) * grRatio(x,y) / gbRatio(x,y)),
                    select(x%2 == 0,
                        //Blue pixel
                        b_b(x/2,y/2),
                        //Green pixel in blue row
                        g_gb(x/2,y/2) / gbRatio(x,y))))) - 0.01f;


    Y_h.compute_root().parallel(y);
    Y_v.compute_root().split(x,xo,xi,16).parallel(xo).vectorize(xi,8);
    X_h.compute_root().parallel(y);
    X_v.compute_root().split(x,xo,xi,16).parallel(xo).vectorize(xi,8);
    MUx_v.compute_root().split(x,xo,xi,16).parallel(xo).vectorize(xi,8);
    MUx_h.compute_root().parallel(y);
    SIGMAx_v.compute_root().split(x,xo,xi,16).parallel(xo).vectorize(xi,8);
    SIGMAx_h.compute_root().parallel(y);
    SIGMAnu_v.compute_root().split(x,xo,xi,16).parallel(xo).vectorize(xi,8);
    SIGMAnu_h.compute_root().parallel(y);
    //Xlmmse_v.split(x,xo,xi,16).parallel(xo).vectorize(xi,4);
    //SIGMAer_v.split(x,xo,xi,16).parallel(xo).vectorize(xi,4);
    //W_v.split(x,xo,xi,16).parallel(xo).vectorize(xi,4);

    //X.compute_root().parallel(y);
    grLogRatioAtR.store_at(output,tile_index).compute_at(grLogRatio,x);
    gbLogRatioAtB.store_at(output,tile_index).compute_at(gbLogRatio,x);
    gbLogRatioAtR.store_at(output,tile_index).compute_at(gbLogRatio,x);
    grLogRatioAtB.store_at(output,tile_index).compute_at(grLogRatio,x);
    grLogRatio.compute_at(output,tile_index);
    gbLogRatio.compute_at(output,tile_index);

    output.tile(x,y,xo,yo,xi,yi,64,64)
      .fuse(xo,yo,tile_index)
      .parallel(tile_index)
      .vectorize(xi,8)
      .bound(c,0,3).reorder(c,xi,yi,tile_index).unroll(c)
      .compute_root();//.compile_to_lowered_stmt("output_unrolledC.html",HTML);

    return output;

}

int main(int argc, char **argv)
{
    Var x, y, c;
    //Halide::Image<uint8_t> input = load<uint8_t>("000734_levels.png");
    //Halide::Image<uint8_t> input = load<uint8_t>("porcupine2.png");
    Halide::Image<uint8_t> input = load<uint8_t>("P1040567.png");
    //Halide::Image<uint8_t> input = load<uint8_t>("teensy.png");
    Func toFloat, toBayer, toDemosaic, toInt;
    toFloat(x,y,c) = cast<float>(input(x,y,c))/255.0f;
    toBayer = bayerize(BoundaryConditions::mirror_image(toFloat,0,input.width(),0,input.height(),0,3));
    //Halide::Image<float> bayer = toBayer.realize(input.width(),input.height(),3);
    //toBayer = bayerize(BoundaryConditions::constant_exterior(toFloat,0.0f,0,input.width(),0,input.height(),0,3));
    toDemosaic = demosaic(toBayer);
    toInt(x,y,c) = cast<uint8_t>(Halide::clamp(Halide::round(toDemosaic(x,y,c)*255.0f),0.0f,255.0f));
    toInt.compile_jit();
    timeval t1, t2;
    std::cout << "Finished compilation, starting processing" << std::endl;
    gettimeofday(&t1, NULL);
    Halide::Image<uint8_t> output  = toInt.realize(input.width(),input.height(),3);
    gettimeofday(&t2, NULL);
    std::cout<< "Finished processing in " <<
      float(t2.tv_sec - t1.tv_sec) + float(t2.tv_usec - t1.tv_usec)/1000000.0f <<
      " seconds" << std::endl;
    save(output,"000734_demosaiced7.png");
    //save(output,"porcupine_demosaiced2.png");
    //save(output,"P1040567-med-out.png");
    //save(output,"teensy_out.png");
    return 0;
}
