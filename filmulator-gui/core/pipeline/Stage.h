#ifndef PIPELINE_STAGE_H
#define PIPELINE_STAGE_H

#include "PipelineContext.h"
#include <optional>

namespace Pipeline {

template<typename InputT, typename OutputT, typename ParamsT> class Stage
{
public:
  using Input = InputT;
  using Output = OutputT;
  using Params = ParamsT;

  virtual ~Stage() = default;
  Stage() = default;
  Stage(const Stage &) = default;
  Stage &operator=(const Stage &) = default;
  Stage(Stage &&) = default;
  Stage &operator=(Stage &&) = default;

  // Returns std::nullopt if aborted
  virtual std::optional<Output> process(const Input &input, const Params &params, PipelineContext &context) = 0;
};

}// namespace Pipeline

#endif// PIPELINE_STAGE_H
