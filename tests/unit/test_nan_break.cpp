#include "../../filmulator-gui/core/debug_utils.h"
#include "../../filmulator-gui/core/matrix.hpp"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <limits>

#ifdef ENABLE_NAN_TRAPPING

TEST_CASE("NaN Break Verification", "[nan][debug]")
{
  float nan_val = std::numeric_limits<float>::quiet_NaN();
  std::cout << "About to trigger BREAK_ON_NAN with value: " << nan_val << std::endl;

  // This is expected to trigger a debugger break or abort if ENABLE_NAN_TRAPPING is ON
  BREAK_ON_NAN(nan_val);
  SUCCEED("BREAK_ON_NAN was called");
}

TEST_CASE("NaN Matrix Scan Verification", "[nan][debug]")
{
  matrix<float> m(10, 10);
  m = 1.0f;

  SECTION("No NaN - should pass")
  {
    SCAN_MATRIX_FOR_NAN(m, "test_matrix_clean");
    SUCCEED("No NaN detected");
  }

  SECTION("With NaN - should trap")
  {
    m(5, 5) = std::numeric_limits<float>::quiet_NaN();
    std::cout << "About to scan matrix with NaN..." << std::endl;
    SCAN_MATRIX_FOR_NAN(m, "test_matrix_dirty");
    FAIL("SCAN_MATRIX_FOR_NAN did not trap as expected.");
  }
}

#endif
