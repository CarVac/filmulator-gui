#include <Halide.h>
#include "halideFilmulate.h"

using namespace Halide;

Var x,y,c;

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


int main(int argc, char **argv){

    Param<float> crystalsPerPixel;
    Param<float> rolloffBoundary;
    Param<float> initialCrystalRadius;
    Param<float> initialSilverSaltDensity;
    Param<float> reservoirConcentration;

    ImageParam input(type_of<float>(), 3);
    Func in = lambda(x, y, c, 65535.0f*input(x, y, c));;

    Func activeCrystalsPerPixel;
    activeCrystalsPerPixel = exposure(in, crystalsPerPixel, rolloffBoundary);

    Func filmulationData;
    filmulationData(x,y,c) = cast<float>(0);
    filmulationData(x,y,CRYSTAL_RAD_R) = initialCrystalRadius;
    filmulationData(x,y,CRYSTAL_RAD_G) = initialCrystalRadius;
    filmulationData(x,y,CRYSTAL_RAD_B) = initialCrystalRadius;
    filmulationData(x,y,ACTIVE_CRYSTALS_R) = activeCrystalsPerPixel(x,y,0);
    filmulationData(x,y,ACTIVE_CRYSTALS_G) = activeCrystalsPerPixel(x,y,1);
    filmulationData(x,y,ACTIVE_CRYSTALS_B) = activeCrystalsPerPixel(x,y,2);
    filmulationData(x,y,SILVER_SALT_DEN_R) = initialSilverSaltDensity;
    filmulationData(x,y,SILVER_SALT_DEN_G) = initialSilverSaltDensity;
    filmulationData(x,y,SILVER_SALT_DEN_B) = initialSilverSaltDensity;
    filmulationData(x,y,DEVEL_CONC) = reservoirConcentration;

    std::vector<Argument> args = filmulationData.infer_arguments();
    filmulationData.compile_to_file("setupFilmulator",args);
    return 0;
}
