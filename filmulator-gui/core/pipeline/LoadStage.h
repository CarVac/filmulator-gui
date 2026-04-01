#ifndef PIPELINE_LOADSTAGE_H
#define PIPELINE_LOADSTAGE_H

#include "../../ui/parameterManager.h"// For LoadParams
#include "RawImage.h"
#include "Stage.h"
#include <libraw/libraw.h>
#include <string>

namespace Pipeline {

class LoadStage : public Stage<std::string, RawImage, LoadParams>
{
public:
  std::optional<RawImage>
    process(const std::string &filename, const LoadParams &params, PipelineContext &context) override;
};

}// namespace Pipeline

#endif// PIPELINE_LOADSTAGE_H
