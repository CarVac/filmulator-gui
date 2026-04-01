/*
 * Golden tests for NLMeans denoising
 */
#include "../golden/golden_test_utils.hpp"
#include "nlmeans/nlmeans.hpp"
#include "ui/parameterManager.h"
#include <QCoreApplication>
#include <catch2/catch_all.hpp>
#include <random>

// Use GOLDEN_DATA_DIR from CMake if defined
#ifdef GOLDEN_DATA_DIR
static const std::string GOLDEN_DIR = GOLDEN_DATA_DIR;
#else
static const std::string GOLDEN_DIR = "../tests/golden/data/";
#endif

// Helper to create a ParameterManager instance ensuring QCoreApplication
// existence
class TestParameterManager : public ParameterManager
{
public:
  TestParameterManager() : ParameterManager() {}
};

TEST_CASE("nlmeans basic operation", "[golden][nlmeans]")
{
  // Ensure QCoreApplication exists for QObject
  static int argc = 1;
  static char *argv[] = { (char *)"test" };
  static QCoreApplication *app = nullptr;
  if (!QCoreApplication::instance()) { app = new QCoreApplication(argc, argv); }

  TestParameterManager paramManager;

  int width = 64;
  int height = 64;
  // Create a noisy gradient
  matrix<float> input(height, width * 3);
  matrix<float> output(height, width * 3);

  // Seed random number generator for reproducibility
  std::mt19937 gen(42);
  std::uniform_real_distribution<float> noise(-0.1f, 0.1f);

  for (int row = 0; row < height; row++) {
    for (int col = 0; col < width; col++) {
      float val = static_cast<float>(col) / width;
      input(row, col * 3 + 0) = std::max(0.0f, std::min(1.0f, val + noise(gen)));
      input(row, col * 3 + 1) = std::max(0.0f, std::min(1.0f, val + noise(gen)));
      input(row, col * 3 + 2) = std::max(0.0f, std::min(1.0f, val + noise(gen)));
    }
  }

  int clusters = 4;
  float threshold = 0.5f;
  float h = 1.0f;

  // Set validity to prevent abort, as kMeansNLMApprox checks
  // claimNlmeansNRAbort
  paramManager.setValid(Valid::partnrnlmeans);

  // x is rows (height), y is cols (width) based on implementation analysis
  bool aborted = kMeansNLMApprox((float *)input, clusters, threshold, h, height, width, (float *)output, &paramManager);

  REQUIRE_FALSE(aborted);

  // Verify output dimensions and values
  REQUIRE(output.nr() == height);
  REQUIRE(output.nc() == width * 3);

  // Check that noise is reduced (smoothness check)
  // Calculate local variance of input vs output
  float inputVar = 0.0f;
  float outputVar = 0.0f;
  int samples = 0;

  for (int row = 1; row < height - 1; row++) {
    for (int col = 1; col < width - 1; col++) {
      // Check green channel variance
      float inVal = input(row, col * 3 + 1);
      float outVal = output(row, col * 3 + 1);

      float inNeighbors = (input(row, (col - 1) * 3 + 1) + input(row, (col + 1) * 3 + 1) + input(row - 1, col * 3 + 1)
                            + input(row + 1, col * 3 + 1))
                          / 4.0f;
      float outNeighbors = (output(row, (col - 1) * 3 + 1) + output(row, (col + 1) * 3 + 1)
                             + output(row - 1, col * 3 + 1) + output(row + 1, col * 3 + 1))
                           / 4.0f;

      inputVar += (inVal - inNeighbors) * (inVal - inNeighbors);
      outputVar += (outVal - outNeighbors) * (outVal - outNeighbors);
      samples++;
    }
  }

  INFO("Input smoothness: " << inputVar << " Output smoothness: " << outputVar);
  REQUIRE(outputVar < inputVar);
}

TEST_CASE("nlmeans golden comparison", "[golden][nlmeans]")
{
  // Ensure QCoreApplication exists
  if (!QCoreApplication::instance()) {
    static int argc = 1;
    static char *argv[] = { (char *)"test" };
    new QCoreApplication(argc, argv);
  }
  TestParameterManager paramManager;

  // Use a smaller image for golden test to keep file size reasonable but large
  // enough for texture
  int width = 96;
  int height = 96;
  matrix<float> input(height, width * 3);
  matrix<float> output(height, width * 3);

  // Create a pattern with features and noise
  std::mt19937 gen(12345);
  std::normal_distribution<float> noise(0.0f, 0.05f);

  for (int row = 0; row < height; row++) {
    for (int col = 0; col < width; col++) {
      // Checkboard pattern
      bool check = ((row / 16) + (col / 16)) % 2 == 0;
      float base = check ? 0.7f : 0.3f;

      // Add some edges
      if (row > 30 && row < 60 && col > 30 && col < 60) { base = 0.5f; }

      for (int c = 0; c < 3; c++) { input(row, col * 3 + c) = std::max(0.0f, std::min(1.0f, base + noise(gen))); }
    }
  }

  int clusters = 10;
  float threshold = 1.0f;
  float h = 10.0f;

  paramManager.setValid(Valid::partnrnlmeans);
  kMeansNLMApprox((float *)input, clusters, threshold, h, height, width, (float *)output, &paramManager);

  std::string goldenPath = GOLDEN_DIR + "nlmeans_denoised.bin";

  matrix<float> golden;
  if (golden::loadGolden(golden, goldenPath)) {
    // NLMeans can be slightly non-deterministic due to k-means seeding or
    // parallel reduction order if not carefully handled, but kMeansNLMApprox
    // uses fixed seed 0 for generator. Parallel execution might still introduce
    // minor float diffs.
    auto result = golden::compare(output, golden, 1e-4f, 1e-4f);
    INFO("Max diff: " << result.maxDiff << " Avg diff: " << result.avgDiff);
    REQUIRE(result.passed);
  } else {
    INFO("Creating golden file: " << goldenPath);
    REQUIRE(golden::saveGolden(output, goldenPath));
    SUCCEED("Created new golden file");
  }
}
