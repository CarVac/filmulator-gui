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

int main(int argc, char **argv) {

    Param<float> reservoirConcentration;
    Param<float> reservoirThickness;
    Param<float> activeLayerThickness; 
    Param<float> filmArea;

    ImageParam devMoved(type_of<float>(),2);
    Func developerMoved = lambda(x,y,devMoved(x,y));
    Expr pixelsPerMillimeter = sqrt(devMoved.width()*devMoved.height()/filmArea);
    RDom r(0 ,devMoved.width(), 0, devMoved.height());
    Func sumD;
    sumD(x) = 0.0f;
    sumD(0) += developerMoved(r.x,r.y);
    Func newResConc;
    newResConc(x) = undef<float>();
    newResConc(0) = reservoirConcentration - sumD(0)*activeLayerThickness/(pow(pixelsPerMillimeter,2)*reservoirThickness);
    std::vector<Argument> newResConcArgs = newResConc.infer_arguments();
    newResConc.compile_to_file("calcReservoirConcentration",newResConcArgs);

    return 0;
}
