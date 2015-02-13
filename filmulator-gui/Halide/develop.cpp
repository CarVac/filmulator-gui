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
  outputs(x,y,c) = select(
             c == CRYSTAL_RAD_R    , inputs(x,y,c) + dCrystalRadR,
      select(c == CRYSTAL_RAD_G    , inputs(x,y,c) + dCrystalRadG,
      select(c == CRYSTAL_RAD_B    , inputs(x,y,c) + dCrystalRadB,

      select(c == DEVEL_CONC       , max(0,inputs(x,y,DEVEL_CONC) -
                                     dcc*(dCrystalVolR + dCrystalVolG + dCrystalVolB)),

      select(c == SILVER_SALT_DEN_R, inputs(x,y,c) + sscc*dCrystalVolR,
      select(c == SILVER_SALT_DEN_G, inputs(x,y,c) + sscc*dCrystalVolG,
      select(c == SILVER_SALT_DEN_B, inputs(x,y,c) + sscc*dCrystalVolB,

      //Otherwise (active crystals) output=input
                                     inputs(x,y,c))))))));

  return outputs;
}


int main(int argc, char **argv) {

    Param<float> activeLayerThickness;
    Param<float> developerConsumptionConst;
    Param<float> crystalGrowthConst;
    Param<float> silverSaltConsumptionConst;
    Param<float> stepTime;

    ImageParam input(type_of<float>(), 3);
    Func filmulationData = lambda(x,y,c,input(x,y,c));

    Func developed;
    developed = develop(filmulationData, crystalGrowthConst, activeLayerThickness,
                        developerConsumptionConst, silverSaltConsumptionConst,
                        stepTime);
    Var x_outer, x_inner;
    developed.split(x,x_outer,x_inner,4).reorder(x_inner,c,x_outer,y)
             .vectorize(x_inner).parallel(y);
    std::vector<Argument> devArgs = developed.infer_arguments();
    developed.compile_to_file("develop",devArgs);
    developed.compile_to_lowered_stmt("develop.html");
    return 0;
}
