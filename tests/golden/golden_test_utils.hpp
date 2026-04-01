/*
 * Golden test infrastructure for Filmulator
 *
 * Provides utilities for:
 * - Comparing output images to known-good "golden" reference files
 * - Saving new golden references
 * - Generating synthetic test images
 */
#ifndef GOLDEN_TEST_UTILS_HPP
#define GOLDEN_TEST_UTILS_HPP

#include "matrix.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

namespace golden {

//-----------------------------------------------------------------------------
// Comparison Result Structure
//-----------------------------------------------------------------------------

struct CompareResult
{
  float maxDiff;// Maximum difference between any two corresponding pixels
  float avgDiff;// Average difference across all pixels
  bool passed;// Whether the comparison passed the tolerance thresholds
  int numDifferences;// Count of pixels exceeding tolerance

  // Default thresholds for passing
  static constexpr float DEFAULT_MAX_TOLERANCE = 1.0f;// Max diff allowed as fraction
  static constexpr float DEFAULT_AVG_TOLERANCE = 0.01f;// 1% average diff
};

//-----------------------------------------------------------------------------
// Core Functions: Save and Load Golden Files
//-----------------------------------------------------------------------------

// Save a matrix to a binary golden file
// Format: [int rows][int cols][data as raw bytes]
template<typename T> bool saveGolden(const matrix<T> &img, const std::string &path)
{
  std::ofstream file(path, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Failed to open file for writing: " << path << std::endl;
    return false;
  }

  int rows = img.nr();
  int cols = img.nc();
  file.write(reinterpret_cast<const char *>(&rows), sizeof(int));
  file.write(reinterpret_cast<const char *>(&cols), sizeof(int));

  // Write data row by row
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      T val = img(r, c);
      file.write(reinterpret_cast<const char *>(&val), sizeof(T));
    }
  }

  file.close();
  return true;
}

// Load a matrix from a binary golden file
template<typename T> bool loadGolden(matrix<T> &img, const std::string &path)
{
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Failed to open file for reading: " << path << std::endl;
    return false;
  }

  int rows, cols;
  file.read(reinterpret_cast<char *>(&rows), sizeof(int));
  file.read(reinterpret_cast<char *>(&cols), sizeof(int));

  img.set_size(rows, cols);

  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      T val;
      file.read(reinterpret_cast<char *>(&val), sizeof(T));
      img(r, c) = val;
    }
  }

  file.close();
  return true;
}

//-----------------------------------------------------------------------------
// Comparison Functions
//-----------------------------------------------------------------------------

// Compare two matrices element-by-element
template<typename T>
CompareResult compare(const matrix<T> &output,
  const matrix<T> &golden,
  float maxTolerance = CompareResult::DEFAULT_MAX_TOLERANCE,
  float avgTolerance = CompareResult::DEFAULT_AVG_TOLERANCE)
{
  CompareResult result = { 0.0f, 0.0f, true, 0 };

  // Check dimensions match
  if (output.nr() != golden.nr() || output.nc() != golden.nc()) {
    result.passed = false;
    result.maxDiff = std::numeric_limits<float>::max();
    return result;
  }

  int rows = output.nr();
  int cols = output.nc();
  int totalPixels = rows * cols;

  if (totalPixels == 0) {
    return result;// Empty matrices are considered equal
  }

  double sumDiff = 0.0;
  float maxDiff = 0.0f;
  int numDiff = 0;

  // Compare each pixel
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      float diff = std::abs(static_cast<float>(output(r, c)) - static_cast<float>(golden(r, c)));
      sumDiff += diff;
      if (diff > maxDiff) { maxDiff = diff; }
      if (diff > maxTolerance) { numDiff++; }
    }
  }

  result.maxDiff = maxDiff;
  result.avgDiff = static_cast<float>(sumDiff / totalPixels);
  result.numDifferences = numDiff;
  result.passed = (result.maxDiff <= maxTolerance && result.avgDiff <= avgTolerance);

  return result;
}

// Compare output to a golden file on disk
template<typename T>
CompareResult compareToGolden(const matrix<T> &output,
  const std::string &goldenPath,
  float maxTolerance = CompareResult::DEFAULT_MAX_TOLERANCE,
  float avgTolerance = CompareResult::DEFAULT_AVG_TOLERANCE)
{
  matrix<T> golden;
  if (!loadGolden(golden, goldenPath)) {
    CompareResult result = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), false, -1 };
    return result;
  }
  return compare(output, golden, maxTolerance, avgTolerance);
}

//-----------------------------------------------------------------------------
// Synthetic Test Image Generators
//-----------------------------------------------------------------------------

// Create a solid color image (useful for testing basic operations)
// For RGB images, cols should be 3*width (R,G,B interleaved)
template<typename T> void createSolidImage(matrix<T> &img, int rows, int cols, T value)
{
  img.set_size(rows, cols);
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) { img(r, c) = value; }
  }
}

// Create a solid RGB image
template<typename T> void createSolidRGB(matrix<T> &img, int height, int width, T r, T g, T b)
{
  int rows = height;
  int cols = width * 3;// RGB interleaved
  img.set_size(rows, cols);

  for (int row = 0; row < rows; row++) {
    for (int x = 0; x < width; x++) {
      img(row, x * 3 + 0) = r;
      img(row, x * 3 + 1) = g;
      img(row, x * 3 + 2) = b;
    }
  }
}

// Create a horizontal gradient (value increases from left to right)
template<typename T> void createHorizontalGradient(matrix<T> &img, int rows, int cols, T minVal, T maxVal)
{
  img.set_size(rows, cols);
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      float t = static_cast<float>(c) / static_cast<float>(cols - 1);
      img(r, c) = static_cast<T>(minVal + t * (maxVal - minVal));
    }
  }
}

// Create a vertical gradient (value increases from top to bottom)
template<typename T> void createVerticalGradient(matrix<T> &img, int rows, int cols, T minVal, T maxVal)
{
  img.set_size(rows, cols);
  for (int r = 0; r < rows; r++) {
    float t = static_cast<float>(r) / static_cast<float>(rows - 1);
    T val = static_cast<T>(minVal + t * (maxVal - minVal));
    for (int c = 0; c < cols; c++) { img(r, c) = val; }
  }
}

// Create an RGB horizontal gradient (R,G,B increase from 0 to max)
template<typename T> void createRGBGradient(matrix<T> &img, int height, int width, T maxVal)
{
  int rows = height;
  int cols = width * 3;
  img.set_size(rows, cols);

  for (int row = 0; row < rows; row++) {
    for (int x = 0; x < width; x++) {
      float t = static_cast<float>(x) / static_cast<float>(width - 1);
      T val = static_cast<T>(t * maxVal);
      img(row, x * 3 + 0) = val;// R
      img(row, x * 3 + 1) = val;// G
      img(row, x * 3 + 2) = val;// B
    }
  }
}

// Create a checkerboard pattern (useful for testing spatial operations)
template<typename T> void createCheckerboard(matrix<T> &img, int rows, int cols, int cellSize, T color1, T color2)
{
  img.set_size(rows, cols);
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      bool isColor1 = ((r / cellSize) + (c / cellSize)) % 2 == 0;
      img(r, c) = isColor1 ? color1 : color2;
    }
  }
}

// Create neutral gray RGB patches (useful for white balance testing)
// Creates a 18% gray image (in linear space: 0.18, or ~0.466 gamma-encoded)
template<typename T> void createNeutralGray(matrix<T> &img, int height, int width, T grayLevel)
{
  createSolidRGB(img, height, width, grayLevel, grayLevel, grayLevel);
}

}// namespace golden

#endif// GOLDEN_TEST_UTILS_HPP
