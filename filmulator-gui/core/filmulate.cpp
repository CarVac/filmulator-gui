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
#include "imagePipeline.h"
#include "logging.h"
#include <algorithm>
#include <cstdio>

// Function-------------------------------------------------------------------------
bool ImagePipeline::filmulate(matrix<float> &input_image,
  matrix<float> &output_density,
  ParameterManager *paramManager,
  ImagePipeline *pipeline)
{
  FilmParams filmParam;
  AbortStatus abort;
  Valid valid;
  std::tie(valid, abort, filmParam) = paramManager->claimFilmParams();
  if (abort == AbortStatus::restart) { return true; }

  // Extract parameters from struct
  float initial_developer_concentration = filmParam.initialDeveloperConcentration;
  float reservoir_thickness = filmParam.reservoirThickness;
  float active_layer_thickness = filmParam.activeLayerThickness;
  float crystals_per_pixel = filmParam.crystalsPerPixel;
  float initial_crystal_radius = filmParam.initialCrystalRadius;
  float initial_silver_salt_density = filmParam.initialSilverSaltDensity;
  float developer_consumption_const = filmParam.developerConsumptionConst;
  float crystal_growth_const = filmParam.crystalGrowthConst;
  float silver_salt_consumption_const = filmParam.silverSaltConsumptionConst;
  float total_development_time = filmParam.totalDevelopmentTime;
  int agitate_count = filmParam.agitateCount;
  int development_steps = filmParam.developmentSteps;
  float film_area = filmParam.filmArea;
  float sigma_const = filmParam.sigmaConst;
  float layer_mix_const = filmParam.layerMixConst;
  float layer_time_divisor = filmParam.layerTimeDivisor;
  float rolloff_boundary = filmParam.rolloffBoundary;
  float toe_boundary = filmParam.toeBoundary;
  float highlight_crosstalk = filmParam.highlightCrosstalk;

  // Set up timers
  std::chrono::steady_clock::time_point initialize_start, development_start, develop_start, diffuse_start,
    agitate_start, layer_mix_start;
  double develop_dif = 0, diffuse_dif = 0, agitate_dif = 0, layer_mix_dif = 0;
  initialize_start = std::chrono::steady_clock::now();

  int nrows = (int)input_image.nr();
  int ncols = (int)input_image.nc() / 3;
  int npix = nrows * ncols;

  // Now we activate some of the crystals on the film. This is literally
  // akin to exposing film to light.
  matrix<float> active_crystals_per_pixel = input_image;
  exposure(active_crystals_per_pixel, crystals_per_pixel, rolloff_boundary, toe_boundary, highlight_crosstalk);
  // We set the crystal radius to a small seed value for each color.
  matrix<float> &crystal_radius = output_density;
  crystal_radius.set_size(nrows, ncols * 3);
  crystal_radius = initial_crystal_radius;

  // All layers share developer, so we only make it the original image size.
  matrix<float> developer_concentration;
  developer_concentration.set_size(nrows, ncols);
  developer_concentration = initial_developer_concentration;

  // Each layer gets its own silver salt which will feed crystal growth.
  matrix<float> silver_salt_density;
  silver_salt_density.set_size(nrows, ncols * 3);
  silver_salt_density = initial_silver_salt_density;

  // Now, we set up the reservoir.
  // Because we don't want the film area to influence the brightness, we
  //  increase the reservoir size in proportion.
#define FILMSIZE 864;// 36x24mm
  reservoir_thickness *= film_area / FILMSIZE;
  float reservoir_developer_concentration = initial_developer_concentration;

  // This is a value used in diffuse to set the length scale.
  float pixels_per_millimeter = sqrt(npix / film_area);

  // Here we do some math for the control logic for the differential
  // equation approximation computations.
  float timestep = total_development_time / development_steps;
  int agitate_period;
  if (agitate_count > 0) {
    agitate_period = floor(development_steps / agitate_count);
  } else {
    agitate_period = 3 * development_steps;
  }
  int half_agitate_period = floor(agitate_period / 2);

  FILM_DEBUG("Initialization time: {} seconds", timeDiff(initialize_start));
  development_start = std::chrono::steady_clock::now();

  // Now we begin the main development/diffusion loop, which approximates the
  // differential equation of film development.
  for (int i = 0; i <= development_steps; i++) {
    // Check for cancellation
    abort = paramManager->claimFilmAbort();
    if (abort == AbortStatus::restart) { return true; }

    // Updating for starting the development simulation. Valid is one too high
    // here.
    pipeline->updateProgress(Valid::partfilmulation, float(i) / float(development_steps));

    develop_start = std::chrono::steady_clock::now();

    // This is where we perform the chemical reaction part.
    // The crystals grow.
    // The developer in the active layer is consumed.
    // So is the silver salt in the film.
    //  The amount consumed increases as the crystals grow larger.
    // Because the developer and silver salts are consumed in bright regions,
    //  this reduces the rate at which they grow. This gives us global
    //  contrast reduction.
    develop(crystal_radius,
      crystal_growth_const,
      active_crystals_per_pixel,
      silver_salt_density,
      developer_concentration,
      active_layer_thickness,
      developer_consumption_const,
      silver_salt_consumption_const,
      timestep);

    develop_dif += timeDiff(develop_start);
    diffuse_start = std::chrono::steady_clock::now();

    // Check for cancellation
    abort = paramManager->claimFilmAbort();
    if (abort == AbortStatus::restart) { return true; }

    // Updating for starting the diffusion simulation. Valid is one too high
    // here.
    pipeline->updateProgress(Valid::partfilmulation, float(i) / float(development_steps));

    // Now, we are going to perform the diffusion part.
    // Here we mix the layer among itself, which grants us the
    //  local contrast increases.
    diffuse(developer_concentration, sigma_const, pixels_per_millimeter, timestep);
    //        diffuse_short_convolution(developer_concentration,
    //                                  sigma_const,
    //                                  pixels_per_millimeter,
    //                                  timestep);
    //        diffuse_resize_iir(developer_concentration,
    //                           sigma_const,
    //                           pixels_per_millimeter,
    //                           timestep);

    diffuse_dif += timeDiff(diffuse_start);

    layer_mix_start = std::chrono::steady_clock::now();
    // This performs mixing between the active layer adjacent to the film
    //  and the reservoir.
    // This keeps the effects from getting too crazy.
    layer_mix(developer_concentration,
      active_layer_thickness,
      reservoir_developer_concentration,
      reservoir_thickness,
      layer_mix_const,
      layer_time_divisor,
      pixels_per_millimeter,
      timestep);

    layer_mix_dif += timeDiff(layer_mix_start);

    agitate_start = std::chrono::steady_clock::now();

    // I want agitation to only occur in the middle of development, not
    // at the very beginning or the ends. So, I add half the agitate
    // period to the current cycle count.
    if ((i + half_agitate_period) % agitate_period == 0)
      agitate(developer_concentration,
        active_layer_thickness,
        reservoir_developer_concentration,
        reservoir_thickness,
        pixels_per_millimeter);

    agitate_dif += timeDiff(agitate_start);
  }
  FILM_DEBUG("Development time: {} seconds", timeDiff(development_start));
  FILM_DEBUG("Develop time: {} seconds", develop_dif);
  FILM_DEBUG("Diffuse time: {} seconds", diffuse_dif);
  FILM_DEBUG("Layer mix time: {} seconds", layer_mix_dif);
  FILM_DEBUG("Agitate time: {} seconds", agitate_dif);

  // Now we compute the density (opacity) of the film.
  // We assume that overlapping crystals or dye clouds are
  // nonexistent. It works okay, for now...
  // The output is crystal_radius^2 * active_crystals_per_pixel
  std::chrono::steady_clock::time_point mult_start;
  mult_start = std::chrono::steady_clock::now();

  abort = paramManager->claimFilmAbort();
  if (abort == AbortStatus::restart) { return true; }

  const int numRows = crystal_radius.nr();
  const int numCols = crystal_radius.nc();

#pragma omp parallel for
  for (int i = 0; i < numRows; ++i) {
    for (int j = 0; j < numCols; ++j) {
      output_density(i, j) = crystal_radius(i, j) * crystal_radius(i, j) * active_crystals_per_pixel(i, j);
    }
  }
  FILM_DEBUG("Output density time: {}", timeDiff(mult_start));
#ifdef DOUT
  debug_out.close();
#endif
  return false;
}
