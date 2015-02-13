#include <Halide.h>
#include "halideFilmulate.h"

using namespace Halide;

Var x,y,c;

int main(int argc, char **argv){

    ImageParam input(type_of<float>(),3);
    Func in = lambda(x,y,c,input(x,y,c));
    Func outputImage;
    outputImage(x,y,c) = undef<uint8_t>();
    outputImage(x,y,0) = cast<uint8_t>(min((1000.0f*256.0f*pow(in(x,y,CRYSTAL_RAD_R),2)*
                                       in(x,y,ACTIVE_CRYSTALS_R)),255.0f));
    outputImage(x,y,1) = cast<uint8_t>(min((1000.0f*256.0f*pow(in(x,y,CRYSTAL_RAD_G),2)*
                                       in(x,y,ACTIVE_CRYSTALS_G)),255.0f));
    outputImage(x,y,2) = cast<uint8_t>(min((1000.0f*256.0f*pow(in(x,y,CRYSTAL_RAD_B),2)*
                                       in(x,y,ACTIVE_CRYSTALS_B)),255.0f));

    std::vector<Argument> args(1);
    args[0] = input;
    outputImage.compile_to_file("generateFilmulatedImage",args);
    return 0;
}
