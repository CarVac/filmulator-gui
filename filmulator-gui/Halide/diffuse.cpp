Func performBlur(Func f, Func coeff, Expr size, Expr sigma) {
    Func blurred;
    blurred(x, y) = undef<float>();

    // Warm up
    blurred(x, 0) = coeff(0) * f(x, 0);
    blurred(x, 1) = (coeff(0) * f(x, 1) +
                     coeff(1) * blurred(x, 0));
    blurred(x, 2) = (coeff(0) * f(x, 2) +
                     coeff(1) * blurred(x, 1) +
                     coeff(2) * blurred(x, 0));

    // Top to bottom
    RDom fwd(3, size - 3);
    blurred(x, fwd) = (coeff(0) * f(x, fwd) +
                       coeff(1) * blurred(x, fwd - 1) +
                       coeff(2) * blurred(x, fwd - 2) +
                       coeff(3) * blurred(x, fwd - 3));

    // Tail end
    Expr padding = cast<int>(ceil(4*sigma) + 3);
    RDom tail(size, padding);
    blurred(x, tail) = (coeff(1) * blurred(x, tail - 1) +
                        coeff(2) * blurred(x, tail - 2) +
                        coeff(3) * blurred(x, tail - 3));

    // Bottom to top
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

    // Also compute attenuation due to zero boundary condition by
    // blurring an image of ones in the same way. This gives a
    // boundary condition equivalent to reweighting the Gaussian
    // near the edge. (TODO: add a generator param to select
    // different boundary conditions).
    Func ones;
    ones(x, y) = 1.0f;
    Func attenuation = performBlur(ones, coeff, size, sigma);

    // Invert the attenuation so we can multiply by it. The
    // attenuation is the same for every row/channel so we only
    // need one column.
    Func inverse_attenuation;
    inverse_attenuation(y) = 1.0f / attenuation(0, y);

    // Transpose it
    Func transposed;
    transposed(x, y) = blurred(y, x);

    // Correct for attenuation
    Func out;
    out(x, y) = transposed(x, y) * inverse_attenuation(x);

    // Schedule it.
    Var yi, xi, yii, xii;

    attenuation.compute_root();
    inverse_attenuation.compute_root().vectorize(y, 8);
    out.compute_root()
        .tile(x, y, xi, yi, 8, 32)
        .tile(xi, yi, xii, yii, 8, 8)
        .vectorize(xii).unroll(yii).parallel(y);
    blurred.compute_at(out, y);
    transposed.compute_at(out, xi).vectorize(y).unroll(x);

    for (int i = 0; i < blurred.num_update_definitions(); i++) {
        RDom r = blurred.reduction_domain(i);
        if (r.defined()) {
            blurred.update(i).reorder(x, r);
        }
        blurred.update(i).vectorize(x, 8).unroll(x);
    }

    return out;
}

Func blur(Func input, Expr sigma, Expr width, Expr height) {

    // Compute IIR coefficients using the method of Young and Van Vliet.
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

    Func blurY, blurX;
    blurY = blur_then_transpose(input, coeff, height, sigma);
    blurX = blur_then_transpose(blurY, coeff, width, sigma);
    return blurX;
}

Func diffuse(Func input, Expr sigma_const, Expr pixels_per_millimeter, Expr timestep, Expr width, Expr height){

    Expr sigma = sqrt(timestep)*sigma_const*pixels_per_millimeter;
//  The following causes bounds inference failure
//  Expr sigma = sqrt(timestep*pow(sigma_const*pixels_per_millimeter,2));
    Func diffused;
    diffused = blur(input,sigma,width,height);
    return diffused;
}
