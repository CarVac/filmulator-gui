#ifndef PIPELINE_DEMOSAICSTAGE_H
#define PIPELINE_DEMOSAICSTAGE_H

#include "../../ui/parameterManager.h"// For DemosaicParams
#include "../matrix.hpp"
#include "RawImage.h"
#include "Stage.h"

namespace Pipeline {

class DemosaicStage : public Stage<RawImage, RawImage, DemosaicParams>
{
public:
  std::optional<RawImage>
    process(const RawImage &input, const DemosaicParams &params, PipelineContext &context) override;
};

}// namespace Pipeline

#endif// PIPELINE_DEMOSAICSTAGE_H
