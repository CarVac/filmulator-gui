/*
 * This file is part of Filmulator.
 *
 * Copyright 2013 Omer Mano and Carlo Vaccari
 *
 * Filmulator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Filmulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Filmulator. If not, see <http://www.gnu.org/licenses/>
 */
#include "filmSim.hpp"

void exposure(matrix<float> &input_image,
              float          crystals_per_pixel,
              float          rolloff_bound,
              float          toe_bound,
              float          highlight_crosstalk) {
    rolloff_bound = std::max(std::min(rolloff_bound, 65534.f), 1.f);
    toe_bound =
      std::max(std::min(toe_bound, rolloff_bound / 2), 0.f);      // bound this to lower than half the rolloff boundary
    rolloff_bound = std::min(65535.f, rolloff_bound - toe_bound); // we mustn't let rolloff boundary exceed 65535
    const int   nrows = input_image.nr();
    const int   ncols = input_image.nc() / 3;
    const float max_crystals = 65535.f - toe_bound;
    const float crystal_headroom = max_crystals - rolloff_bound;
    float       crosstalkToe = rolloff_bound / 2;
    // Magic number mostly for historical reasons
    crystals_per_pixel *= 0.00015387105f;
#pragma omp parallel
    {
#pragma omp for schedule(dynamic) nowait
        for (int row = 0; row < nrows; row++) {
            for (int col = 0; col < ncols; col++) {
                const int   colr = col * 3;
                const int   colg = colr + 1;
                const int   colb = colr + 2;
                const float inputR = max(0.0f, input_image(row, colr));
                const float inputG = max(0.0f, input_image(row, colg));
                const float inputB = max(0.0f, input_image(row, colb));
                // crosstalk
                const float rsurp = max(0.0f, inputR - rolloff_bound);
                const float gsurp = max(0.0f, inputG - rolloff_bound);
                const float bsurp = max(0.0f, inputB - rolloff_bound);
                const float rSurplus =
                  rsurp - crosstalkToe + (crosstalkToe * crosstalkToe) / (rsurp + crosstalkToe + 1 / 65535.0f);
                const float gSurplus =
                  gsurp - crosstalkToe + (crosstalkToe * crosstalkToe) / (gsurp + crosstalkToe + 1 / 65535.0f);
                const float bSurplus =
                  bsurp - crosstalkToe + (crosstalkToe * crosstalkToe) / (bsurp + crosstalkToe + 1 / 65535.0f);
                const float maxSurplus = max(max(max(rSurplus, gSurplus), bSurplus), 0.000001f);
                const float sumSurplus = rSurplus + gSurplus + bSurplus;
                const float crosstalkR =
                  inputR + sumSurplus * highlight_crosstalk - (rSurplus * rSurplus / maxSurplus) * highlight_crosstalk;
                const float crosstalkG =
                  inputG + sumSurplus * highlight_crosstalk - (gSurplus * rSurplus / maxSurplus) * highlight_crosstalk;
                const float crosstalkB =
                  inputB + sumSurplus * highlight_crosstalk - (bSurplus * rSurplus / maxSurplus) * highlight_crosstalk;
                // rolloff
                float rOutput =
                  max(0.0f, crosstalkR - toe_bound + (toe_bound * toe_bound) / (crosstalkR + toe_bound + 1 / 65535.0f));
                rOutput =
                  rOutput > rolloff_bound
                    ? 65535.f - ((crystal_headroom * crystal_headroom) / (rOutput + crystal_headroom - rolloff_bound))
                    : rOutput;
                input_image(row, colr) = rOutput * crystals_per_pixel;
                float gOutput =
                  max(0.0f, crosstalkG - toe_bound + (toe_bound * toe_bound) / (crosstalkG + toe_bound + 1 / 65535.0f));
                gOutput =
                  gOutput > rolloff_bound
                    ? 65535.f - ((crystal_headroom * crystal_headroom) / (gOutput + crystal_headroom - rolloff_bound))
                    : gOutput;
                input_image(row, colg) = gOutput * crystals_per_pixel;
                float bOutput =
                  max(0.0f, crosstalkB - toe_bound + (toe_bound * toe_bound) / (crosstalkB + toe_bound + 1 / 65535.0f));
                bOutput =
                  bOutput > rolloff_bound
                    ? 65535.f - ((crystal_headroom * crystal_headroom) / (bOutput + crystal_headroom - rolloff_bound))
                    : bOutput;
                input_image(row, colb) = bOutput * crystals_per_pixel;
            }
        }
    }
}
