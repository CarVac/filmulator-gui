/*
 * Unit tests for curve functions in filmulator-gui/core/curves.cpp
 */
#include "filmSim.hpp"
#include <catch2/catch_all.hpp>

TEST_CASE("default_tonecurve boundary values", "[curves]")
{
  // Test that endpoints are correct
  REQUIRE(default_tonecurve(0.0f) == Catch::Approx(0.0f).margin(0.001f));
  REQUIRE(default_tonecurve(1.0f) == Catch::Approx(1.0f).margin(0.001f));
}

TEST_CASE("default_tonecurve lifts midtones", "[curves]")
{
  // The bezier curve with control point at (0.2, 1) should lift midtones
  float mid = default_tonecurve(0.5f);
  REQUIRE(mid > 0.5f);
  REQUIRE(mid < 1.0f);

  // Test a few more sample points are within valid range
  for (float x = 0.1f; x <= 0.9f; x += 0.1f) {
    float y = default_tonecurve(x);
    INFO("Input: " << x << " Output: " << y);
    REQUIRE(y >= 0.0f);
    REQUIRE(y <= 1.0f);
  }
}

TEST_CASE("default_tonecurve is monotonically increasing", "[curves]")
{
  // Verify the curve doesn't go backwards
  float prev = 0.0f;
  for (float x = 0.0f; x <= 1.0f; x += 0.01f) {
    float y = default_tonecurve(x);
    REQUIRE(y >= prev);
    prev = y;
  }
}

TEST_CASE("shadows_highlights with identity control points", "[curves]")
{
  // When control points are on the diagonal line, output should approximate input
  // Control points at (0.25, 0.25) and (0.75, 0.75) form a straight line
  float shadowsX = 0.25f;
  float shadowsY = 0.25f;
  float highlightsX = 0.75f;
  float highlightsY = 0.75f;

  REQUIRE(shadows_highlights(0.0f, shadowsX, shadowsY, highlightsX, highlightsY) == Catch::Approx(0.0f).margin(0.01f));
  REQUIRE(shadows_highlights(0.5f, shadowsX, shadowsY, highlightsX, highlightsY) == Catch::Approx(0.5f).margin(0.01f));
  REQUIRE(shadows_highlights(1.0f, shadowsX, shadowsY, highlightsX, highlightsY) == Catch::Approx(1.0f).margin(0.01f));
}

TEST_CASE("shadows_highlights lifted shadows", "[curves]")
{
  // Control point (0.25, 0.5) lifts shadows
  float shadowsX = 0.25f;
  float shadowsY = 0.5f;
  float highlightsX = 0.75f;
  float highlightsY = 0.75f;

  float darkValue = shadows_highlights(0.25f, shadowsX, shadowsY, highlightsX, highlightsY);
  // With lifted shadows, a 0.25 input should produce > 0.25 output
  REQUIRE(darkValue > 0.25f);
}

TEST_CASE("shadows_highlights compressed highlights", "[curves]")
{
  // Control point (0.75, 0.5) compresses highlights
  float shadowsX = 0.25f;
  float shadowsY = 0.25f;
  float highlightsX = 0.75f;
  float highlightsY = 0.5f;

  float brightValue = shadows_highlights(0.75f, shadowsX, shadowsY, highlightsX, highlightsY);
  // With compressed highlights, a 0.75 input should produce < 0.75 output
  REQUIRE(brightValue < 0.75f);
}

TEST_CASE("shadows_highlights boundary values", "[curves]")
{
  // Endpoints should always map to 0 and 1
  float shadowsX = 0.3f;
  float shadowsY = 0.4f;
  float highlightsX = 0.6f;
  float highlightsY = 0.7f;

  REQUIRE(shadows_highlights(0.0f, shadowsX, shadowsY, highlightsX, highlightsY) == Catch::Approx(0.0f).margin(0.01f));
  REQUIRE(shadows_highlights(1.0f, shadowsX, shadowsY, highlightsX, highlightsY) == Catch::Approx(1.0f).margin(0.01f));
}

TEST_CASE("slopeFromT helper function", "[curves][helpers]")
{
  // Test with known coefficients
  // For a simple linear case: x = t, we have A=0, B=0, C=1
  // slope = 1 / (3*0*t^2 + 2*0*t + 1) = 1
  REQUIRE(slopeFromT(0.5f, 0.0f, 0.0f, 1.0f) == Catch::Approx(1.0f));
}

TEST_CASE("xFromT helper function", "[curves][helpers]")
{
  // Test with simple polynomial: x = t (A=0, B=0, C=1, D=0)
  REQUIRE(xFromT(0.0f, 0.0f, 0.0f, 1.0f, 0.0f) == Catch::Approx(0.0f));
  REQUIRE(xFromT(0.5f, 0.0f, 0.0f, 1.0f, 0.0f) == Catch::Approx(0.5f));
  REQUIRE(xFromT(1.0f, 0.0f, 0.0f, 1.0f, 0.0f) == Catch::Approx(1.0f));

  // Test with cubic: x = t^3 (A=1, B=0, C=0, D=0)
  REQUIRE(xFromT(0.5f, 1.0f, 0.0f, 0.0f, 0.0f) == Catch::Approx(0.125f));
}

TEST_CASE("yFromT helper function", "[curves][helpers]")
{
  // Same structure as xFromT - test simple polynomial y = t
  REQUIRE(yFromT(0.0f, 0.0f, 0.0f, 1.0f, 0.0f) == Catch::Approx(0.0f));
  REQUIRE(yFromT(0.5f, 0.0f, 0.0f, 1.0f, 0.0f) == Catch::Approx(0.5f));
  REQUIRE(yFromT(1.0f, 0.0f, 0.0f, 1.0f, 0.0f) == Catch::Approx(1.0f));
}
