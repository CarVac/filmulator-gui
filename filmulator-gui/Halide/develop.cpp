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
    developed.parallel(y);
    std::vector<Argument> devArgs = developed.infer_arguments();
    developed.compile_to_file("develop",devArgs);
    return 0;
}
