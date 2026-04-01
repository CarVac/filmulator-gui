#ifndef PIPELINE_COORDINATOR_H
#define PIPELINE_COORDINATOR_H

#include "Stage.h"
#include <optional>
#include <tuple>

namespace Pipeline {

// Helper to run a sequence of stages
// This is a placeholder for the full implementation
template<typename... Stages> class PipelineCoordinator
{
public:
  std::tuple<Stages...> stages;

  template<std::size_t I> auto &getStage() { return std::get<I>(stages); }
};

}// namespace Pipeline

#endif// PIPELINE_COORDINATOR_H
