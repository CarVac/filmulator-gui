#include <stdio.h>
#include <Halide.h>
#include <sys/time.h>

using namespace Halide;
//using namespace Halide::BoundaryConditions;
using namespace std;

using Halide::Image;
#include "image_io.h"
#include "halideFilmulate.h"

Var x, y, c;

Func develop(Func inputs, Expr crystalGrowthConst,
             Expr activeLayerThickness, Expr developerConsumptionConst,
             Expr silverSaltConsumptionConst, Expr timestep) {

  Expr cgc = crystalGrowthConst*timestep;
  Expr dcc = 2.0f*developerConsumptionConst / ( activeLayerThickness*3.0f);
  Expr sscc = silverSaltConsumptionConst * 2.0f;

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
  outputs(x,y,c) = undef<float>();
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

Func performBlur(Func f, Func coeff, Expr size, Expr sigma) {
    Func blurred;
    blurred(x, y) = undef<float>();

    // warm up
    blurred(x, 0) = coeff(0) * f(x, 0);
    blurred(x, 1) = (coeff(0) * f(x, 1) +
                     coeff(1) * blurred(x, 0));
    blurred(x, 2) = (coeff(0) * f(x, 2) +
                     coeff(1) * blurred(x, 1) +
                     coeff(2) * blurred(x, 0));

    // top to bottom
    RDom fwd(3, size - 3);
    blurred(x, fwd) = (coeff(0) * f(x, fwd) +
                       coeff(1) * blurred(x, fwd - 1) +
                       coeff(2) * blurred(x, fwd - 2) +
                       coeff(3) * blurred(x, fwd - 3));

    // tail end
    Expr padding = cast<int>(ceil(4*sigma) + 3);
    RDom tail(size, padding);
    blurred(x, tail) = (coeff(1) * blurred(x, tail - 1) +
                        coeff(2) * blurred(x, tail - 2) +
                        coeff(3) * blurred(x, tail - 3));

    // bottom to top
    Expr last = size + padding - 1;
    RDom backwards(0, last - 2);
    Expr b = last - 3 - backwards; // runs from last - 3 down to zero
    blurred(x, b) = (coeff(0) * blurred(x, b) +
                     coeff(1) * blurred(x, b + 1) +
                     coeff(2) * blurred(x, b + 2) +
                     coeff(3) * blurred(x, b + 3));
    return blurred;
}

Func blur_then_transpose(Func f, Func coeff, Expr size, Expr sigma) {

    Func blurred = performBlur(f, coeff, size, sigma);
    //Func blurred = f;

    // also compute attenuation due to zero boundary condition by
    // blurring an image of ones in the same way. this gives a
    // boundary condition equivalent to reweighting the gaussian
    // near the edge. (todo: add a generator param to select
    // different boundary conditions).
    Func ones;
    ones(x, y) = 1.0f;
    Func attenuation = performBlur(ones, coeff, size, sigma);

    // invert the attenuation so we can multiply by it. the
    // attenuation is the same for every row/channel so we only
    // need one column.
    Func inverse_attenuation;
    inverse_attenuation(y) = 1.0f / attenuation(0, y);

    // transpose it
    Func transposed;
    transposed(x, y) = blurred(y, x);

    // correct for attenuation
    Func out;
    out(x, y) = transposed(x, y) * inverse_attenuation(x);

    // schedule it.
    Var yi, xi, yii, xii;

    attenuation.compute_root();
    inverse_attenuation.compute_root().vectorize(y, 8);
    out.compute_root()
        .tile(x, y, xi, yi, 8, 32)
        .tile(xi, yi, xii, yii, 8, 8)
        .vectorize(xii).unroll(yii).parallel(y);
    blurred.compute_at(out, y);
    transposed.compute_at(out, xi).vectorize(y).unroll(x);

    /*
    for (int i = 0; i < blurred.num_update_definitions(); i++) {
        RDom r = blurred.reduction_domain(i);
        if (r.defined()) {
            blurred.update(i).reorder(x, r);
        }
        blurred.update(i).vectorize(x, 8).unroll(x);
    }
    */

    return out;
}

Func blur(Func input, Expr sigma, Expr width, Expr height) {

    // compute iir coefficients using the method of young and van vliet.
    Func coeff;
    Expr q = select(sigma < 2.5f,
                    3.97156f - 4.14554f*sqrt(1 - 0.26891f*sigma),
                    q = 0.98711f*sigma - 0.96330f);
    Expr denom = 1.57825f + 2.44413f*q + 1.4281f*q*q + 0.422205f*q*q*q;
    coeff(x) = undef<float>();
    coeff(1) = (2.44413f*q + 2.85619f*q*q + 1.26661f*q*q*q)/denom;
    coeff(2) = -(1.4281f*q*q + 1.26661f*q*q*q)/denom;
    coeff(3) = (0.422205f*q*q*q)/denom;
    coeff(0) = 1 - (coeff(1) + coeff(2) + coeff(3));
    coeff.compute_root();

    Func f;
    f(x, y) = input(x, y);
    f = blur_then_transpose(f, coeff, height, sigma);
    f = blur_then_transpose(f, coeff, width, sigma);
    return f;
}

Func diffuse(Func input, Expr sigma_const, Expr pixels_per_millimeter, Expr timestep, Expr width, Expr height){

    Expr sigma = sqrt(timestep*pow(sigma_const*pixels_per_millimeter,2));
    Func blurred;
    blurred = blur(input,sigma,width,height);
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

    Param<float> reservoirConcentration;
    Param<float> reservoirThickness;
    Param<float> activeLayerThickness;
    Param<float> developerConsumptionConst;
    Param<float> crystalGrowthConst;
    Param<float> silverSaltConsumptionConst;
    Param<float> stepTime;
    Param<float> filmArea;
    Param<float> sigmaConst;
    Param<float> layerMixConst;
    Param<float> layerTimeDivisor;

    ImageParam input(type_of<float>(), 3);
    Func filmulationData = lambda(x,y,c,input(x,y,c));

    Func sumD;
    Func diffused;
    Func developed;
    Func initialDeveloper;
    Func dDevelConc;
    Func sumDx;
    Func layerMixed;
    Func initialDeveloperMirrored;

    developed = develop(filmulationData, crystalGrowthConst, activeLayerThickness,
                        developerConsumptionConst, silverSaltConsumptionConst,
                        stepTime);
    std::vector<Argument> devArgs = developed.infer_arguments();
    developed.compile_to_file("develop",devArgs);


    Expr pixelsPerMillimeter = sqrt(input.width()*input.height()/filmArea);
    initialDeveloper(x,y) = filmulationData(x,y,DEVEL_CONC);
    //initialDeveloperMirrored[i] = BoundaryConditions::mirror_interior(initialDeveloper[i],0,input.width(),0,input.height());
    diffused = diffuse(initialDeveloper,sigmaConst,pixelsPerMillimeter, stepTime,
                          input.width(), input.height());
    std::vector<Argument> diffArgs = diffused.infer_arguments();
    diffused.compile_to_file("diffuse", diffArgs);


    ImageParam devConc(type_of<float>(),2);
    Func developerConcentration = lambda(x,y,devConc(x,y));
    dDevelConc = calcLayerMix(developerConcentration, layerMixConst, stepTime,
                              layerTimeDivisor, reservoirConcentration);
    std::vector<Argument> ddcArgs = dDevelConc.infer_arguments();
    dDevelConc.compile_to_file("calcLayerMix",ddcArgs);


    ImageParam devMoved(type_of<float>(),2);
    Func developerMoved = lambda(x,y,devMoved(x,y));
    Expr pixelsPerMillimeterSumD = sqrt(devMoved.width()*devMoved.height()/filmArea);
    RDom r(0 ,devMoved.width(), 0, devMoved.height());
    sumD(x) = 0.0f;
    sumD(0) += developerMoved(r.x,r.y);
    sumD(0) = reservoirConcentration - sumD(0)*activeLayerThickness/(pow(pixelsPerMillimeterSumD,2)*reservoirThickness);
    std::vector<Argument> sumDArgs = sumD.infer_arguments();
    sumD.compile_to_file("calcReservoirConcentration",sumDArgs);


    Func filmulationDataOut;
    filmulationDataOut(x,y,c) = filmulationData(x,y,c);
    filmulationDataOut(x,y,DEVEL_CONC) = developerConcentration(x,y) + developerMoved(x,y);
    std::vector<Argument> combArgs = filmulationDataOut.infer_arguments();
    filmulationDataOut.compile_to_file("performLayerMix",combArgs);

    return 0;
}



