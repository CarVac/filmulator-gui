#ifndef PIPELINE_PREFILMULATIONSTAGE_H
#define PIPELINE_PREFILMULATIONSTAGE_H

#include "../../ui/parameterManager.h"// For PrefilmParams and CropParams
#include "RawImage.h"
#include "Stage.h"

namespace Pipeline {

class PrefilmulationStage : public Stage<RawImage, RawImage, PrefilmParams>
{
public:
  std::optional<RawImage>
    process(const RawImage &input, const PrefilmParams &params, PipelineContext &context) override;
};

}// namespace Pipeline

#endif// PIPELINE_PREFILMULATIONSTAGE_H
