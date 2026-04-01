#include "PostDemosaicStage.h"
#include "../filmSim.hpp"
#include <cmath>
#include <iostream>
#include <rtprocess/librtprocess.h>

using namespace std;

namespace Pipeline {

std::optional<RawImage>
  PostDemosaicStage::process(const RawImage &input, const PostDemosaicParams &params, PipelineContext &context)
{
  if (context.isAborted()) return std::nullopt;

  RawImage output = input;
  matrix<float> &image = output.data;

  if (image.nc() == 0 || image.nr() == 0) return std::nullopt;

  // 1. User White Balance
  if (!input.isMonochrome) {
    float rUserMul, gUserMul, bUserMul;

    rawWhiteBalance(input.data,
      image,// input, output
      params.temperature,
      params.tint,
      (float (*)[3])input.xyzToCam,
      input.rPreMul,
      input.gPreMul,
      input.bPreMul,
      rUserMul,
      gUserMul,
      bUserMul);

    output.rUserMul = rUserMul;
    output.gUserMul = gUserMul;
    output.bUserMul = bUserMul;

  } else {
    // Monochrome: just copy rawWB not applied?
    // In imagePipeline.cpp: post_demosaic_image = demosaiced_image;
    image = input.data;
  }

  // 2. Highlight Recovery
  if (params.highlights >= 2 && !input.isMonochrome) {
    int height = image.nr();
    int width = image.nc() / 3;

    // Split channels
    matrix<float> rChannel(height, width), gChannel(height, width), bChannel(height, width);
// Copy to channels (parallelized in original)
#pragma omp parallel for
    for (int row = 0; row < height; row++) {
      for (int col = 0; col < width; col++) {
        rChannel(row, col) = image(row, col * 3);
        gChannel(row, col) = image(row, col * 3 + 1);
        bChannel(row, col) = image(row, col * 3 + 2);
      }
    }

    const float chmax[3] = { rChannel.max(), gChannel.max(), bChannel.max() };
    const float clmax[3] = { 65535.0f * output.rUserMul * input.colorMaxValue[0] / input.maxValue,
      65535.0f * output.gUserMul * input.colorMaxValue[1] / input.maxValue,
      65535.0f * output.bUserMul * input.colorMaxValue[2] / input.maxValue };

    std::function<bool(double)> setProg = [](double) { return false; };

    HLRecovery_inpaint(width, height, rChannel, gChannel, bChannel, chmax, clmax, setProg);

// Merge back
#pragma omp parallel for
    for (int row = 0; row < height; row++) {
      for (int col = 0; col < width; col++) {
        image(row, col * 3) = rChannel(row, col);
        image(row, col * 3 + 1) = gChannel(row, col);
        image(row, col * 3 + 2) = bChannel(row, col);
      }
    }

  } else if (params.highlights == 0 && !input.isMonochrome) {
// Clip to 65535
#pragma omp parallel for
    for (int row = 0; row < image.nr(); row++) {
      for (int col = 0; col < image.nc(); col++) { image(row, col) = std::min(image(row, col), 65535.0f); }
    }
  }

  // 3. Exposure Compensation
  float expCompMult = pow(2, params.exposureComp);
#pragma omp parallel for
  for (int row = 0; row < image.nr(); row++) {
    for (int col = 0; col < image.nc(); col++) { image(row, col) *= expCompMult; }
  }

  return output;
}

}// namespace Pipeline
