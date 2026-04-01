#include "ChromaNRStage.h"
#include "../rawtherapee/rt_routines.h"
#include <iostream>

namespace Pipeline {

std::optional<RawImage>
  ChromaNRStage::process(const RawImage &input, const ChromaNRParams &params, PipelineContext &context)
{
  if (context.isAborted()) return std::nullopt;

  RawImage output = input;

  if (params.nrEnabled && params.chromaStrength > 0) {
    matrix<float> temp_in = input.data;
    matrix<float> temp_out(temp_in.nr(), temp_in.nc());

    std::cerr << "ChromaNRStage: enabled=" << params.nrEnabled << " strength=" << params.chromaStrength << std::endl;
    // RGB_denoise(kall, src, dst, chroma, redchro, bluechro, paramManager, eraseInput)
    RGB_denoise(0, temp_in, temp_out, params.chromaStrength, 0.0f, 0.0f, context.paramManager, false);

    output.data = temp_out;
    output.isOklab = input.isOklab;
  } else {
    // Pass through
  }

  return output;
}

}// namespace Pipeline
