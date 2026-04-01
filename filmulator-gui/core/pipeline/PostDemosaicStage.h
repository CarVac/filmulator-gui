#ifndef PIPELINE_POSTDEMOSAICSTAGE_H
#define PIPELINE_POSTDEMOSAICSTAGE_H

#include "../../ui/parameterManager.h"// For PostDemosaicParams
#include "RawImage.h"
#include "Stage.h"

namespace Pipeline {

class PostDemosaicStage : public Stage<RawImage, RawImage, PostDemosaicParams>
{
public:
  std::optional<RawImage>
    process(const RawImage &input, const PostDemosaicParams &params, PipelineContext &context) override;
};

}// namespace Pipeline

#endif// PIPELINE_POSTDEMOSAICSTAGE_H
