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
    Param<float> stepTime;
    Param<float> layerMixConst;
    Param<float> layerTimeDivisor;

    Func sumDx;
    Func layerMixed;
    Func initialDeveloperMirrored;
    ImageParam devConc(type_of<float>(),2);
    Func dDevelConc;
    Func developerConcentration = lambda(x,y,devConc(x,y));
    dDevelConc = calcLayerMix(developerConcentration, layerMixConst, stepTime,
                              layerTimeDivisor, reservoirConcentration);
    std::vector<Argument> ddcArgs = dDevelConc.infer_arguments();
    dDevelConc.compile_to_file("calcLayerMix",ddcArgs);

    return 0;
}
