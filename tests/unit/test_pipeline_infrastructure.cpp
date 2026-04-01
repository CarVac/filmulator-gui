#include <catch2/catch_all.hpp>

#include "pipeline/Pipeline.h"
#include "pipeline/PipelineContext.h"
#include "pipeline/Stage.h"
#include <string>

// Mocks
struct TestParams
{
  int multiplier;
};

// Test Stage
class IntMultiplierStage : public Pipeline::Stage<int, int, TestParams>
{
public:
  std::optional<int> process(const int &input, const TestParams &params, Pipeline::PipelineContext &) override
  {
    return input * params.multiplier;
  }
};

TEST_CASE("Pipeline Stage Infrastructure", "[pipeline]")
{
  Pipeline::PipelineContext context;

  SECTION("Stage Processing")
  {
    IntMultiplierStage stage;
    TestParams params{ 2 };
    auto result = stage.process(10, params, context);
    REQUIRE(result.has_value());
    REQUIRE(result.value() == 20);
  }

  SECTION("Pipeline Context")
  {
    REQUIRE(context.isAborted() == false);
    context.abortParams = true;
    REQUIRE(context.isAborted() == true);
  }
}

TEST_CASE("Pipeline Coordinator", "[pipeline]")
{
  // Basic instantiation test
  Pipeline::PipelineCoordinator<IntMultiplierStage> pipeline;
  REQUIRE(std::tuple_size<decltype(pipeline.stages)>::value == 1);
}
