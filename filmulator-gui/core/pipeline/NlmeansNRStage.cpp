#include "NlmeansNRStage.h"
#include "../filmSim.hpp"
#include "../nlmeans/nlmeans.hpp"
#include <cmath>
#include <iostream>

using namespace std;

#include "../debug_utils.h"
#include <iomanip>
#include <iostream>

namespace Pipeline {

std::optional<RawImage>
  NlmeansNRStage::process(const RawImage &input, const NlmeansNRParams &params, PipelineContext &context)
{
  if (context.isAborted()) return std::nullopt;

  RawImage output = input;
  matrix<float> image = input.data;// copy for processing

  // Nlmeans requires RGB (post-demosaic) input.

  std::cerr << "NlmeansNRStage: enabled=" << params.nrEnabled << " strength=" << params.nlStrength << std::endl;

  if (params.nrEnabled && params.nlStrength > 0) {
    // Preprocessing
    matrix<float> denoised(image.nr(), image.nc());
    matrix<float> preconditioned = image;

    const float rMulTemp = input.isMonochrome ? 1.0f : input.rUserMul;
    const float gMulTemp = input.isMonochrome ? 1.0f : input.gUserMul;
    const float bMulTemp = input.isMonochrome ? 1.0f : input.bUserMul;

#pragma omp parallel for
    for (int row = 0; row < preconditioned.nr(); row++) {
      for (int col = 0; col < preconditioned.nc(); col += 3) {
        preconditioned(row, col + 0) =
          sRGB_forward_gamma_unclipped(preconditioned(row, col + 0) / (rMulTemp * 65535.0f));
        preconditioned(row, col + 1) =
          sRGB_forward_gamma_unclipped(preconditioned(row, col + 1) / (gMulTemp * 65535.0f));
        preconditioned(row, col + 2) =
          sRGB_forward_gamma_unclipped(preconditioned(row, col + 2) / (bMulTemp * 65535.0f));
      }
    }

    float offset = std::max(-preconditioned.min() + 0.001f, 0.001f);
    float scale = std::max(preconditioned.max() + offset, 1.0f);

#pragma omp parallel for
    for (int row = 0; row < preconditioned.nr(); row++) {
      for (int col = 0; col < preconditioned.nc(); col++) {
        preconditioned(row, col) = (preconditioned(row, col) + offset) / scale;
        if (std::isnan(preconditioned(row, col))) { preconditioned(row, col) = 0.0f; }
      }
    }

    const int numClusters = params.nlClusters;
    const float clusterThreshold = params.nlThresh;
    const float strength = params.nlStrength;

    // Perform Nlmeans
    if (kMeansNLMApprox(preconditioned,
          numClusters,
          clusterThreshold,
          strength,
          preconditioned.nr(),
          preconditioned.nc() / 3,
          denoised,
          context.paramManager)) {
      // Aborted
      return std::nullopt;
    }

// Postprocessing (Undo preconditioning)
#pragma omp parallel for
    for (int row = 0; row < denoised.nr(); row++) {
      for (int col = 0; col < denoised.nc(); col += 3) {
        denoised(row, col + 0) =
          sRGB_inverse_gamma_unclipped(scale * denoised(row, col + 0) - offset) * rMulTemp * 65535.0f;
        denoised(row, col + 1) =
          sRGB_inverse_gamma_unclipped(scale * denoised(row, col + 1) - offset) * gMulTemp * 65535.0f;
        denoised(row, col + 2) =
          sRGB_inverse_gamma_unclipped(scale * denoised(row, col + 2) - offset) * bMulTemp * 65535.0f;
      }
    }

    // Convert to OKLAB
    // raw_to_oklab(denoised, nlmeans_nr_image, camToRGB);
    matrix<float> oklab_image;
    raw_to_oklab(denoised, oklab_image, input.camToRGB);

    output.data = oklab_image;
    output.isOklab = true;

#ifdef ENABLE_NAN_TRAPPING
    int nans = 0;
    for (int i = 0; i < oklab_image.nr(); ++i) {
      for (int j = 0; j < oklab_image.nc(); ++j) {
        if (std::isnan(oklab_image(i, j))) {
          nans++;
          BREAK_ON_NAN(oklab_image(i, j));
        }
      }
    }
    if (nans > 0) { std::cerr << "NlmeansNRStage output contains " << nans << " NaNs!" << std::endl; }
#endif

  } else if (params.nrEnabled) {
    // Just convert to OKLAB
    matrix<float> oklab_image;
    raw_to_oklab(image, oklab_image, input.camToRGB);
    output.data = oklab_image;
    output.isOklab = true;
  } else {
    // Assuming user disabled NR, but what about subsequent stages?
    // If !nrEnabled, return input (RGB).
    return input;
    // Pipeline logic must handle this.
    return input;
  }

  return output;
}

}// namespace Pipeline
