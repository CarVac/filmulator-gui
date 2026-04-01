#ifndef PIPELINE_NLMEANSNRSTAGE_H
#define PIPELINE_NLMEANSNRSTAGE_H

#include "../../ui/parameterManager.h"// For NlmeansNRParams
#include "RawImage.h"
#include "Stage.h"

namespace Pipeline {

class NlmeansNRStage : public Stage<RawImage, RawImage, NlmeansNRParams>
{
public:
  std::optional<RawImage>
    process(const RawImage &input, const NlmeansNRParams &params, PipelineContext &context) override;
};

}// namespace Pipeline

#endif// PIPELINE_NLMEANSNRSTAGE_H
