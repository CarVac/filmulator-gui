/*
 * Golden tests for curve functions
 *
 * Tests the film_like_curve function which applies tone curves to images
 * while maintaining hue stability.
 */
#include "../golden/golden_test_utils.hpp"
#include "filmSim.hpp"
#include "lut.hpp"
#include <catch2/catch_all.hpp>
#include <cmath>

// Use GOLDEN_DATA_DIR from CMake if defined, otherwise fall back to relative path
#ifdef GOLDEN_DATA_DIR
static const std::string GOLDEN_DIR = GOLDEN_DATA_DIR;
#else
static const std::string GOLDEN_DIR = "../tests/golden/";
#endif

// Helper to create a linear identity LUT
LUT<unsigned short> createIdentityLUT()
{
  LUT<unsigned short> lut;
  lut.setUnity();// Use unity mode - returns input unchanged
  return lut;
}

// Helper to create a gamma-like tonecurve LUT
LUT<unsigned short> createGammaLUT(float gamma)
{
  LUT<unsigned short> lut;
  lut.fill([gamma](unsigned short i) -> unsigned short {
    float normalized = static_cast<float>(i) / 65535.0f;
    float curved = std::pow(normalized, 1.0f / gamma);
    return static_cast<unsigned short>(curved * 65535.0f);
  });
  return lut;
}

// Helper to create the default tone curve LUT using default_tonecurve()
LUT<unsigned short> createDefaultTonecurveLUT()
{
  LUT<unsigned short> lut;
  lut.fill([](unsigned short i) -> unsigned short {
    float normalized = static_cast<float>(i) / 65535.0f;
    float curved = default_tonecurve(normalized);
    return static_cast<unsigned short>(curved * 65535.0f);
  });
  return lut;
}

TEST_CASE("film_like_curve with identity LUT", "[golden][curves]")
{
  // Create a small test image (gradient for varied values)
  matrix<unsigned short> input;
  golden::createRGBGradient(input, 16, 16, static_cast<unsigned short>(65535));

  matrix<unsigned short> output;
  LUT<unsigned short> identityLUT = createIdentityLUT();

  // Apply the curve
  film_like_curve(input, output, identityLUT);

  // With identity LUT, output should equal input
  auto result = golden::compare(output, input, 1.0f, 0.001f);

  INFO("Max diff: " << result.maxDiff << " Avg diff: " << result.avgDiff);
  REQUIRE(result.passed);
}

TEST_CASE("film_like_curve preserves solid neutrals", "[golden][curves]")
{
  // A neutral gray (R=G=B) should remain neutral after any tone curve
  matrix<unsigned short> input;
  unsigned short grayLevel = 32768;// Mid gray
  golden::createSolidRGB(input, 16, 16, grayLevel, grayLevel, grayLevel);

  matrix<unsigned short> output;
  LUT<unsigned short> gammaLUT = createGammaLUT(2.2f);

  film_like_curve(input, output, gammaLUT);

  // Check all pixels remain neutral (R == G == B)
  int width = 16;
  for (int row = 0; row < 16; row++) {
    for (int x = 0; x < width; x++) {
      unsigned short r = output(row, x * 3 + 0);
      unsigned short g = output(row, x * 3 + 1);
      unsigned short b = output(row, x * 3 + 2);

      INFO("Pixel (" << row << "," << x << "): R=" << r << " G=" << g << " B=" << b);
      // Values should be equal or very close
      REQUIRE(std::abs(static_cast<int>(r) - static_cast<int>(g)) <= 1);
      REQUIRE(std::abs(static_cast<int>(g) - static_cast<int>(b)) <= 1);
    }
  }
}

TEST_CASE("film_like_curve output dimensions match input", "[golden][curves]")
{
  matrix<unsigned short> input(24, 48);// 24 rows, 48 cols (16 RGB pixels wide)
  input = 32768;// Fill with mid gray

  matrix<unsigned short> output;
  LUT<unsigned short> lut = createIdentityLUT();

  film_like_curve(input, output, lut);

  REQUIRE(output.nr() == input.nr());
  REQUIRE(output.nc() == input.nc());
}

TEST_CASE("film_like_curve lifts midtones with default tonecurve", "[golden][curves]")
{
  // Create a mid-gray image
  matrix<unsigned short> input;
  unsigned short midGray = 32768;
  golden::createSolidRGB(input, 8, 8, midGray, midGray, midGray);

  matrix<unsigned short> output;
  LUT<unsigned short> defaultLUT = createDefaultTonecurveLUT();

  film_like_curve(input, output, defaultLUT);

  // Output should be brighter than input (lifted midtones)
  unsigned short outR = output(0, 0);
  unsigned short outG = output(0, 1);
  unsigned short outB = output(0, 2);

  INFO("Input: " << midGray << " Output R: " << outR << " G: " << outG << " B: " << outB);
  REQUIRE(outR > midGray);
  REQUIRE(outG > midGray);
  REQUIRE(outB > midGray);
}

TEST_CASE("film_like_curve with saturated colors", "[golden][curves]")
{
  // Test with a saturated red - explicit casts to avoid type deduction issues
  matrix<unsigned short> input;
  unsigned short rVal = 65535;
  unsigned short gVal = 16384;
  unsigned short bVal = 8192;
  golden::createSolidRGB(input, 8, 8, rVal, gVal, bVal);

  matrix<unsigned short> output;
  LUT<unsigned short> gammaLUT = createGammaLUT(2.2f);

  film_like_curve(input, output, gammaLUT);

  // The output should still have R > G > B ordering (hue preservation)
  unsigned short outR = output(0, 0);
  unsigned short outG = output(0, 1);
  unsigned short outB = output(0, 2);

  INFO("Output R: " << outR << " G: " << outG << " B: " << outB);
  REQUIRE(outR > outG);
  REQUIRE(outG > outB);
}

// This test generates and saves a golden file for regression testing
TEST_CASE("film_like_curve golden file comparison", "[golden][curves][!mayfail]")
{
  // Create a colorful test pattern
  matrix<unsigned short> input;
  int height = 16;
  int width = 16;
  int rows = height;
  int cols = width * 3;
  input.set_size(rows, cols);

  // Create a color test pattern: each row is different saturation
  for (int row = 0; row < height; row++) {
    float satFactor = static_cast<float>(row) / static_cast<float>(height - 1);
    for (int x = 0; x < width; x++) {
      float t = static_cast<float>(x) / static_cast<float>(width - 1);
      unsigned short base = static_cast<unsigned short>(t * 65535);
      unsigned short sat = static_cast<unsigned short>((1.0f - satFactor) * base);

      input(row, x * 3 + 0) = base;// R varies across x
      input(row, x * 3 + 1) = sat;// G varies with saturation
      input(row, x * 3 + 2) = 32768;// B constant
    }
  }

  matrix<unsigned short> output;
  LUT<unsigned short> defaultLUT = createDefaultTonecurveLUT();

  film_like_curve(input, output, defaultLUT);

  std::string goldenPath = GOLDEN_DIR + "film_like_curve_default.bin";

  // Try to compare to existing golden file
  matrix<unsigned short> golden;
  if (golden::loadGolden(golden, goldenPath)) {
    auto result = golden::compare(output, golden, 1.0f, 0.01f);
    INFO("Max diff: " << result.maxDiff << " Avg diff: " << result.avgDiff);
    REQUIRE(result.passed);
  } else {
    // Golden file doesn't exist - save it
    INFO("Golden file not found. Creating: " << goldenPath);
    REQUIRE(golden::saveGolden(output, goldenPath));
    SUCCEED("Created new golden file");
  }
}
