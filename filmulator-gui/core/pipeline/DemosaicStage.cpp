#include "DemosaicStage.h"
#include "../filmSim.hpp"
#include <cmath>
#include <iostream>
#include <rtprocess/librtprocess.h>

using namespace std;

namespace Pipeline {

std::optional<RawImage>
  DemosaicStage::process(const RawImage &input, const DemosaicParams &params, PipelineContext &context)
{
  if (context.isAborted()) return std::nullopt;

  // If already demosaiced (TIFF/JPEG), just pass through
  if (input.isDemosaiced) { return input; }

  int raw_width = input.data.nc();
  int raw_height = input.data.nr();

  matrix<float> demosaiced_image;

  if (input.isSraw) {
    // sRAW logic
    float inputscale = input.colorMaxValue[0];
    float outputscale = 65535.0f;
    float scaleFactor = outputscale / inputscale;

    demosaiced_image = input.data;

    // Apply scaling and generic wb prescale
#pragma omp parallel for
    for (int row = 0; row < demosaiced_image.nr(); row++) {
      for (int col = 0; col < demosaiced_image.nc(); col++) {
        int color = col % 3;
        float wb = (color == 0) ? input.rPreMul : (color == 1) ? input.gPreMul : input.bPreMul;
        demosaiced_image(row, col) *= scaleFactor * wb;
      }
    }

    RawImage output = input;
    output.data = demosaiced_image;
    output.isDemosaiced = true;
    return output;
  }

  matrix<float> premultiplied(raw_height, raw_width);
  matrix<float> red(raw_height, raw_width);
  matrix<float> green(raw_height, raw_width);
  matrix<float> blue(raw_height, raw_width);

// Apply WB Pre-multipliers (Raw WB)
#pragma omp parallel for
  for (int row = 0; row < raw_height; row++) {
    for (int col = 0; col < raw_width; col++) {
      uint color = 0;
      if (input.maxXtrans > 0) {
        color = input.xtrans[row % 6][col % 6];
      } else {
        color = input.cfa[row & 1][col & 1];
      }
      // color mapping: 0=R, 1=G, 2=B, 3=G2
      float mul = (color == 0) ? input.rPreMul : (color == 2) ? input.bPreMul : input.gPreMul;
      premultiplied(row, col) = input.data(row, col) * mul;
    }
  }

  // Auto CA
  std::function<bool(double)> setProg = [](double) { return false; };

  // Demosaic
  if (input.maxXtrans > 0) {
    // XTrans
    xtransfast_demosaic(
      raw_width, raw_height, premultiplied, red, green, blue, (unsigned int (*)[6])input.xtrans, setProg);
  } else {
    // Bayer
    if (params.demosaicMethod == 0) {
      // amaze
      double initialGain = 1.0;
      float inputscale = input.colorMaxValue[1];
      float outputscale = 65535.0f;
      amaze_demosaic(raw_width,
        raw_height,
        0,
        0,
        raw_width,
        raw_height,
        premultiplied,
        red,
        green,
        blue,
        (unsigned int (*)[2])input.cfa,
        setProg,
        initialGain,
        4,
        inputscale,
        outputscale);
    } else {
      // LMMSE
      lmmse_demosaic(
        raw_width, raw_height, premultiplied, red, green, blue, (unsigned int (*)[2])input.cfa, setProg, 3);
    }
  }

  // Merge to interleaved
  demosaiced_image.set_size(raw_height, raw_width * 3);
#pragma omp parallel for
  for (int row = 0; row < raw_height; row++) {
    for (int col = 0; col < raw_width; col++) {
      demosaiced_image(row, col * 3) = red(row, col);
      demosaiced_image(row, col * 3 + 1) = green(row, col);
      demosaiced_image(row, col * 3 + 2) = blue(row, col);
    }
  }

  RawImage output = input;
  output.data = demosaiced_image;
  output.isDemosaiced = true;
  return output;
}

}// namespace Pipeline
