#include <stdio.h>
#include <Halide.h>
#include <sys/time.h>

using namespace Halide;
using namespace Halide::BoundaryConditions;

Var x, y;

// Sum an image vertically using a box filter with a zero boundary condition.
Func downscale(Func input, Expr radius, Expr width, Expr height) {

    Func b,c,d;
    // Accumulate as a uint16
    b(x, y) = cast<float>(0);
    c(x, y) = cast<float>(0);

    // The first value is the sum over pixels [0, radius]
    RDom r(-radius, radius);
    b(x,y) += input(x + r - radius,y);
    c(x,y) += b(x,y + r - radius);

    Expr denominator = pow(2*radius+1,2);

    d(x,y) = c(x, y)/denominator;   
    Var xo,xi;
    //d.bound(x,0,ceil(width/denominator)).bound(y,0,ceil(height/denominator));
    b.compute_root().gpu_tile(x,y,16,16);
    //d.reorder(y,x).gpu_tile(x,y,16,16);
    return d;
}

Func upscale(Func input, Expr radius){
    Func output;
    
    Expr scaleFactor = (2*radius+1);
    Expr xf = (x % scaleFactor)/cast<float>(scaleFactor);
    Expr yf = (y % scaleFactor)/cast<float>(scaleFactor);
    Expr x0 = cast<int>(floor(x/scaleFactor));
    Expr y0 = cast<int>(floor(y/scaleFactor));
    Expr x1 = cast<int>( ceil(x/scaleFactor));
    Expr y1 = cast<int>( ceil(x/scaleFactor));
    Expr Ix0y0 = input(x0,y0);
    Expr Ix0y1 = input(x0,y1);
    Expr Ix1y0 = input(x1,y0);
    Expr Ix1y1 = input(x1,y1);
    output(x,y) = lerp(lerp(Ix0y0,Ix1y0,xf),
                       lerp(Ix0y1,Ix1y1,xf),yf);
    return output;
}

Func gaussBlur(Func input){
    Func blurx,blury;
    blurx(x, y) = (input(x-2, y) +
                   input(x-1, y)*4 +
                   input(x  , y)*6 +
                   input(x+1, y)*4 +
                   input(x+2, y));
    blury(x, y) = (blurx(x, y-2) +
                   blurx(x, y-1)*4 +
                   blurx(x, y  )*6 +
                   blurx(x, y+1)*4 +
                   blurx(x, y+2));
    return blury;
}

int main(int argc, char **argv) {

    ImageParam input(UInt(8), 2);
    Param<int> radius;
    Expr denominator = cast<uint8_t>(radius * 2 + 1);

    // radius can be at most 127 before the sums overflow
    radius.set_range(0, 127);

    Func in = mirror_interior(input);

    Func downscaled;
    downscaled = downscale(in,150,input.width(), input.height());

    Func blurred_small;
    blurred_small = gaussBlur(downscaled);
    
    Func blurred_float;
    blurred_float = upscale(blurred_small,150);

    Func blurred;
    blurred(x,y) = cast<uint8_t>(blurred_float(x,y));

    Target target = get_target_from_environment();
    std::cout << target.to_string() << std::endl;
    if(target.has_gpu_feature())
    {
      downscaled.compute_root().gpu_tile(x,y,16,16);
      blurred_small.compute_root().gpu_tile(x,y,16,16);
      blurred.gpu_tile(x,y,16,16);

      Target target = get_host_target();
      target.set_feature(Target::CUDA);
      //target.set_feature(Target::GPUDebug);
      blurred.compile_jit(target);
    }
    else
    {
        /*blur_in_x.compute_root().vectorize(x, 8).split(x, xo, xi, 2).reorder(xi, y, xo).parallel(xo).unroll(xi);
        // It makes sense to explicitly compute the transpose so that the
        // blur can be done using dense vector loads.
        transpose_x.compute_at(blur_in_x, xo).vectorize(x, 8).unroll(x);
        sum_in_x.compute_at(blur_in_x, xo);
        for (int stage = 0; stage < 5; stage++) {
            sum_in_x.update(stage).vectorize(x, 8).unroll(x);
            if (stage > 0) {
                RVar r = sum_in_x.reduction_domain(stage).x;
                sum_in_x.update(stage).reorder(x, r);
            }
        }


        blur_in_y.compute_root().vectorize(x, 8).split(x, xo, xi, 2).reorder(xi, y, xo).parallel(xo).unroll(xi);
        transpose_y.compute_at(blur_in_y, xo).vectorize(x, 16).unroll(x);
        sum_in_y.compute_at(blur_in_y, xo);
        for (int stage = 0; stage < 5; stage++) {
            sum_in_y.update(stage).vectorize(x, 8).unroll(x);
            if (stage > 0) {
                RVar r = sum_in_y.reduction_domain(stage).x;
                sum_in_y.update(stage).reorder(x, r);
            }
        }*/
    }

    // Dump the assembly for inspection.
    //blur_in_y.compile_to_assembly("/dev/stdout", Internal::vec<Argument>(input, radius), "box_blur");

    // Save the output. Comment this out for benchmarking - it takes
    // way more time than the actual algorithm. Should look like a
    // blurry circle.
    //blur_in_y.debug_to_file("output.tiff");

    // Set some test parameters
    radius.set(19);

    // Make a test input image of a circle.
    Image<uint8_t> input_image(4000, 4000);
    lambda(x, y, select(((x - 500)*(x - 500) + (y - 500)*(y - 500)) < 100*100, cast<uint8_t>(255), cast<uint8_t>(0))).realize(input_image);
    input.set(input_image);

    Image<uint8_t> out(4000, 4000);
    // Realize it once to trigger compilation.
    blurred.realize(out);
    timeval t1, t2;
    gettimeofday(&t1, NULL);
    float numIter = 50;
    for (size_t i = 0; i < numIter; i++) {
        // Realize 100 more times for timing.
        blurred.realize(out);
    }
    gettimeofday(&t2, NULL);

    int64_t dt = t2.tv_sec - t1.tv_sec;
    dt *= 1000;
    dt += (t2.tv_usec - t1.tv_usec) / 1000;
    printf("%0.4f ms\n", dt / numIter);

    /*std::vector<Argument> arguments;
    arguments.push_back(input_image);
    arguments.push_back(radius);
    arguments.push_back(radius);
    blur_in_y.compile_to_c("gradient.cpp", arguments, "attachment");*/
    printf("Success!\n");
    return 0;
}


