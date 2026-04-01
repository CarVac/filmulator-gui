#ifndef PIPELINE_RAWIMAGE_H
#define PIPELINE_RAWIMAGE_H

#include "../matrix.hpp"
#include <exiv2/exiv2.hpp>

namespace Pipeline {

struct RawImage
{
  matrix<float> data;// Black-subtracted raw data
  Exiv2::ExifData exif;

  // CFA / Geometry
  unsigned cfa[2][2];
  unsigned xtrans[6][6];
  int maxXtrans = 0;

  // Color Matrix & WB
  float camToRGB[3][3];
  float xyzToCam[3][3];
  float rPreMul = 1.0f, gPreMul = 1.0f, bPreMul = 1.0f;
  float rCamMul = 1.0f, gCamMul = 1.0f, bCamMul = 1.0f;
  float rUserMul = 1.0f, gUserMul = 1.0f, bUserMul = 1.0f;// Output of WB optimization
  float colorMaxValue[3];
  float maxValue = 65535.0f;

  // Flags
  bool isCR3 = false;
  bool isSraw = false;
  bool isMonochrome = false;
  bool isNikonSraw = false;
  bool isFloat = false;
  bool isDemosaiced = false;
  bool isOklab = false;
};

}// namespace Pipeline

#endif// PIPELINE_RAWIMAGE_H
