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

    ImageParam devConc(type_of<float>(),2);
    Func developerConcentration = lambda(x,y,devConc(x,y));

    ImageParam devMoved(type_of<float>(),2);
    Func developerMoved = lambda(x,y,devMoved(x,y));

    ImageParam filmData(type_of<float>(), 3);
    Func filmulationData = lambda(x,y,c,filmData(x,y,c));

    Func filmulationDataOut;
    filmulationDataOut(x,y,c) = undef<float>(); //filmulationData(x,y,c);
    filmulationDataOut(x,y,DEVEL_CONC) = developerMoved(x,y) + developerConcentration(x,y);
    std::vector<Argument> combArgs = filmulationDataOut.infer_arguments();
    filmulationDataOut.compile_to_file("performLayerMix",combArgs);

    return 0;
}
