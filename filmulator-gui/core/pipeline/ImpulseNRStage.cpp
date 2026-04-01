#include "ImpulseNRStage.h"
#include "../debug_utils.h"
#include "../rawtherapee/rt_routines.h"
#include <iostream>

namespace Pipeline {

std::optional<RawImage>
  ImpulseNRStage::process(const RawImage &input, const ImpulseNRParams &params, PipelineContext &context)
{
  if (context.isAborted()) return std::nullopt;

  // impulse_nr expects LAB (or OKLAB).

  RawImage output = input;

  std::cerr << "ImpulseNRStage: enabled=" << params.nrEnabled << " thresh=" << params.impulseThresh
            << " input min=" << "TODO" << std::endl;

  if (params.nrEnabled && params.impulseThresh > 0) {
    matrix<float> temp_in = input.data;
    matrix<float> temp_out(temp_in.nr(), temp_in.nc());

    // impulse_nr(imageIn, imageOut, thresh, chromaFactor, eraseInput)
    impulse_nr(temp_in, temp_out, params.impulseThresh, 3.0, false);

    output.data = temp_out;
    output.isOklab = input.isOklab;// Preserve flag

    int nans = 0;
    for (int i = 0; i < output.data.nr(); ++i) {
      for (int j = 0; j < output.data.nc(); ++j) {
        if (std::isnan(output.data(i, j))) {
          nans++;
          BREAK_ON_NAN(output.data(i, j));
        }
      }
    }
    if (nans > 0) { std::cerr << "ImpulseNRStage output contains " << nans << " NaNs!" << std::endl; }
  } else {
    // Pass through
  }

  return output;
}

}// namespace Pipeline
