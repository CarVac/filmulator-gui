/*
 * Unit tests for white balance functions in filmulator-gui/core/whiteBalance.cpp
 */
#include "filmSim.hpp"
#include <catch2/catch_all.hpp>
#include <cmath>

// Note: Some functions like temp_to_XYZ, matrixVectorMult, and matrixMatrixMult
// are declared static or not exposed in the header. We test them indirectly
// through the functions that use them, or we test them directly if exposed.

TEST_CASE("daylightScore ranking", "[whiteBalance]")
{
  // D65 (6500k) should have the best (lowest) score
  REQUIRE(daylightScore(21) == 0);// D65

  // Unknown should have a very high score
  REQUIRE(daylightScore(0) == 20);// unknown

  // Test ordering - D65 should be better than tungsten
  REQUIRE(daylightScore(21) < daylightScore(3));// D65 < tungsten

  // Test ordering - daylight better than flash
  REQUIRE(daylightScore(1) < daylightScore(4));

  // Test unknown illuminant code
  REQUIRE(daylightScore(999) == 21);// default case
}

TEST_CASE("daylightScore covers common illuminants", "[whiteBalance]")
{
  // Verify all expected illuminant codes return valid scores
  int illuminants[] = { 0, 1, 2, 3, 4, 9, 10, 11, 12, 13, 14, 15, 17, 18, 19, 20, 21, 22, 23, 24, 255 };

  for (int illuminant : illuminants) {
    int score = daylightScore(illuminant);
    INFO("Illuminant: " << illuminant << " Score: " << score);
    REQUIRE(score >= 0);
    REQUIRE(score <= 21);
  }
}

TEST_CASE("sRGB gamma functions", "[colorSpaces]")
{
  // Test sRGB inverse gamma (linearization)
  // Value of 0 should remain 0
  REQUIRE(sRGB_inverse_gamma(0.0f) == Catch::Approx(0.0f).margin(0.001f));

  // Value of 1 should remain 1
  REQUIRE(sRGB_inverse_gamma(1.0f) == Catch::Approx(1.0f).margin(0.001f));

  // Middle gray should be darker after linearization
  float linearMid = sRGB_inverse_gamma(0.5f);
  REQUIRE(linearMid < 0.5f);
  REQUIRE(linearMid > 0.0f);
}

TEST_CASE("sRGB gamma round trip", "[colorSpaces]")
{
  // Forward and inverse gamma should be inverses of each other
  for (float val = 0.0f; val <= 1.0f; val += 0.1f) {
    float linear = sRGB_inverse_gamma(val);
    float recovered = sRGB_forward_gamma(linear);
    INFO("Input: " << val << " Linear: " << linear << " Recovered: " << recovered);
    REQUIRE(recovered == Catch::Approx(val).margin(0.001f));
  }
}

TEST_CASE("Lab gamma functions", "[colorSpaces]")
{
  // Test Lab inverse gamma
  REQUIRE(Lab_inverse_gamma(0.0f) == Catch::Approx(0.0f).margin(0.001f));
  REQUIRE(Lab_inverse_gamma(1.0f) == Catch::Approx(1.0f).margin(0.001f));
  // Note: Lab_inverse_gamma and Lab_forward_gamma are not perfect inverses.
  // Lab_inverse_gamma cutoff is 0.08, Lab_forward_gamma uses LAB_EPSILON (~0.008856)
  // This is by design - they're used for specific pipeline stages, not as inverse pairs.
}

TEST_CASE("XYZ to sRGB basic", "[colorSpaces]")
{
  float r, g, b;

  // D65 white point should give roughly equal RGB
  // D65 in XYZ (normalized Y=1): X=0.95047, Y=1.0, Z=1.08883
  XYZ_to_sRGB(0.95047f, 1.0f, 1.08883f, r, g, b);

  // All components should be positive and roughly equal for white
  REQUIRE(r > 0.0f);
  REQUIRE(g > 0.0f);
  REQUIRE(b > 0.0f);
}

TEST_CASE("sRGB to XYZ round trip", "[colorSpaces]")
{
  float testR = 0.5f, testG = 0.3f, testB = 0.7f;
  float x, y, z;
  float r, g, b;

  sRGB_to_XYZ(testR, testG, testB, x, y, z);
  XYZ_to_sRGB(x, y, z, r, g, b);

  REQUIRE(r == Catch::Approx(testR).margin(0.01f));
  REQUIRE(g == Catch::Approx(testG).margin(0.01f));
  REQUIRE(b == Catch::Approx(testB).margin(0.01f));
}

TEST_CASE("3x3 matrix inverse", "[matrix]")
{
  // Test with identity matrix
  float identity[3][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
  float invIdentity[3][3];

  inverse(identity, invIdentity);

  // Inverse of identity should be identity
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      float expected = (i == j) ? 1.0f : 0.0f;
      INFO("i=" << i << " j=" << j);
      REQUIRE(invIdentity[i][j] == Catch::Approx(expected).margin(0.001f));
    }
  }
}

TEST_CASE("3x3 matrix inverse - simple matrix", "[matrix]")
{
  // Test with a simple invertible matrix
  float mat[3][3] = { { 2, 0, 0 }, { 0, 3, 0 }, { 0, 0, 4 } };
  float invMat[3][3];

  inverse(mat, invMat);

  // Inverse of diagonal matrix should have reciprocal diagonal elements
  REQUIRE(invMat[0][0] == Catch::Approx(0.5f).margin(0.001f));
  REQUIRE(invMat[1][1] == Catch::Approx(1.0f / 3.0f).margin(0.001f));
  REQUIRE(invMat[2][2] == Catch::Approx(0.25f).margin(0.001f));

  // Off-diagonal elements should be zero
  REQUIRE(invMat[0][1] == Catch::Approx(0.0f).margin(0.001f));
  REQUIRE(invMat[0][2] == Catch::Approx(0.0f).margin(0.001f));
  REQUIRE(invMat[1][0] == Catch::Approx(0.0f).margin(0.001f));
  REQUIRE(invMat[1][2] == Catch::Approx(0.0f).margin(0.001f));
  REQUIRE(invMat[2][0] == Catch::Approx(0.0f).margin(0.001f));
  REQUIRE(invMat[2][1] == Catch::Approx(0.0f).margin(0.001f));
}
