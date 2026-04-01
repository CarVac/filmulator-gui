#ifndef PIPELINE_CHROMANRSTAGE_H
#define PIPELINE_CHROMANRSTAGE_H

#include "../../ui/parameterManager.h"// For ChromaNRParams
#include "RawImage.h"
#include "Stage.h"

namespace Pipeline {

class ChromaNRStage : public Stage<RawImage, RawImage, ChromaNRParams>
{
public:
  std::optional<RawImage>
    process(const RawImage &input, const ChromaNRParams &params, PipelineContext &context) override;
};

}// namespace Pipeline

#endif// PIPELINE_CHROMANRSTAGE_H
