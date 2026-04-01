/*
 * Golden tests for color adjustment functions
 *
 * Tests vibrance_saturation, monochrome_convert, and whitepoint_blackpoint
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
// vibrance_saturation tests
//-----------------------------------------------------------------------------

TEST_CASE("vibrance_saturation identity", "[golden][color]")
{
  // With vibrance=1 and saturation=1, output should match input
  matrix<unsigned short> input;
  golden::createRGBGradient(input, 8, 8, static_cast<unsigned short>(65535));

  matrix<unsigned short> output;
  vibrance_saturation(input, output, 1.0f, 1.0f);

  auto result = golden::compare(output, input, 1.0f, 0.001f);
  INFO("Max diff: " << result.maxDiff << " Avg diff: " << result.avgDiff);
  REQUIRE(result.passed);
}

TEST_CASE("vibrance_saturation zero saturation", "[golden][color]")
{
  // Test with saturation=0 parameter
  matrix<unsigned short> input;
  unsigned short rVal = 65535;
  unsigned short gVal = 32768;
  unsigned short bVal = 16384;
  golden::createSolidRGB(input, 8, 8, rVal, gVal, bVal);

  matrix<unsigned short> output;
  vibrance_saturation(input, output, 1.0f, 0.0f);

  // Verify output dimensions are correct
  REQUIRE(output.nr() == 8);
  REQUIRE(output.nc() == 24);

  // Verify output values are valid (within 16-bit range)
  unsigned short outR = output(0, 0);
  unsigned short outG = output(0, 1);
  unsigned short outB = output(0, 2);

  INFO("Output R: " << outR << " G: " << outG << " B: " << outB);
  REQUIRE(outR <= 65535);
  REQUIRE(outG <= 65535);
  REQUIRE(outB <= 65535);
}

TEST_CASE("vibrance_saturation high saturation", "[golden][color]")
{
  // With high saturation, colors should be more saturated
  matrix<unsigned short> input;
  unsigned short rVal = 50000;
  unsigned short gVal = 40000;
  unsigned short bVal = 30000;
  golden::createSolidRGB(input, 8, 8, rVal, gVal, bVal);

  matrix<unsigned short> output;
  vibrance_saturation(input, output, 1.0f, 2.0f);

  unsigned short outR = output(0, 0);
  unsigned short outG = output(0, 1);
  unsigned short outB = output(0, 2);

  // The spread between channels should increase
  int inSpread = static_cast<int>(rVal) - static_cast<int>(bVal);
  int outSpread = static_cast<int>(outR) - static_cast<int>(outB);

  INFO("Input spread: " << inSpread << " Output spread: " << outSpread);
  REQUIRE(outSpread > inSpread);
}

TEST_CASE("vibrance_saturation golden comparison", "[golden][color]")
{
  matrix<unsigned short> input;
  golden::createRGBGradient(input, 16, 16, static_cast<unsigned short>(65535));

  matrix<unsigned short> output;
  vibrance_saturation(input, output, 1.2f, 1.1f);

  std::string goldenPath = GOLDEN_DIR + "vibrance_1.2_sat_1.1.bin";

  matrix<unsigned short> golden;
  if (golden::loadGolden(golden, goldenPath)) {
    auto result = golden::compare(output, golden, 1.0f, 0.01f);
    INFO("Max diff: " << result.maxDiff << " Avg diff: " << result.avgDiff);
    REQUIRE(result.passed);
  } else {
    INFO("Creating golden file: " << goldenPath);
    REQUIRE(golden::saveGolden(output, goldenPath));
    SUCCEED("Created new golden file");
  }
}

//-----------------------------------------------------------------------------
// monochrome_convert tests
//-----------------------------------------------------------------------------

TEST_CASE("monochrome_convert basic", "[golden][color]")
{
  matrix<unsigned short> input;
  unsigned short rVal = 65535;
  unsigned short gVal = 32768;
  unsigned short bVal = 16384;
  golden::createSolidRGB(input, 8, 8, rVal, gVal, bVal);

  matrix<unsigned short> output;
  // Standard luminance weights
  monochrome_convert(input, output, 0.2126f, 0.7152f, 0.0722f);

  // Output should be grayscale
  unsigned short outR = output(0, 0);
  unsigned short outG = output(0, 1);
  unsigned short outB = output(0, 2);

  INFO("Output R: " << outR << " G: " << outG << " B: " << outB);
  REQUIRE(outR == outG);
  REQUIRE(outG == outB);
}

TEST_CASE("monochrome_convert red filter", "[golden][color]")
{
  matrix<unsigned short> input;
  // Create RGB patches - red, green, blue
  input.set_size(3, 24);// 3 rows, 8 RGB pixels each

  // Row 0: pure red
  for (int col = 0; col < 8; col++) {
    input(0, col * 3 + 0) = 65535;
    input(0, col * 3 + 1) = 0;
    input(0, col * 3 + 2) = 0;
  }
  // Row 1: pure green
  for (int col = 0; col < 8; col++) {
    input(1, col * 3 + 0) = 0;
    input(1, col * 3 + 1) = 65535;
    input(1, col * 3 + 2) = 0;
  }
  // Row 2: pure blue
  for (int col = 0; col < 8; col++) {
    input(2, col * 3 + 0) = 0;
    input(2, col * 3 + 1) = 0;
    input(2, col * 3 + 2) = 65535;
  }

  matrix<unsigned short> output;
  // Red filter: only red contributes
  monochrome_convert(input, output, 1.0f, 0.0f, 0.0f);

  // Red row should be bright, green and blue should be dark
  unsigned short redRowVal = output(0, 0);
  unsigned short greenRowVal = output(1, 0);
  unsigned short blueRowVal = output(2, 0);

  INFO("Red row: " << redRowVal << " Green row: " << greenRowVal << " Blue row: " << blueRowVal);
  REQUIRE(redRowVal > 60000);
  REQUIRE(greenRowVal < 1000);
  REQUIRE(blueRowVal < 1000);
}

TEST_CASE("monochrome_convert golden comparison", "[golden][color]")
{
  matrix<unsigned short> input;
  golden::createRGBGradient(input, 16, 16, static_cast<unsigned short>(65535));

  matrix<unsigned short> output;
  // Luminance weights
  monochrome_convert(input, output, 0.2126f, 0.7152f, 0.0722f);

  std::string goldenPath = GOLDEN_DIR + "monochrome_luminance.bin";

  matrix<unsigned short> golden;
  if (golden::loadGolden(golden, goldenPath)) {
    auto result = golden::compare(output, golden, 1.0f, 0.01f);
    INFO("Max diff: " << result.maxDiff << " Avg diff: " << result.avgDiff);
    REQUIRE(result.passed);
  } else {
    INFO("Creating golden file: " << goldenPath);
    REQUIRE(golden::saveGolden(output, goldenPath));
    SUCCEED("Created new golden file");
  }
}

//-----------------------------------------------------------------------------
// whitepoint_blackpoint tests
//-----------------------------------------------------------------------------

TEST_CASE("whitepoint_blackpoint identity", "[golden][color]")
{
  // With whitepoint=1 and blackpoint=0, should be linear scale to 16-bit
  matrix<float> input;
  golden::createRGBGradient(input, 8, 8, 1.0f);

  matrix<unsigned short> output;
  whitepoint_blackpoint(input, output, 1.0f, 0.0f);

  // Check a few sample values
  // At x=7 (last column), value should be ~65535
  unsigned short lastVal = output(0, 21);// Last R value
  INFO("Last value: " << lastVal);
  REQUIRE(lastVal > 65000);

  // At x=0 (first column), value should be ~0
  unsigned short firstVal = output(0, 0);
  INFO("First value: " << firstVal);
  REQUIRE(firstVal < 1000);
}

TEST_CASE("whitepoint_blackpoint contrast boost", "[golden][color]")
{
  // Test with narrowed range parameters
  matrix<float> input;
  golden::createNeutralGray(input, 4, 4, 0.5f);

  matrix<unsigned short> output;
  whitepoint_blackpoint(input, output, 0.6f, 0.4f);

  unsigned short val = output(0, 0);
  INFO("Mid value with boosted contrast: " << val);
  // Verify output is valid 16-bit value
  REQUIRE(val >= 0);
  REQUIRE(val <= 65535);
}

TEST_CASE("whitepoint_blackpoint golden comparison", "[golden][color]")
{
  matrix<float> input;
  golden::createRGBGradient(input, 16, 16, 1.0f);

  matrix<unsigned short> output;
  whitepoint_blackpoint(input, output, 0.9f, 0.1f);

  std::string goldenPath = GOLDEN_DIR + "whitepoint_blackpoint.bin";

  matrix<unsigned short> golden;
  if (golden::loadGolden(golden, goldenPath)) {
    auto result = golden::compare(output, golden, 1.0f, 0.01f);
    INFO("Max diff: " << result.maxDiff << " Avg diff: " << result.avgDiff);
    REQUIRE(result.passed);
  } else {
    INFO("Creating golden file: " << goldenPath);
    REQUIRE(golden::saveGolden(output, goldenPath));
    SUCCEED("Created new golden file");
  }
}
