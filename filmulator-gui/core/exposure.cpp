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

void exposure(matrix<float> &input_image, float crystals_per_pixel,
        float rolloff_boundary)
{
    rolloff_boundary = std::max(std::min(rolloff_boundary, 65534.f), 1.f);
    const int nrows = input_image.nr();
    const int ncols = input_image.nc();
    const float crystal_headroom = 65535.f - rolloff_boundary;
    //Magic number mostly for historical reasons
    crystals_per_pixel *= 0.00015387105f;
#pragma omp parallel
    {
        #pragma omp for schedule(dynamic) nowait
        for(int row = 0; row < nrows; row++) {
            for(int col = 0; col<ncols; col++) {
                float input = max(0.0f,input_image(row,col));
                input = input > rolloff_boundary ? 65535.f - ((crystal_headroom * crystal_headroom) / (input + crystal_headroom - rolloff_boundary)) : input;
                input_image(row,col) = input * crystals_per_pixel;
            }
        }
    }
}
