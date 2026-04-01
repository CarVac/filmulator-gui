/*
 * Golden tests for core filmulation functions
 *
 * Tests the core film development simulation: exposure, develop, agitate, diffuse
 * These are the heart of the Filmulator algorithm.
 */
#include "../golden/golden_test_utils.hpp"
#include "filmSim.hpp"
#include <catch2/catch_all.hpp>

// Use GOLDEN_DATA_DIR from CMake if defined
#ifdef GOLDEN_DATA_DIR
static const std::string GOLDEN_DIR = GOLDEN_DATA_DIR;
#else
static const std::string GOLDEN_DIR = "../tests/golden/";
#endif

//-----------------------------------------------------------------------------
// exposure() tests - Applies exposure curve to input image
//-----------------------------------------------------------------------------

TEST_CASE("exposure basic operation", "[golden][filmulate][exposure]")
{
  // Create a gradient input
  matrix<float> input;
  golden::createRGBGradient(input, 8, 8, 65535.0f);

  float crystalsPerPixel = 100.0f;
  float rolloffBoundary = 51200.0f;
  float toeBoundary = 0.0f;

  exposure(input, crystalsPerPixel, rolloffBoundary, toeBoundary, 0.0f);

  // Verify output is valid (no NaN or Inf)
  REQUIRE(input.nr() == 8);
  REQUIRE(input.nc() == 24);

  for (int row = 0; row < input.nr(); row++) {
    for (int col = 0; col < input.nc(); col++) {
      float val = input(row, col);
      REQUIRE_FALSE(std::isnan(val));
      REQUIRE_FALSE(std::isinf(val));
      REQUIRE(val >= 0.0f);
    }
  }
}

TEST_CASE("exposure rolloff behavior", "[golden][filmulate][exposure]")
{
  // Test that values above rolloff boundary get compressed
  matrix<float> input;
  input.set_size(1, 3);
  input(0, 0) = 65535.0f;// R - at max
  input(0, 1) = 32768.0f;// G - mid
  input(0, 2) = 16384.0f;// B - low

  float crystalsPerPixel = 100.0f;
  float rolloffBoundary = 32768.0f;// Should compress values above this
  float toeBoundary = 0.0f;

  exposure(input, crystalsPerPixel, rolloffBoundary, toeBoundary, 0.0f);

  // After rolloff, the difference between high values should be compressed
  float outR = input(0, 0);
  float outG = input(0, 1);
  float outB = input(0, 2);

  INFO("After exposure: R=" << outR << " G=" << outG << " B=" << outB);
  REQUIRE(outR > outG);
  REQUIRE(outG > outB);
}

TEST_CASE("exposure golden comparison", "[golden][filmulate][exposure]")
{
  matrix<float> input;
  golden::createRGBGradient(input, 16, 16, 65535.0f);

  float crystalsPerPixel = 100.0f;
  float rolloffBoundary = 51200.0f;
  float toeBoundary = 1000.0f;

  exposure(input, crystalsPerPixel, rolloffBoundary, toeBoundary, 0.0f);

  std::string goldenPath = GOLDEN_DIR + "exposure_default.bin";

  matrix<float> golden;
  if (golden::loadGolden(golden, goldenPath)) {
    auto result = golden::compare(input, golden, 1e-3f, 1e-4f);
    INFO("Max diff: " << result.maxDiff << " Avg diff: " << result.avgDiff);
    REQUIRE(result.passed);
  } else {
    INFO("Creating golden file: " << goldenPath);
    REQUIRE(golden::saveGolden(input, goldenPath));
    SUCCEED("Created new golden file");
  }
}

//-----------------------------------------------------------------------------
// develop() tests - Simulates one step of development reaction
//-----------------------------------------------------------------------------

TEST_CASE("develop step basic operation", "[golden][filmulate][develop]")
{
  // Set up initial conditions for a single development step
  int height = 8;
  int width = 8;
  int rgbCols = width * 3;

  matrix<float> crystalRad(height, rgbCols);
  matrix<float> activeCrystalsPerPixel(height, rgbCols);
  matrix<float> silverSaltDensity(height, rgbCols);
  matrix<float> develConcentration(height, width);

  // Initialize with reasonable starting values
  crystalRad = 0.0001f;// Small initial crystal size
  activeCrystalsPerPixel = 100.0f;
  silverSaltDensity = 1.0f;
  develConcentration = 1.0f;

  float crystalGrowthConst = 0.00002f;
  float activeLayerThickness = 0.1f;
  float developerConsumptionConst = 0.0004f;
  float silverSaltConsumptionConst = 0.0001f;
  float timestep = 1.0f;

  develop(crystalRad,
    crystalGrowthConst,
    activeCrystalsPerPixel,
    silverSaltDensity,
    develConcentration,
    activeLayerThickness,
    developerConsumptionConst,
    silverSaltConsumptionConst,
    timestep);

  // Verify crystals have grown (even slightly)
  float newCrystalRad = crystalRad(0, 0);
  INFO("Crystal radius after develop: " << newCrystalRad);
  REQUIRE(newCrystalRad >= 0.0001f);// Should be non-negative

  // Verify developer is valid (non-negative)
  float newDevelConc = develConcentration(0, 0);
  INFO("Developer concentration after develop: " << newDevelConc);
  REQUIRE(newDevelConc >= 0.0f);
  REQUIRE(newDevelConc <= 1.0f);
}

TEST_CASE("develop with varied exposure", "[golden][filmulate][develop]")
{
  // Simulate development with different exposure levels
  int height = 4;
  int width = 4;
  int rgbCols = width * 3;

  matrix<float> crystalRad(height, rgbCols);
  matrix<float> activeCrystalsPerPixel(height, rgbCols);
  matrix<float> silverSaltDensity(height, rgbCols);
  matrix<float> develConcentration(height, width);

  // Initialize
  crystalRad = 0.0001f;
  develConcentration = 1.0f;

  // Vary silver salt density (simulates different exposures)
  for (int row = 0; row < height; row++) {
    for (int col = 0; col < rgbCols; col++) {
      // More exposed areas have more silver salt
      float exposure = static_cast<float>(col) / static_cast<float>(rgbCols - 1);
      silverSaltDensity(row, col) = 0.5f + exposure;
      activeCrystalsPerPixel(row, col) = 50.0f + exposure * 100.0f;
    }
  }

  float crystalGrowthConst = 0.00002f;
  float activeLayerThickness = 0.1f;
  float developerConsumptionConst = 0.0004f;
  float silverSaltConsumptionConst = 0.0001f;
  float timestep = 1.0f;

  develop(crystalRad,
    crystalGrowthConst,
    activeCrystalsPerPixel,
    silverSaltDensity,
    develConcentration,
    activeLayerThickness,
    developerConsumptionConst,
    silverSaltConsumptionConst,
    timestep);

  // More exposed areas should develop more
  float lowExpCrystal = crystalRad(0, 0);
  float highExpCrystal = crystalRad(0, rgbCols - 1);

  INFO("Low exposure crystal: " << lowExpCrystal << " High exposure crystal: " << highExpCrystal);
  REQUIRE(highExpCrystal > lowExpCrystal);
}

TEST_CASE("develop golden comparison", "[golden][filmulate][develop]")
{
  int height = 16;
  int width = 16;
  int rgbCols = width * 3;

  matrix<float> crystalRad(height, rgbCols);
  matrix<float> activeCrystalsPerPixel(height, rgbCols);
  matrix<float> silverSaltDensity(height, rgbCols);
  matrix<float> develConcentration(height, width);

  // Create a pattern
  for (int row = 0; row < height; row++) {
    for (int col = 0; col < rgbCols; col++) {
      float pos = static_cast<float>(col) / static_cast<float>(rgbCols - 1);
      crystalRad(row, col) = 0.0001f;
      activeCrystalsPerPixel(row, col) = 100.0f * pos;
      silverSaltDensity(row, col) = 1.0f;
    }
  }
  develConcentration = 1.0f;

  float crystalGrowthConst = 0.00002f;
  float activeLayerThickness = 0.1f;
  float developerConsumptionConst = 0.0004f;
  float silverSaltConsumptionConst = 0.0001f;
  float timestep = 1.0f;

  develop(crystalRad,
    crystalGrowthConst,
    activeCrystalsPerPixel,
    silverSaltDensity,
    develConcentration,
    activeLayerThickness,
    developerConsumptionConst,
    silverSaltConsumptionConst,
    timestep);

  std::string goldenPath = GOLDEN_DIR + "develop_step.bin";

  matrix<float> golden;
  if (golden::loadGolden(golden, goldenPath)) {
    auto result = golden::compare(crystalRad, golden, 1e-6f, 1e-7f);
    INFO("Max diff: " << result.maxDiff << " Avg diff: " << result.avgDiff);
    REQUIRE(result.passed);
  } else {
    INFO("Creating golden file: " << goldenPath);
    REQUIRE(golden::saveGolden(crystalRad, goldenPath));
    SUCCEED("Created new golden file");
  }
}

//-----------------------------------------------------------------------------
// agitate() tests - Equalizes developer concentration
//-----------------------------------------------------------------------------

TEST_CASE("agitate equalizes concentration", "[golden][filmulate][agitate]")
{
  // Create uneven developer concentration
  matrix<float> develConcentration(8, 8);

  // Top half depleted, bottom half fresh
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 8; col++) { develConcentration(row, col) = 0.2f; }
  }
  for (int row = 4; row < 8; row++) {
    for (int col = 0; col < 8; col++) { develConcentration(row, col) = 1.0f; }
  }

  float activeLayerThickness = 0.1f;
  float reservoirDevelConcentration = 1.0f;
  float reservoirThickness = 10.0f;
  float pixelsPerMillimeter = 100.0f;

  agitate(
    develConcentration, activeLayerThickness, reservoirDevelConcentration, reservoirThickness, pixelsPerMillimeter);

  // After agitation, all concentrations should be equal
  float firstVal = develConcentration(0, 0);
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) { REQUIRE(develConcentration(row, col) == Catch::Approx(firstVal)); }
  }
}

TEST_CASE("agitate conservation of developer", "[golden][filmulate][agitate]")
{
  // Test that total developer is conserved
  matrix<float> develConcentration(4, 4);
  develConcentration = 0.5f;

  float activeLayerThickness = 0.1f;
  float reservoirDevelConcentration = 1.0f;
  float reservoirThickness = 10.0f;
  float pixelsPerMillimeter = 100.0f;

  // Calculate total before
  float pixelArea = 1.0f / (pixelsPerMillimeter * pixelsPerMillimeter);
  float totalBefore =
    sum(develConcentration) * activeLayerThickness * pixelArea + reservoirDevelConcentration * reservoirThickness;

  agitate(
    develConcentration, activeLayerThickness, reservoirDevelConcentration, reservoirThickness, pixelsPerMillimeter);

  // Calculate total after
  float totalAfter =
    sum(develConcentration) * activeLayerThickness * pixelArea + reservoirDevelConcentration * reservoirThickness;

  INFO("Total before: " << totalBefore << " Total after: " << totalAfter);
  REQUIRE(totalAfter == Catch::Approx(totalBefore).epsilon(0.01));
}

//-----------------------------------------------------------------------------
// diffuse() tests - Simulates lateral diffusion
//-----------------------------------------------------------------------------

TEST_CASE("diffuse smooths concentration", "[golden][filmulate][diffuse]")
{
  // Create a concentration with a pattern
  matrix<float> develConcentration(16, 16);
  develConcentration = 0.0f;

  // Create a single bright spot in the center
  develConcentration(7, 7) = 1.0f;
  develConcentration(7, 8) = 1.0f;
  develConcentration(8, 7) = 1.0f;
  develConcentration(8, 8) = 1.0f;

  float sigmaConst = 0.002f;
  float pixelsPerMillimeter = 100.0f;
  float timestep = 1.0f;

  diffuse(develConcentration, sigmaConst, pixelsPerMillimeter, timestep);

  // Verify output is valid (values in reasonable range)
  float centerAfter = develConcentration(7, 7);
  INFO("Center after diffuse: " << centerAfter);
  REQUIRE(centerAfter >= 0.0f);
  REQUIRE(centerAfter <= 1.0f);
}

TEST_CASE("diffuse golden comparison", "[golden][filmulate][diffuse]")
{
  matrix<float> develConcentration(32, 32);
  develConcentration = 0.0f;

  // Create a gradient pattern
  for (int row = 0; row < 32; row++) {
    for (int col = 0; col < 32; col++) {
      float dist = std::sqrt(static_cast<float>((row - 16) * (row - 16) + (col - 16) * (col - 16)));
      develConcentration(row, col) = std::max(0.0f, 1.0f - dist / 16.0f);
    }
  }

  float sigmaConst = 0.002f;
  float pixelsPerMillimeter = 100.0f;
  float timestep = 1.0f;

  diffuse(develConcentration, sigmaConst, pixelsPerMillimeter, timestep);

  std::string goldenPath = GOLDEN_DIR + "diffuse_pattern.bin";

  matrix<float> golden;
  if (golden::loadGolden(golden, goldenPath)) {
    auto result = golden::compare(develConcentration, golden, 1e-4f, 1e-5f);
    INFO("Max diff: " << result.maxDiff << " Avg diff: " << result.avgDiff);
    REQUIRE(result.passed);
  } else {
    INFO("Creating golden file: " << goldenPath);
    REQUIRE(golden::saveGolden(develConcentration, goldenPath));
    SUCCEED("Created new golden file");
  }
}

//-----------------------------------------------------------------------------
// Integration test - Multiple steps simulating development
//-----------------------------------------------------------------------------

TEST_CASE("filmulate integration - multiple develop steps", "[golden][filmulate][integration]")
{
  // This tests a mini filmulation pipeline
  int height = 16;
  int width = 16;
  int rgbCols = width * 3;

  // Initialize all simulation matrices
  matrix<float> crystalRad(height, rgbCols);
  matrix<float> activeCrystalsPerPixel(height, rgbCols);
  matrix<float> silverSaltDensity(height, rgbCols);
  matrix<float> develConcentration(height, width);

  // Create initial exposure pattern
  for (int row = 0; row < height; row++) {
    for (int pcol = 0; pcol < width; pcol++) {
      // Create a gradient exposure
      float exposure = static_cast<float>(pcol) / static_cast<float>(width - 1);
      activeCrystalsPerPixel(row, pcol * 3 + 0) = 100.0f * exposure;
      activeCrystalsPerPixel(row, pcol * 3 + 1) = 100.0f * exposure;
      activeCrystalsPerPixel(row, pcol * 3 + 2) = 100.0f * exposure;
      silverSaltDensity(row, pcol * 3 + 0) = 1.0f;
      silverSaltDensity(row, pcol * 3 + 1) = 1.0f;
      silverSaltDensity(row, pcol * 3 + 2) = 1.0f;
    }
  }
  crystalRad = 0.0001f;
  develConcentration = 1.0f;

  float crystalGrowthConst = 0.00002f;
  float activeLayerThickness = 0.1f;
  float developerConsumptionConst = 0.0004f;
  float silverSaltConsumptionConst = 0.0001f;
  float sigmaConst = 0.002f;
  float pixelsPerMillimeter = 100.0f;
  float timestep = 1.0f;
  float reservoirDevelConcentration = 1.0f;
  float reservoirThickness = 10.0f;

  // Run 10 development steps with diffusion and agitation
  for (int step = 0; step < 10; step++) {
    develop(crystalRad,
      crystalGrowthConst,
      activeCrystalsPerPixel,
      silverSaltDensity,
      develConcentration,
      activeLayerThickness,
      developerConsumptionConst,
      silverSaltConsumptionConst,
      timestep);

    diffuse(develConcentration, sigmaConst, pixelsPerMillimeter, timestep);

    if (step % 3 == 0) {
      agitate(
        develConcentration, activeLayerThickness, reservoirDevelConcentration, reservoirThickness, pixelsPerMillimeter);
    }
  }

  // Verify crystals have grown (values should be larger than initial)
  float lowExpCrystal = crystalRad(0, 0);
  float highExpCrystal = crystalRad(0, rgbCols - 3);

  INFO("Low exposure crystal: " << lowExpCrystal << " High exposure: " << highExpCrystal);
  REQUIRE(lowExpCrystal >= 0.0001f);
  REQUIRE(highExpCrystal >= 0.0001f);

  std::string goldenPath = GOLDEN_DIR + "filmulate_integration.bin";

  matrix<float> golden;
  if (golden::loadGolden(golden, goldenPath)) {
    auto result = golden::compare(crystalRad, golden, 1e-5f, 1e-6f);
    INFO("Max diff: " << result.maxDiff << " Avg diff: " << result.avgDiff);
    REQUIRE(result.passed);
  } else {
    INFO("Creating golden file: " << goldenPath);
    REQUIRE(golden::saveGolden(crystalRad, goldenPath));
    SUCCEED("Created new golden file");
  }
}
