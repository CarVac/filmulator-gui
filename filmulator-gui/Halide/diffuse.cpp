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

Func performBlur(Func f, Func coeff, Expr size, Expr sigma) {
    Func blurred;
    blurred(x, y) = undef<float>();

    Expr padding = cast<int>(ceil(3*sigma) + 3);

    // warm up
    blurred(x, 0-padding) =  coeff(0) * f(x, 0);
    blurred(x, 1-padding) = (coeff(0) * f(x, 1) +
                             coeff(1) * blurred(x, 0));
    blurred(x, 2-padding) = (coeff(0) * f(x, 2) +
                             coeff(1) * blurred(x, 1) +
                             coeff(2) * blurred(x, 0));

    // top to bottom
    RDom fwd(3-padding, size - 3+padding);
    blurred(x, fwd) = (coeff(0) * f(x, fwd) +
                       coeff(1) * blurred(x, fwd - 1) +
                       coeff(2) * blurred(x, fwd - 2) +
                       coeff(3) * blurred(x, fwd - 3));

    // tail end
    RDom tail(size, padding);
    blurred(x, tail) = (coeff(0) * f(x,tail) +
                        coeff(1) * blurred(x, tail - 1) +
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

    // Crashes on first 3 RDoms because this version of Halide doesn't
    // allow for assigning an invalid Rdom to r.
    for (int i = 3; i < blurred.num_update_definitions(); i++) {
        RDom r = blurred.reduction_domain(i);
        if (r.defined()) {
            blurred.update(i).reorder(x, r);
        }
    }
    for (int i = 0; i < blurred.num_update_definitions(); i++) {
        blurred.update(i).vectorize(x, 8).unroll(x);
    }

    return out;
}

Func blur(Func input, Expr sigma, Expr width, Expr height) {

    // compute iir coefficients using the method of young and van vliet.
    Func coeff;
    Expr q = select(sigma < 2.5f,
                    3.97156f - 4.14554f*sqrt(1 - 0.26891f*sigma),
                    0.98711f*sigma - 0.96330f);
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

int main(int argc, char **argv) {

    Param<float> stepTime;
    Param<float> filmArea;
    Param<float> sigmaConst;

    ImageParam input(type_of<float>(), 3);
    Func filmulationData = lambda(x,y,c,input(x,y,c));

    Expr pixelsPerMillimeter = sqrt(input.width()*input.height()/filmArea);
    Func initialDeveloper;
    initialDeveloper(x,y) = filmulationData(x,y,DEVEL_CONC);
    Func initialDeveloperMirrored;
    initialDeveloperMirrored = BoundaryConditions::mirror_interior(initialDeveloper,input.width(),0,input.height(),0);
    Func diffused;
    diffused = diffuse(initialDeveloperMirrored,sigmaConst,pixelsPerMillimeter, stepTime,
                          input.width(), input.height());
    std::vector<Argument> diffArgs = diffused.infer_arguments();
    diffused.compile_to_file("diffuse", diffArgs);
    return 0;
}
