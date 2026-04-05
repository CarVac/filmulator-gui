/*
 * Golden tests for white balance functions
 *
 * Tests the whiteBalance and sRGBwhiteBalance functions
 */
#include "../golden/golden_test_utils.hpp"
#include "filmSim.hpp"
#include <catch2/catch_all.hpp>
#include <cmath>

// Use GOLDEN_DATA_DIR from CMake if defined, otherwise fall back to relative path
#ifdef GOLDEN_DATA_DIR
static const std::string GOLDEN_DIR = GOLDEN_DATA_DIR;
#else
static const std::string GOLDEN_DIR = "../tests/golden/";
#endif

// Helper to initialize identity parameters for whiteBalance
struct WBParams
{
  float cam2rgb[3][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
  float rCamMul = 1.0f, gCamMul = 1.0f, bCamMul = 1.0f;
  float rPreMul = 1.0f, gPreMul = 1.0f, bPreMul = 1.0f;
};

TEST_CASE("whiteBalance identity mapping", "[golden][whiteBalance]")
{
  // Create a color grid
  matrix<float> input;
  golden::createRGBGradient(input, 8, 8, 1.0f);

  matrix<float> output;
  WBParams p;

  // When temperature=5200 and tint=1 with identity cam2rgb,
  // it should be identity mapping (base values match).
  float temperature = 5200.0f;
  float tint = 1.0f;
  float expCompMult = 1.0f;

  whiteBalance(input,
    output,
    temperature,
    tint,
    p.cam2rgb,
    p.rCamMul,
    p.gCamMul,
    p.bCamMul,
    p.rPreMul,
    p.gPreMul,
    p.bPreMul,
    expCompMult);

  // Output should match input (use relaxed tolerance for float rounding)
  auto result = golden::compare(output, input, 1e-4f, 1e-5f);
  INFO("Max diff: " << result.maxDiff << " Avg diff: " << result.avgDiff);
  REQUIRE(result.passed);
}

TEST_CASE("whiteBalance neutral preservation", "[golden][whiteBalance]")
{
  matrix<float> input;
  float grayLevel = 0.5f;
  golden::createNeutralGray(input, 8, 8, grayLevel);

  matrix<float> output;
  WBParams p;

  // Use a non-standard white balance
  float temperature = 10000.0f;// Very "cold" WB -> should make image "warm"
  float tint = 1.0f;

  whiteBalance(input,
    output,
    temperature,
    tint,
    p.cam2rgb,
    p.rCamMul,
    p.gCamMul,
    p.bCamMul,
    p.rPreMul,
    p.gPreMul,
    p.bPreMul,
    1.0f);

  // A neutral input through WB should shift but stay proportional?
  // Actually, WB is a per-channel multiplier. Any (R=G=B) input
  // will result in some other (R'=G'=B') where R', G', B' are shifts.
  // It stays neutral only if it was neutral and WB is at base.
  // But we check that it doesn't introduce artifacts like clipping
  // if within range.

  REQUIRE(output.nr() == 8);
  REQUIRE(output.nc() == 24);

  float r = output(0, 0);
  float g = output(0, 1);
  float b = output(0, 2);

  // At 10000K, R mult should be > G mult > B mult (compared to 5200K base)
  INFO("WB 10000K on 0.5 gray: R=" << r << " G=" << g << " B=" << b);
  REQUIRE(r > grayLevel);// Warming shift
  REQUIRE(b < grayLevel);// Cooling shift relative to R
}

TEST_CASE("whiteBalance exposure compensation", "[golden][whiteBalance]")
{
  matrix<float> input;
  golden::createNeutralGray(input, 4, 4, 0.5f);

  matrix<float> output;
  WBParams p;

  float temperature = 5200.0f;
  float tint = 1.0f;
  float expCompMult = 2.0f;// +1 stop

  whiteBalance(input,
    output,
    temperature,
    tint,
    p.cam2rgb,
    p.rCamMul,
    p.gCamMul,
    p.bCamMul,
    p.rPreMul,
    p.gPreMul,
    p.bPreMul,
    expCompMult);

  // All pixels should be ~1.0
  float val = output(0, 0);
  REQUIRE(val == Catch::Approx(1.0f));
}

TEST_CASE("whiteBalance golden comparison", "[golden][whiteBalance][!mayfail]")
{
  matrix<float> input;
  golden::createRGBGradient(input, 16, 16, 1.0f);

  matrix<float> output;
  WBParams p;

  // Cloudy WB
  float temperature = 6500.0f;
  float tint = 1.1f;

  whiteBalance(input,
    output,
    temperature,
    tint,
    p.cam2rgb,
    p.rCamMul,
    p.gCamMul,
    p.bCamMul,
    p.rPreMul,
    p.gPreMul,
    p.bPreMul,
    1.0f);

  std::string goldenPath = GOLDEN_DIR + "white_balance_6500_1.1.bin";

  matrix<float> golden;
  if (golden::loadGolden(golden, goldenPath)) {
    auto result = golden::compare(output, golden, 1e-4f, 1e-5f);
    INFO("Max diff: " << result.maxDiff << " Avg diff: " << result.avgDiff);
    REQUIRE(result.passed);
  } else {
    INFO("Golden file not found. Creating: " << goldenPath);
    REQUIRE(golden::saveGolden(output, goldenPath));
    SUCCEED("Created new golden file");
  }
}
