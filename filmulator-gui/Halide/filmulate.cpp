#include <Halide.h>

#include <iostream>
using namespace std;

using namespace Halide;
using namespace Halide::BoundaryConditions;

#include "halideFilmulate.h"
Var x, y, c;

#include "develop.cpp"
#include "diffuse.cpp"

class filmulateIterationGenerator : public Halide::Generator<filmulateIterationGenerator> {
  public:

    Param<float> reservoirConcentration{"reservoirConcentration"};
    Param<float> reservoirThickness{"reservoirThickness"};
    Param<float> crystalGrowthConst{"crystalGrowthConst"};
    Param<float> activeLayerThickness{"activeLayerThickness"};
    Param<float> developerConsumptionConst{"developerConsumptionConst"};
    Param<float> silverSaltConsumptionConst{"silverSaltConsumptionConst"};
    Param<float> stepTime{"stepTime"};
    Param<float> filmArea{"filmArea"};
    Param<float> sigmaConst{"sigmaConst"};
    Param<float> layerMixConst{"layerMixConst"};
    Param<float> layerTimeDivisor{"layerTimeDivisor"};
    Param<bool> doDiffuse{"doDiffuse"};

    ImageParam input{Float(32), 3,"input"};

    Pipeline build() {
      Func filmulationData = lambda(x,y,c,input(x,y,c));

      Func developed;
      developed = develop(filmulationData, crystalGrowthConst, activeLayerThickness,
                          developerConsumptionConst, silverSaltConsumptionConst,
                          stepTime);
      developed.compute_root();

      Func diffused;
      Func initialDeveloper, initialDeveloperMirrored;
      initialDeveloper(x,y) = developed(x,y,DEVEL_CONC);
      initialDeveloperMirrored = BoundaryConditions::mirror_interior(initialDeveloper,0,input.width(),0,input.height());
      Expr pixelsPerMillimeter = sqrt(input.width()*input.height()/filmArea);
      diffused = diffuse(initialDeveloper,sigmaConst,pixelsPerMillimeter, stepTime,
                         input.width(), input.height());
      diffused.compute_root();

      Func developerFlux; //Developer moving from reservoir to active layer
      Expr layerMixCoef = pow(layerMixConst,stepTime/layerTimeDivisor);
      developerFlux(x,y) = (reservoirConcentration - diffused(x,y))*layerMixCoef;
      developerFlux.compute_root();

      Func layerMixed;
      layerMixed(x,y) = diffused(x,y) + developerFlux(x,y);
      layerMixed.compute_root();

      Func fluxSum; // Total developer moved in units of density*pixelVolume^3
      RDom r(0, input.width(), 0, input.height());
      fluxSum(x) = 0.0f;
      fluxSum(0) += developerFlux(r.x,r.y);
      fluxSum.compute_root();

      Func newReservoirConcentration;
      // Total developer moved in units of density*mm^3
      Expr totalFluxMM = fluxSum(0)*activeLayerThickness * 1/pow(pixelsPerMillimeter,2);
      Expr reservoirVolume = reservoirThickness*filmArea;
      Expr reservoirTotalDeveloper = reservoirVolume*reservoirConcentration;
      newReservoirConcentration(x) = (reservoirTotalDeveloper - totalFluxMM)/reservoirVolume;

      Func filmulationDataOut;
      filmulationDataOut(x,y,c) = select(c == DEVEL_CONC && doDiffuse == 1,
                                         layerMixed(x,y),
                                         developed(x,y,c));
      Func reservoirConcentrationOut;
      reservoirConcentrationOut(x) = select(doDiffuse == 1,
                                            newReservoirConcentration(x),
                                            reservoirConcentration);
      return Pipeline({filmulationDataOut,reservoirConcentrationOut});
    };
};

RegisterGenerator<filmulateIterationGenerator> filmulateIterationGenerator{"filmulateIterationGenerator"};


