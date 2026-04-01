#ifndef PIPELINE_IMPULSENRSTAGE_H
#define PIPELINE_IMPULSENRSTAGE_H

#include "../../ui/parameterManager.h"// For ImpulseNRParams
#include "RawImage.h"
#include "Stage.h"

namespace Pipeline {

class ImpulseNRStage : public Stage<RawImage, RawImage, ImpulseNRParams>
{
public:
  std::optional<RawImage>
    process(const RawImage &input, const ImpulseNRParams &params, PipelineContext &context) override;
};

}// namespace Pipeline

#endif// PIPELINE_IMPULSENRSTAGE_H
