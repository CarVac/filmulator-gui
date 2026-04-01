#include "PrefilmulationStage.h"
#include "../filmSim.hpp"
#include <vector>
// oklab_to_raw is in colorSpaces.cpp but declared in filmSim.hpp
#include <QDir>
#include <QStandardPaths>
#include <cmath>
#include <iostream>
#include <lensfun/lensfun.h>

using namespace std;

namespace Pipeline {

std::optional<RawImage>
  PrefilmulationStage::process(const RawImage &input, const PrefilmParams &params, PipelineContext &context)
{
  if (context.isAborted()) return std::nullopt;

  RawImage output = input;

  matrix<float> prefilm_input_image;

  if (input.isOklab) {
    matrix<float> temp_oklab = input.data;
    oklab_to_raw(temp_oklab, prefilm_input_image, input.camToRGB);
  } else {
    prefilm_input_image = input.data;
  }

  int height = prefilm_input_image.nr();
  int width = prefilm_input_image.nc() / 3;

  // 2. Lensfun

  lfDatabase *ldb = lf_db_new();
  QString dirstr = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
  dirstr.append("/filmulator/version_2");
  ldb->Load(dirstr.toStdString().c_str());

  std::string camName = params.cameraName.toStdString();
  const lfCamera *camera = NULL;
  const lfCamera **cameraList = ldb->FindCamerasExt(NULL, camName.c_str());

  float rotationAngle = params.rotationAngle * 3.1415926535 / 180;
  if (params.rotationAngle <= -49) { rotationAngle = 0; }

  bool lensfunGeometryCorrectionApplied = false;
  matrix<float> pre_film_image;

  if (cameraList) {
    const float cropFactor = cameraList[0]->CropFactor;
    QString tempLensName = params.lensName;
    if (tempLensName.length() > 0) {
      if (tempLensName.front() == '\\') {
        tempLensName.remove(0, 1);
      } else {
        camera = cameraList[0];
      }
    }
    std::string lensName = tempLensName.toStdString();
    const lfLens *lens = NULL;
    const lfLens **lensList = ldb->FindLenses(camera, NULL, lensName.c_str());

    if (lensList) {
      lens = lensList[0];
      lfModifier *mod = new lfModifier(lens, cropFactor, width, height);

      int flags = 0;
      if (params.lensfunCA && !input.isMonochrome) { flags |= LF_MODIFY_TCA; }
      if (params.lensfunVignetting) { flags |= LF_MODIFY_VIGNETTING; }
      if (params.lensfunDistortion) { flags |= LF_MODIFY_DISTORTION | LF_MODIFY_SCALE; }

      float scale = (flags & LF_MODIFY_SCALE) ? mod->GetAutoScale(false) : 1.0f;
      mod->Initialize(
        lens, LF_PF_F32, params.focalLength, params.fnumber, 1000.0f, scale, LF_RECTILINEAR, flags, false);

      if (params.lensfunVignetting) {
#pragma omp parallel for
        for (int row = 0; row < height; row++) {
          mod->ApplyColorModification(prefilm_input_image[row], 0.0f, row, width, 1, LF_CR_3(RED, GREEN, BLUE), width);
        }
      }

      if (params.lensfunCA || params.lensfunDistortion) {
        lensfunGeometryCorrectionApplied = true;
        float maxOvershootDistance = 1.0f;
        float semiwidth = (width - 1) / 2.0f;
        float semiheight = (height - 1) / 2.0f;
        int listWidth = width * 2 * 3;

#pragma omp parallel for reduction(max : maxOvershootDistance)
        for (int row = 0; row < height; row++) {
          std::vector<float> positionList(listWidth);
          if (mod->ApplySubpixelGeometryDistortion(0.0f, row, width, 1, positionList.data())) {
            for (int col = 0; col < width; col++) {
              int listIndex = col * 2 * 3;
              for (int c = 0; c < 3; c++) {
                float coordX = positionList[listIndex + 2 * c] - semiwidth;
                float coordY = positionList[listIndex + 2 * c + 1] - semiheight;
                float rotatedX = coordX * cos(rotationAngle) - coordY * sin(rotationAngle);
                float rotatedY = coordX * sin(rotationAngle) + coordY * cos(rotationAngle);

                float overshoot = 1.0f;
                if (abs(rotatedX) > semiwidth) overshoot = max(abs(rotatedX) / semiwidth, overshoot);
                if (abs(rotatedY) > semiheight) overshoot = max(abs(rotatedY) / semiheight, overshoot);
                if (overshoot > maxOvershootDistance) maxOvershootDistance = overshoot;
              }
            }
          }
        }

        pre_film_image.set_size(height, width * 3);
#pragma omp parallel for
        for (int row = 0; row < height; row++) {
          std::vector<float> positionList(listWidth);
          if (mod->ApplySubpixelGeometryDistortion(0.0f, row, width, 1, positionList.data())) {
            for (int col = 0; col < width; col++) {
              int listIndex = col * 2 * 3;
              for (int c = 0; c < 3; c++) {
                float coordX = positionList[listIndex + 2 * c] - semiwidth;
                float coordY = positionList[listIndex + 2 * c + 1] - semiheight;

                float rotatedX =
                  (coordX * cos(rotationAngle) - coordY * sin(rotationAngle)) / maxOvershootDistance + semiwidth;
                float rotatedY =
                  (coordX * sin(rotationAngle) + coordY * cos(rotationAngle)) / maxOvershootDistance + semiheight;

                int sX = max(0, min(width - 1, int(floor(rotatedX)))) * 3 + c;
                int eX = max(0, min(width - 1, int(ceil(rotatedX)))) * 3 + c;
                int sY = max(0, min(height - 1, int(floor(rotatedY))));
                int eY = max(0, min(height - 1, int(ceil(rotatedY))));

                float notUsed;
                float eWX = modf(rotatedX, &notUsed);
                float eWY = modf(rotatedY, &notUsed);
                float sWX = 1 - eWX;
                float sWY = 1 - eWY;

                pre_film_image(row, col * 3 + c) =
                  prefilm_input_image(sY, sX) * sWY * sWX + prefilm_input_image(eY, sX) * eWY * sWX
                  + prefilm_input_image(sY, eX) * sWY * eWX + prefilm_input_image(eY, eX) * eWY * eWX;
              }
            }
          }
        }
      }

      delete mod;
      lf_free(lensList);
    }
    lf_free(cameraList);
  }

  if (ldb) lf_db_destroy(ldb);

  // 3. Simple Rotation (if lensfun not applied)
  if (!lensfunGeometryCorrectionApplied) {
    if (rotationAngle != 0.0f) {
      float maxOvershootDistance = 1.0f;
      float semiwidth = (width - 1) / 2.0f;
      float semiheight = (height - 1) / 2.0f;

      // check corners
      for (int row = 0; row < height; row += height - 1) {
        for (int col = 0; col < width; col += width - 1) {
          float coordX = col - semiwidth;
          float coordY = row - semiheight;
          float rotatedX = coordX * cos(rotationAngle) - coordY * sin(rotationAngle);
          float rotatedY = coordX * sin(rotationAngle) + coordY * cos(rotationAngle);
          float overshoot = 1.0f;
          if (abs(rotatedX) > semiwidth) overshoot = max(abs(rotatedX) / semiwidth, overshoot);
          if (abs(rotatedY) > semiheight) overshoot = max(abs(rotatedY) / semiheight, overshoot);
          if (overshoot > maxOvershootDistance) maxOvershootDistance = overshoot;
        }
      }

      pre_film_image.set_size(height, width * 3);

      for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
          float coordX = col - semiwidth;
          float coordY = row - semiheight;
          float rotatedX =
            (coordX * cos(rotationAngle) - coordY * sin(rotationAngle)) / maxOvershootDistance + semiwidth;
          float rotatedY =
            (coordX * sin(rotationAngle) + coordY * cos(rotationAngle)) / maxOvershootDistance + semiheight;

          int sX = max(0, min(width - 1, int(floor(rotatedX)))) * 3;
          int eX = max(0, min(width - 1, int(ceil(rotatedX)))) * 3;
          int sY = max(0, min(height - 1, int(floor(rotatedY))));
          int eY = max(0, min(height - 1, int(ceil(rotatedY))));

          float notUsed;
          float eWX = modf(rotatedX, &notUsed);
          float eWY = modf(rotatedY, &notUsed);
          float sWX = 1 - eWX;
          float sWY = 1 - eWY;

          for (int c = 0; c < 3; c++) {
            pre_film_image(row, col * 3 + c) =
              prefilm_input_image(sY, sX + c) * sWY * sWX + prefilm_input_image(eY, sX + c) * eWY * sWX
              + prefilm_input_image(sY, eX + c) * sWY * eWX + prefilm_input_image(eY, eX + c) * eWY * eWX;
          }
        }
      }
    } else {
      pre_film_image.swap(prefilm_input_image);
    }
  }

  // 4. Update Histogram
  if (context.histo == WithHisto) {
    CropParams cropParam = context.paramManager->claimCropParams();
    context.interface->updateHistPreFilm(pre_film_image,
      65535,
      cropParam.rotation,
      cropParam.cropHeight,
      cropParam.cropAspect,
      cropParam.cropHoffset,
      cropParam.cropVoffset);
  }

  // 5. Resize / Downscale
  output.data = pre_film_image;
  // It is now strictly raw data again (linear, likely), not Oklab
  output.isOklab = false;

  return output;
}

}// namespace Pipeline
