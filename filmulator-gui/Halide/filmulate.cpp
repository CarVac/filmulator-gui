#include <stdio.h>
#include <Halide.h>
#include <sys/time.h>

using namespace Halide;
//using namespace Halide::BoundaryConditions;
using namespace std;

using Halide::Image;
#include "image_io.h"

#define CRYSTAL_RAD_R 0
#define CRYSTAL_RAD_G 1
#define CRYSTAL_RAD_B 2
#define ACTIVE_CRYSTALS_R 3
#define ACTIVE_CRYSTALS_G 4
#define ACTIVE_CRYSTALS_B 5
#define SILVER_SALT_DEN_R 6
#define SILVER_SALT_DEN_G 7
#define SILVER_SALT_DEN_B 8
#define DEVEL_CONC 9

Var x, y, c;

Func develop(Func inputs, Expr crystalGrowthConst,
             Expr activeLayerThickness, Expr developerConsumptionConst,
             Expr silverSaltConsumptionConst, Expr timestep) {

  Expr cgc = crystalGrowthConst*timestep;
  Expr dcc = 2.0*developerConsumptionConst / ( activeLayerThickness*3.0);
  Expr sscc = silverSaltConsumptionConst * 2.0;

  Expr dCrystalRadR = inputs(x,y,DEVEL_CONC) * inputs(x,y,SILVER_SALT_DEN_R) * cgc;
  Expr dCrystalRadG = inputs(x,y,DEVEL_CONC) * inputs(x,y,SILVER_SALT_DEN_G) * cgc;
  Expr dCrystalRadB = inputs(x,y,DEVEL_CONC) * inputs(x,y,SILVER_SALT_DEN_B) * cgc;

  Expr dCrystalVolR = dCrystalRadR * inputs(x,y,CRYSTAL_RAD_R) * inputs(x,y,CRYSTAL_RAD_R) *
                      inputs(x,y,ACTIVE_CRYSTALS_R);
  Expr dCrystalVolG = dCrystalRadG * inputs(x,y,CRYSTAL_RAD_G) * inputs(x,y,CRYSTAL_RAD_G) *
                      inputs(x,y,ACTIVE_CRYSTALS_G);
  Expr dCrystalVolB = dCrystalRadB * inputs(x,y,CRYSTAL_RAD_B) * inputs(x,y,CRYSTAL_RAD_B) *
                      inputs(x,y,ACTIVE_CRYSTALS_B);

  Func outputs;
  outputs(x,y,c) = cast<float>(0);
  outputs(x,y,CRYSTAL_RAD_R) = inputs(x,y,CRYSTAL_RAD_R) + dCrystalRadR;
  outputs(x,y,CRYSTAL_RAD_G) = inputs(x,y,CRYSTAL_RAD_G) + dCrystalRadG;
  outputs(x,y,CRYSTAL_RAD_B) = inputs(x,y,CRYSTAL_RAD_B) + dCrystalRadB;

  outputs(x,y,DEVEL_CONC) = max(0,inputs(x,y,DEVEL_CONC) - dcc*(dCrystalVolR + dCrystalVolG + dCrystalVolB));

  outputs(x,y,SILVER_SALT_DEN_R) = inputs(x,y,SILVER_SALT_DEN_R) - sscc*dCrystalVolR;
  outputs(x,y,SILVER_SALT_DEN_G) = inputs(x,y,SILVER_SALT_DEN_G) - sscc*dCrystalVolG;
  outputs(x,y,SILVER_SALT_DEN_B) = inputs(x,y,SILVER_SALT_DEN_B) - sscc*dCrystalVolB;

  outputs(x,y,ACTIVE_CRYSTALS_R) = inputs(x,y,ACTIVE_CRYSTALS_R);
  outputs(x,y,ACTIVE_CRYSTALS_G) = inputs(x,y,ACTIVE_CRYSTALS_G);
  outputs(x,y,ACTIVE_CRYSTALS_B) = inputs(x,y,ACTIVE_CRYSTALS_B);

  return outputs;
}

Func exposure(Func input, Expr crystals_per_pixel, Expr rolloff_boundary){
  Expr crystal_headroom = 65535.0f - rolloff_boundary;
  Func rolloff;
  rolloff(x,y,c) = select(input(x,y,c) > rolloff_boundary,
                          65535.0f - crystal_headroom*crystal_headroom/
                            (input(x,y,c) + crystal_headroom-rolloff_boundary),
                          input(x,y,c));
  Func output;
  output(x,y,c) = rolloff(x,y,c)*crystals_per_pixel*0.00015387105f;
  return output;
}

Func downscale(Func input, Expr radius) {

    Func horizontalSum,verticalSum,average;
    // Accumulate as a uint16
    horizontalSum(x, y) = cast<float>(0);
    verticalSum(x, y) = cast<float>(0);

    // The first value is the sum over pixels [0, radius]
    RDom r(-radius,2*radius+1);
    horizontalSum(x,y) += input(cast<int>((2*radius+1)*x + r),cast<int>(y));
    verticalSum(x,y) += horizontalSum(cast<int>(x),cast<int>((2*radius+1)*y + r));

    Expr denominator = pow(2*radius+1,2);

    average(x,y) = verticalSum(x, y)/denominator;
    Var xo,xi;
    //d.bound(x,0,ceil(width/denominator)).bound(y,0,ceil(height/denominator));
    horizontalSum.compute_root();//.gpu_tile(x,y,16,16);
    verticalSum.compute_root();
    //d.reorder(y,x).gpu_tile(x,y,16,16);
    return average;
}

Func upscale(Func input, Expr radius){
    Func output;
    
    Expr scaleFactor = cast<float>(2*radius+1);
    Expr xf = (x % scaleFactor)/scaleFactor;
    Expr yf = (y % scaleFactor)/scaleFactor;
    Expr x0 = cast<int>(floor(cast<float>(x)/scaleFactor));
    Expr y0 = cast<int>(floor(cast<float>(y)/scaleFactor));
    Expr x1 = cast<int>( ceil(cast<float>(x)/scaleFactor));
    Expr y1 = cast<int>( ceil(cast<float>(y)/scaleFactor));
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
    Func blur;
    blurx.compute_root();
    blur(x,y) = blury(x,y)/(16*16);
    return blur;
}

Func diffuse(Func input, Expr sigma_const, Expr pixels_per_millimeter, Expr timestep){

    Expr sigma = sqrt(timestep*pow(sigma_const*pixels_per_millimeter,2));
    Expr radius = round(sigma/2);
    Func downscaled;
    downscaled = downscale(input,radius);

    Func blurred_small;
    blurred_small = gaussBlur(downscaled);

    Func blurred_float;
    blurred_float = upscale(blurred_small,radius);

    Func blurred;
    blurred(x,y) = blurred_float(x,y);

    downscaled.compute_root();//.gpu_tile(x,y,16,16);
    blurred_small.compute_root();//.gpu_tile(x,y,16,16);
    blurred.compute_root();
    //blured.gpu_tile(x,y,16,16);
    return blurred;
}

Func calcLayerMix(Func developer_concentration, Expr layer_mix_const, Expr timestep,
                  Expr layer_time_divisor, Expr reservoir_developer_concentration){
    Expr layer_mix = pow(layer_mix_const,timestep/layer_time_divisor);
    
    Expr reservoir_portion = (1 - layer_mix) * reservoir_developer_concentration;

    Func output;
    output(x,y) = developer_concentration(x,y) * (layer_mix - 1) + reservoir_portion;
    return output;
}

int main(int argc, char **argv) {

    ImageParam input(UInt(8), 3);
    
    Func inInt = lambda(x, y, c, input(x, y, c));;
    Func in;
    in(x,y,c) = cast<float>(256.0*inInt(x,y,c));
    Func activeCrystalsPerPixel;
    activeCrystalsPerPixel = exposure(in, 500, 51275);

    Func filmulationData[12];
    filmulationData[0](x,y,c) = cast<float>(0);
    filmulationData[0](x,y,CRYSTAL_RAD_R) = 0.00001;
    filmulationData[0](x,y,CRYSTAL_RAD_G) = 0.00001;
    filmulationData[0](x,y,CRYSTAL_RAD_B) = 0.00001;
    filmulationData[0](x,y,ACTIVE_CRYSTALS_R) = activeCrystalsPerPixel(x,y,0);
    filmulationData[0](x,y,ACTIVE_CRYSTALS_G) = activeCrystalsPerPixel(x,y,1);
    filmulationData[0](x,y,ACTIVE_CRYSTALS_B) = activeCrystalsPerPixel(x,y,2);
    filmulationData[0](x,y,SILVER_SALT_DEN_R) = 1.0;
    filmulationData[0](x,y,SILVER_SALT_DEN_G) = 1.0;
    filmulationData[0](x,y,SILVER_SALT_DEN_B) = 1.0;
    filmulationData[0](x,y,DEVEL_CONC) = 1.0;

    Expr reservoirConc[12];
    reservoirConc[0]  = 1.0;
    Func sumD[12];
    Func diffused[12];
    Func developed[12];
    Func initialDeveloper[12];
    Func dDevelConc[12];
    Func sumDx[12];
    Func layerMixed[12];
    Func initialDeveloperMirrored[12];
    
    Expr crystalGrowthConst = 0.00001;
    Expr activeLayerThickness = 0.1;
    Expr developerConsumptionConst = 2000000.0;
    Expr silverSaltConsumptionConst = 2000000.0;
    Expr sigmaConst = 0.2;
    Expr pixelsPerMillimeter = sqrt(input.width()*input.height()/864);
    Expr layerMixConst = 0.2;
    Expr layerTimeDivisor = 20.0;
    Expr totalTime = 100.0;
    Expr numSteps = 11.0;
    Expr reservoirThickness = 1000.0;

    for(int i = 1; i < 12; i ++)
    {
      developed[i] = develop(filmulationData[i-1], crystalGrowthConst, activeLayerThickness,
                             developerConsumptionConst, silverSaltConsumptionConst, totalTime/numSteps);
      developed[i].compute_root();

      //std::pair<Expr,Expr> xDim(0,input.width());
      //std::pair<Expr,Expr> yDim(0,input.height());
      //std::vector<std::pair<Expr,Expr> > dimensions(xDim,yDim);
      initialDeveloper[i](x,y) = developed[i](x,y,DEVEL_CONC);
      initialDeveloperMirrored[i] = BoundaryConditions::mirror_interior(initialDeveloper[i],0,input.width(),0,input.height());
      diffused[i] = diffuse(initialDeveloperMirrored[i],sigmaConst,pixelsPerMillimeter, totalTime/numSteps);
      diffused[i].compute_root();

      dDevelConc[i] = calcLayerMix(diffused[i], layerMixConst, totalTime/numSteps, layerTimeDivisor, reservoirConc[i-1]);
      RDom r(0 ,input.width(), 0, input.height());
      sumD[i](x) = 0.0;
      sumD[i](0) += dDevelConc[i](r.x,r.y);
      sumD[i].compute_root();
      reservoirConc[i] = reservoirConc[i-1] - sumD[i](0)*activeLayerThickness/
                                                         (pow(pixelsPerMillimeter,2)*reservoirThickness);
      layerMixed[i](x,y) = diffused[i](x,y) + dDevelConc[i](x,y);

      filmulationData[i](x,y,c) = developed[i](x,y,c);
      filmulationData[i](x,y,DEVEL_CONC) = layerMixed[i](x,y);
    }

    Func outputImage;
    int endIter = 11;
    
    outputImage(x,y,c) = cast<uint8_t>(0);
    outputImage(x,y,0) = cast<uint8_t>(1000.0*256.0*pow(filmulationData[endIter](x,y,CRYSTAL_RAD_R),2)*
                                       filmulationData[endIter](x,y,ACTIVE_CRYSTALS_R));
    outputImage(x,y,1) = cast<uint8_t>(1000.0*256.0*pow(filmulationData[endIter](x,y,CRYSTAL_RAD_G),2)*
                                       filmulationData[endIter](x,y,ACTIVE_CRYSTALS_G));
    outputImage(x,y,2) = cast<uint8_t>(1000.0*256.0*pow(filmulationData[endIter](x,y,CRYSTAL_RAD_B),2)*
                                       filmulationData[endIter](x,y,ACTIVE_CRYSTALS_B));
    
    //outputImage(x,y,c) = cast<uint8_t>(100.0*filmulationData[3](x,y,ACTIVE_CRYSTALS_R));
    Target target = get_target_from_environment();
    std::cout << target.to_string() << std::endl;
    if(target.has_gpu_feature())
    {
      Target target = get_host_target();
      target.set_feature(Target::CUDA);
      //target.set_feature(Target::GPUDebug);
      outputImage.compile_jit(target);
    }
    else
    {
        /*blur_in_x.compute_root().vectorize(x, 8).split(x, xo, xi, 2).reorder(xi, y, xo).parallel(xo).unroll(xi);
        // It makes sense to explicitly compute the transpose so that the
        // blur can be done using dense vector loads
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
    outputImage.compile_to_c("compiled.c", Internal::vec<Argument>(input), "outputImage");

    // Save the output. Comment this out for benchmarking - it takes
    // way more time than the actual algorithm. Should look like a
    // blurry circle.
    //blurred.debug_to_file("output.tiff");

    // Make a test input image of a circle.
    Image<uint8_t> input_image = load<uint8_t>("P1040567.png");
    input.set(input_image);

    //Buffer out(UInt(8), 4768, 3184,3);
    Buffer out(UInt(8), 47, 31,3);
    Image<uint8_t> outImage(input_image.width(), input_image.height(),3);
    // Realize it once to trigger compilation.
    outputImage.realize(outImage);
    save(outImage, "output.png");
    timeval t1, t2;
    gettimeofday(&t1, NULL);
    float numIter = 1;
    for (size_t i = 0; i < numIter; i++) {
        // Realize numIter more times for timing.
        outputImage.realize(out);
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



