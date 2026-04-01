#include "imagePipeline.h"
#include "../database/camconst.h"
#include "../database/exifFunctions.h"
#include "filmSim.hpp"
#include "logging.h"
#include "nlmeans/nlmeans.hpp"
#include "rawtherapee/rt_routines.h"
#include <QDir>
#include <QStandardPaths>

#include "debug_utils.h"
#include "pipeline/ChromaNRStage.h"
#include "pipeline/DemosaicStage.h"
#include "pipeline/ImpulseNRStage.h"
#include "pipeline/LoadStage.h"
#include "pipeline/NlmeansNRStage.h"
#include "pipeline/PostDemosaicStage.h"
#include "pipeline/PrefilmulationStage.h"

ImagePipeline::ImagePipeline(Cache cacheIn, Histo histoIn, QuickQuality qualityIn)
{
  cache = cacheIn;
  histo = histoIn;
  quality = qualityIn;
  valid = Valid::none;
  filename = "";

  // initialize lensfun db
  QString dirstr = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
  dirstr.append("/filmulator/version_1/");
  // cout << "ImagePipeline lensfun dirstring: " << dirstr.toStdString() << endl;
  QDir dir(dirstr);
  QStringList filters;
  filters << "*.xml";
  QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);

  // cout << "ImagePipeline initializing lensfun db" << endl;
  ldb = lf_db_new();
  if (!ldb) { cout << "ImagePipeline lensfun failed to create database!" << endl; }

  foreach (const QFileInfo &fileInfo, fileList) {
    const QString filename = fileInfo.absoluteFilePath();
    const std::string stdstring = filename.toStdString();
    // cout << "ImagePipeline lensfun loading file " << stdstring << endl;
    lfError loadError = ldb->Load(stdstring.c_str());
    if (loadError == LF_WRONG_FORMAT) {
      cout << "ImagePipeline lensfun loading file " << stdstring << endl;
      cout << "ImagePipeline lensfun database wrong format!" << endl;
    } else if (loadError == LF_NO_DATABASE) {
      cout << "ImagePipeline lensfun loading file " << stdstring << endl;
      cout << "ImagePipeline lensfun no database found!" << endl;
    } else if (loadError == LF_NO_ERROR) {
      // cout << "ImagePipeline lensfun database loaded" << endl;
    } else {
      cout << "ImagePipeline lensfun loading file " << stdstring << endl;
      cout << "ImagePipeline lensfun what happened? " << loadError << endl;
    }
  }

  completionTimes.resize(Valid::count);
  completionTimes[Valid::none] = 0;
  completionTimes[Valid::load] = 5;
  completionTimes[Valid::demosaic] = 30;
  completionTimes[Valid::postdemosaic] = 10;
  completionTimes[Valid::nrnlmeans] = 20;
  completionTimes[Valid::nrimpulse] = 10;
  completionTimes[Valid::nrchroma] = 20;
  completionTimes[Valid::prefilmulation] = 5;
  completionTimes[Valid::filmulation] = 50;
  completionTimes[Valid::blackwhite] = 10;
  completionTimes[Valid::colorcurve] = 10;
  rCamMul = gCamMul = bCamMul = 1.0f;
  rPreMul = gPreMul = bPreMul = 1.0f;
  rUserMul = gUserMul = bUserMul = 1.0f;
  maxValue = 65535.0f;
  for (int i = 0; i < 3; ++i) colorMaxValue[i] = 65535.0f;
  isSraw = isNikonSraw = isMonochrome = isCR3 = false;
  resolution = 0;
  progress = 0;
  cropHeight = cropAspect = cropHoffset = cropVoffset = 0;
  rotation = 0;
}

ImagePipeline::~ImagePipeline()
{
  if (ldb != NULL) { lf_db_destroy(ldb); }
}

// int ImagePipeline::libraw_callback(void *data, LibRaw_progress p, int
// iteration, int expected) but we only need data.
int ImagePipeline::libraw_callback(void *data, LibRaw_progress, int, int)
{
  AbortStatus abort;

  // Recover the param_manager from the data
  ParameterManager *pManager = static_cast<ParameterManager *>(data);
  // See whether to abort or not.
  abort = pManager->claimDemosaicAbort();
  if (abort == AbortStatus::restart) {
    return 1;// cancel processing
  } else {
    return 0;
  }
}

matrix<unsigned short> &ImagePipeline::processImage(ParameterManager *paramManager,
  Interface *interface_in,
  Exiv2::ExifData &exifOutput,
  const QString fileHash,// make this empty string if you don't want to mess
                         // around with validity
  ImagePipeline *stealVictim)// defaults to nullptr
{
  // Say that we've started processing to prevent cache status from changing..
  hasStartedProcessing = true;
  // Record when the function was requested. This is so that the function will
  // not give up
  //  until a given short time has elapsed.
  timeRequested = std::chrono::steady_clock::now();
  histoInterface = interface_in;

  // check that file requested matches the file associated with the parameter
  // manager
  if (fileHash != "") {
    QString paramIndex = paramManager->getImageIndex();
    paramIndex.truncate(32);
    if (fileHash != paramIndex) {
      FILM_WARN("processImage shuffle mismatch:  Requested Index: {}", fileHash.toStdString());
      FILM_WARN("processImage shuffle mismatch:  Parameter Index: {}", paramIndex.toStdString());
      FILM_WARN("processImage shuffle mismatch:  full pipeline?: {}", (quality == HighQuality));
      valid = none;
    }
    fileID = paramIndex;
  } else {
    QString paramIndex = paramManager->getImageIndex();
    paramIndex.truncate(32);
    fileID = paramIndex;
  }

  valid = paramManager->getValid();
  if (NoCache == cache || true == cacheEmpty) {
    valid = none;// we need to start fresh if nothing is going to be cached.
  }

  // If we are a high-res pipeline that's going to steal data, skip to
  // filmulation
  if (stealData) {
    if (stealVictim == nullptr) { FILM_ERROR("stealVictim should not be null!"); }
    valid = max(valid, prefilmulation);
    paramManager->setValid(valid);
  }

  // If we think we have valid image data,
  // check that the file last processed corresponds to the one requested.
  if (valid > none && !stealData) {
    // if something has been processed before, and we think it's valid
    // it had better be the same filename.
    if (paramManager->getFullFilename() != filename.toStdString()) {
      FILM_WARN("processImage paramManager filename doesn't match pipeline filename");
      FILM_WARN("processImage paramManager filename: {}", paramManager->getFullFilename());
      FILM_WARN("processImage pipeline filename:     {}", filename.toStdString());
      FILM_WARN("processImage setting validity to none due to filename");
      valid = none;
    }
  }

  FILM_INFO("ImagePipeline::processImage valid: {}", (int)valid);

  updateProgress(valid, 0.0f);
  PipelineContext context;
  context.interface = interface_in;
  context.paramManager = paramManager;
  context.cache = cache;
  context.histo = histo;
  context.quality = quality;
  context.resolution = resolution;

  switch (valid) {
  case partload:
    [[fallthrough]];
  case none:// Load image into buffer
  {
    LoadParams loadParam;
    AbortStatus abort;
    std::tie(valid, abort, loadParam) = paramManager->claimLoadParams();
    if (abort == AbortStatus::restart) return emptyMatrix();

    LoadStage stage;
    auto result = stage.process(loadParam.fullFilename, loadParam, context);

    if (!result) return emptyMatrix();

    RawImage &output = *result;

    // Unpack Logic
    filename = QString::fromStdString(loadParam.fullFilename);
    isCR3 = output.isCR3;
    isSraw = output.isSraw;
    isNikonSraw = output.isNikonSraw;
    isMonochrome = output.isMonochrome;
    bool isFloat = output.isFloat;
    raw_image.swap(output.data);

    if ((isSraw && !isMonochrome) || (output.isFloat && output.isSraw)) {// Use flags from output
      raw_width = raw_image.nc() / 3;
    } else {
      raw_width = raw_image.nc();
    }
    raw_height = raw_image.nr();

    for (int i = 0; i < 2; ++i)
      for (int j = 0; j < 2; ++j) cfa[i][j] = output.cfa[i][j];
    for (int i = 0; i < 6; ++i)
      for (int j = 0; j < 6; ++j) xtrans[i][j] = output.xtrans[i][j];
    maxXtrans = output.maxXtrans;

    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) {
        camToRGB[i][j] = output.camToRGB[i][j];
        xyzToCam[i][j] = output.xyzToCam[i][j];
      }
    // Reconstruct camToRGB4
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 4; j++) {
        camToRGB4[i][j] = camToRGB[i][j];
        if (i == j)
          camToRGB4[i][j] = 1;
        else
          camToRGB4[i][j] = 0;
        if (j == 3) camToRGB4[i][j] = camToRGB4[i][1];
      }
    }

    exifData = output.exif;

    rPreMul = output.rPreMul;
    gPreMul = output.gPreMul;
    bPreMul = output.bPreMul;
    rCamMul = output.rCamMul;
    gCamMul = output.gCamMul;
    bCamMul = output.bCamMul;

    rUserMul = rCamMul;
    gUserMul = gCamMul;
    bUserMul = bCamMul;

    maxValue = output.maxValue;
    for (int i = 0; i < 3; ++i) colorMaxValue[i] = output.colorMaxValue[i];

    valid = paramManager->markLoadComplete();
    updateProgress(valid, 0.0f);
    [[fallthrough]];
  }
  case partdemosaic:
    [[fallthrough]];
  case load:// Do demosaic, or load non-raw images
  {
    LoadParams loadParam;
    DemosaicParams demosaicParam;
    AbortStatus abort;
    std::tie(valid, abort, loadParam, demosaicParam) = paramManager->claimDemosaicParams();
    if (abort == AbortStatus::restart) {
      FILM_WARN("imagePipeline.cpp: aborted at demosaic");
      return emptyMatrix();
    }

    // Construct Input
    RawImage input;
    input.data.swap(raw_image);// Move logic

    // Copy Metadata
    input.isSraw = isSraw;
    input.isNikonSraw = isNikonSraw;
    input.isMonochrome = isMonochrome;
    input.isCR3 = isCR3;
    input.maxXtrans = maxXtrans;
    for (int i = 0; i < 2; ++i)
      for (int j = 0; j < 2; ++j) input.cfa[i][j] = cfa[i][j];
    for (int i = 0; i < 6; ++i)
      for (int j = 0; j < 6; ++j) input.xtrans[i][j] = xtrans[i][j];

    input.rPreMul = rPreMul;
    input.gPreMul = gPreMul;
    input.bPreMul = bPreMul;
    input.rCamMul = rCamMul;
    input.gCamMul = gCamMul;
    input.bCamMul = bCamMul;
    input.maxValue = maxValue;
    for (int i = 0; i < 3; ++i) input.colorMaxValue[i] = colorMaxValue[i];

    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) {
        input.camToRGB[i][j] = camToRGB[i][j];
        input.xyzToCam[i][j] = xyzToCam[i][j];
      }
    // DemosaicStage doesn't seem to need camToRGB4 per my check.

    DemosaicStage stage;
    auto result = stage.process(input, demosaicParam, context);

    input.data.swap(raw_image);// Restore raw_image

    if (!result) return emptyMatrix();

    RawImage &output = *result;
    demosaiced_image.swap(output.data);

    // Mark complete
    valid = paramManager->markDemosaicComplete();
    updateProgress(valid, 0.0f);
    [[fallthrough]];
  }
  case partpostdemosaic:
    [[fallthrough]];
  case demosaic:// Do postdemosaic work
  {
    PostDemosaicParams postDemosaicParam;
    AbortStatus abort;
    std::tie(valid, abort, postDemosaicParam) = paramManager->claimPostDemosaicParams();
    if (abort == AbortStatus::restart) return emptyMatrix();

    // Construct Input
    RawImage input;
    input.data.swap(demosaiced_image);

    input.rPreMul = rPreMul;
    input.gPreMul = gPreMul;
    input.bPreMul = bPreMul;
    input.rUserMul = rUserMul;
    input.gUserMul = gUserMul;
    input.bUserMul = bUserMul;
    input.rCamMul = rCamMul;
    input.gCamMul = gCamMul;
    input.bCamMul = bCamMul;
    input.maxValue = maxValue;
    for (int i = 0; i < 3; ++i) input.colorMaxValue[i] = colorMaxValue[i];
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) input.xyzToCam[i][j] = xyzToCam[i][j];
    input.isMonochrome = isMonochrome;

    PostDemosaicStage stage;
    auto result = stage.process(input, postDemosaicParam, context);

    input.data.swap(demosaiced_image);// Restore

    if (!result) return emptyMatrix();

    RawImage &output = *result;
    post_demosaic_image.swap(output.data);

    rUserMul = output.rUserMul;
    gUserMul = output.gUserMul;
    bUserMul = output.bUserMul;

    if (rUserMul == 0) rUserMul = 1;
    if (gUserMul == 0) gUserMul = 1;
    if (bUserMul == 0) bUserMul = 1;

    valid = paramManager->markPostDemosaicComplete();
#ifdef ENABLE_NAN_TRAPPING
    SCAN_MATRIX_FOR_NAN(post_demosaic_image, "post_demosaic_image");
#endif
    updateProgress(valid, 0.0f);
    [[fallthrough]];
  }
  case partnrnlmeans:
    [[fallthrough]];
  case postdemosaic:// Do nonlocal means (luma+chroma) noise reduction
  {
    NlmeansNRParams nrParam;
    AbortStatus abort;
    std::tie(valid, abort, nrParam) = paramManager->claimNlmeansNRParams();
    if (abort == AbortStatus::restart) return emptyMatrix();

    // Construct Input
    RawImage input;
    input.data.swap(post_demosaic_image);

    input.rUserMul = rUserMul;
    input.gUserMul = gUserMul;
    input.bUserMul = bUserMul;
    input.isMonochrome = isMonochrome;
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) input.camToRGB[i][j] = camToRGB[i][j];

    NlmeansNRStage stage;
    auto result = stage.process(input, nrParam, context);

    input.data.swap(post_demosaic_image);// Restore

    if (!result) return emptyMatrix();
    RawImage &output = *result;

    nlmeans_nr_image.swap(output.data);

    valid = paramManager->markNlmeansNRComplete();
#ifdef ENABLE_NAN_TRAPPING
    SCAN_MATRIX_FOR_NAN(nlmeans_nr_image, "nlmeans_nr_image");
#endif
    updateProgress(valid, 0.0f);
    [[fallthrough]];
  }
  case partnrimpulse:
    [[fallthrough]];
  case nrnlmeans:// do impulse denoise
  {
    ImpulseNRParams nrParam;
    AbortStatus abort;
    std::tie(valid, abort, nrParam) = paramManager->claimImpulseNRParams();
    if (abort == AbortStatus::restart) return emptyMatrix();

    RawImage input;
    input.data.swap(nlmeans_nr_image);
    input.isOklab = nrParam.nrEnabled;

    ImpulseNRStage stage;
    auto result = stage.process(input, nrParam, context);

    input.data.swap(nlmeans_nr_image);// Restore

    if (!result) return emptyMatrix();
    RawImage &output = *result;

    impulse_nr_image.swap(output.data);

    valid = paramManager->markImpulseNRComplete();
#ifdef ENABLE_NAN_TRAPPING
    SCAN_MATRIX_FOR_NAN(impulse_nr_image, "impulse_nr_image");
#endif
    updateProgress(valid, 0.0f);
    [[fallthrough]];
  }
  case partnrchroma:
    [[fallthrough]];
  case nrimpulse:// do chroma denoise
  {
    ChromaNRParams nrParam;
    AbortStatus abort;
    std::tie(valid, abort, nrParam) = paramManager->claimChromaNRParams();
    if (abort == AbortStatus::restart) return emptyMatrix();

    RawImage input;
    input.data.swap(impulse_nr_image);
    input.isOklab = nrParam.nrEnabled;
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) input.camToRGB[i][j] = camToRGB[i][j];

    ChromaNRStage stage;
    auto result = stage.process(input, nrParam, context);

    input.data.swap(impulse_nr_image);// Restore

    if (!result) return emptyMatrix();
    RawImage &output = *result;

    chroma_nr_image.swap(output.data);

    valid = paramManager->markChromaNRComplete();
#ifdef ENABLE_NAN_TRAPPING
    SCAN_MATRIX_FOR_NAN(chroma_nr_image, "chroma_nr_image");
#endif
    updateProgress(valid, 0.0f);
    [[fallthrough]];
  }
  case partprefilmulation:
    [[fallthrough]];
  case nrchroma:// Do pre-filmulation work.
  {
    PrefilmParams prefilmParam;
    FILM_DEBUG("imagePipeline beginning pre-filmulation");
    AbortStatus abort;
    std::tie(valid, abort, prefilmParam) = paramManager->claimPrefilmParams();
    if (abort == AbortStatus::restart) return emptyMatrix();

    RawImage input;
    if (prefilmParam.nrEnabled) {
      input.data.swap(chroma_nr_image);
      input.isOklab = true;
    } else {
      input.data.swap(post_demosaic_image);
      input.isOklab = false;
    }
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) input.camToRGB[i][j] = camToRGB[i][j];

    PrefilmulationStage stage;
    auto result = stage.process(input, prefilmParam, context);

    if (prefilmParam.nrEnabled) {
      input.data.swap(chroma_nr_image);
    } else {
      input.data.swap(post_demosaic_image);
    }

    if (!result) return emptyMatrix();
    RawImage &output = *result;

    // Always save the full size image, so we can steal it later if needed.
    pre_film_image.swap(output.data);

#ifdef ENABLE_NAN_TRAPPING
    {
      float *pdata = pre_film_image;
      if (pdata) {
        float pmin = *std::min_element(pdata, pdata + pre_film_image.nr() * pre_film_image.nc());
        float pmax = *std::max_element(pdata, pdata + pre_film_image.nr() * pre_film_image.nc());
        FILM_TRACE("QuickPipe: pre_film_image min: {} max: {}", pmin, pmax);
      }
    }
#endif

    if (quality == LowQuality) {
      downscale_and_crop(
        pre_film_image, pre_film_image_small, 0, 0, (pre_film_image.nc() / 3) - 1, pre_film_image.nr() - 1, 600, 600);
    } else if (quality == PreviewQuality) {
      int paritywidth = resolution + resolution % 2 + (pre_film_image.nc() / 3) % 2;
      int parityheight = resolution + resolution % 2 + (pre_film_image.nr()) % 2;
      downscale_and_crop(pre_film_image,
        pre_film_image_small,
        0,
        0,
        (pre_film_image.nc() / 3) - 1,
        pre_film_image.nr() - 1,
        paritywidth,
        parityheight);
    } else {
      // High quality, we don't need the small image necessarily, but we can clear it or leave it.
      // Let's clear it to save memory if we are doing full processing and don't need preview?
      // Actually, existing logic cleared it. Use:
      if (cache == NoCache) pre_film_image_small.set_size(0, 0);
    }

    valid = paramManager->markPrefilmComplete();
#ifdef ENABLE_NAN_TRAPPING
    SCAN_MATRIX_FOR_NAN(pre_film_image, "pre_film_image");
#endif
    updateProgress(valid, 0.0f);
    [[fallthrough]];
  }
  case partfilmulation:
    [[fallthrough]];
  case prefilmulation:// Do filmulation
  {

    matrix<float> film_input_image;
    if (stealData) {
      FILM_DEBUG("imagePipeline stealing data");
      exifData = stealVictim->exifData;
      rCamMul = stealVictim->rCamMul;
      gCamMul = stealVictim->gCamMul;
      bCamMul = stealVictim->bCamMul;
      rPreMul = stealVictim->rPreMul;
      gPreMul = stealVictim->gPreMul;
      bPreMul = stealVictim->bPreMul;
      rUserMul = stealVictim->rUserMul;
      gUserMul = stealVictim->gUserMul;
      bUserMul = stealVictim->bUserMul;
      maxValue = stealVictim->maxValue;
      isSraw = stealVictim->isSraw;
      isNikonSraw = stealVictim->isNikonSraw;
      isMonochrome = stealVictim->isMonochrome;
      isCR3 = stealVictim->isCR3;
      raw_width = stealVictim->raw_width;
      raw_height = stealVictim->raw_height;
      // copy color matrix
      // get color matrix
      for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
          camToRGB[i][j] = stealVictim->camToRGB[i][j];
          xyzToCam[i][j] = stealVictim->xyzToCam[i][j];
        }
      }
      for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) { camToRGB4[i][j] = stealVictim->camToRGB4[i][j]; }
      }
      matrix<float> *sourceImage = &stealVictim->pre_film_image;
      if (sourceImage->nr() == 0 && stealVictim->pre_film_image_small.nr() > 0) {
        sourceImage = &stealVictim->pre_film_image_small;
        FILM_DEBUG("Stealing from small image");
      }

      if (!isMonochrome) {
        FILM_DEBUG("Stealing: sourceImage sizes: {}x{}", sourceImage->nr(), sourceImage->nc());
        float *sdata = *sourceImage;
        if (sdata) {
          float smin = *std::min_element(sdata, sdata + sourceImage->nr() * sourceImage->nc());
          float smax = *std::max_element(sdata, sdata + sourceImage->nr() * sourceImage->nc());
          FILM_DEBUG("Stealing: sourceImage min: {} max: {}", smin, smax);
        } else {
          FILM_ERROR("Stealing: sourceImage data is NULL");
        }

        FILM_DEBUG("Stealing: camToRGB[0][0]: {}", camToRGB[0][0]);

        raw_to_sRGB(*sourceImage, film_input_image, camToRGB);

        float *fdata = film_input_image;
        if (fdata) {
          float fmin = *std::min_element(fdata, fdata + film_input_image.nr() * film_input_image.nc());
          float fmax = *std::max_element(fdata, fdata + film_input_image.nr() * film_input_image.nc());
          FILM_DEBUG("Stealing: film_input_image min: {} max: {}", fmin, fmax);
        }
      } else {
        film_input_image = *sourceImage;
      }
    } else {
      FILM_DEBUG("imagePipeline not stealing data");
      if (quality == LowQuality || quality == PreviewQuality) {
        // grab shrunken image
        if (!isMonochrome) {
          raw_to_sRGB(pre_film_image_small, film_input_image, camToRGB);
        } else {
          film_input_image = pre_film_image_small;
        }
      } else {
        if (!isMonochrome) {
          raw_to_sRGB(pre_film_image, film_input_image, camToRGB);
        } else {
          film_input_image = pre_film_image;
        }
      }

      if (NoCache == cache) {
        pre_film_image.set_size(0, 0);
        pre_film_image_small.set_size(0, 0);
        cacheEmpty = true;
      } else {
        cacheEmpty = false;
      }
    }

    FILM_DEBUG("imagePipeline beginning filmulation");
    FILM_DEBUG("imagePipeline image width:  {}", film_input_image.nc() / 3);
    FILM_DEBUG("imagePipeline image height: {}", film_input_image.nr());

    // We don't need to check abort status out here, because
    // the filmulate function will do so inside its loop.
    // We just check for it returning an empty matrix.

    // Here we do the film simulation on the image...
    // If filmulate detects an abort, it returns true.
    if (filmulate(film_input_image, filmulated_image, paramManager, this)) {
      FILM_WARN("imagePipeline aborted at filmulation");
      return emptyMatrix();
    }

    if (WithHisto == histo) {
      // grab crop and rotation parameters
      CropParams cropParam = paramManager->claimCropParams();
      bool updatePreFilm = false;
      if (cropHeight != cropParam.cropHeight || cropAspect != cropParam.cropAspect
          || cropHoffset != cropParam.cropHoffset || cropVoffset != cropParam.cropVoffset
          || rotation != cropParam.rotation) {
        updatePreFilm = true;
      }
      cropHeight = cropParam.cropHeight;
      cropAspect = cropParam.cropAspect;
      cropHoffset = cropParam.cropHoffset;
      cropVoffset = cropParam.cropVoffset;
      rotation = cropParam.rotation;
      if (updatePreFilm) {
        if (!stealData) {
          histoInterface->updateHistPreFilm(
            pre_film_image, 65535, rotation, cropHeight, cropAspect, cropHoffset, cropVoffset);
        } else {
          histoInterface->updateHistPreFilm(
            stealVictim->pre_film_image, 65535, rotation, cropHeight, cropAspect, cropHoffset, cropVoffset);
        }
      }
      histoInterface->updateHistPostFilm(filmulated_image,
        .0025f,// TODO connect this magic number to the qml
        rotation,
        cropHeight,
        cropAspect,
        cropHoffset,
        cropVoffset);
    }

    FILM_INFO("ImagePipeline::processImage: Filmulation complete.");

    valid = paramManager->markFilmComplete();
    updateProgress(valid, 0.0f);
    [[fallthrough]];
  }
  case partblackwhite:
    [[fallthrough]];
  case filmulation:// Do whitepoint_blackpoint
  {
    FILM_DEBUG("imagePipeline beginning whitepoint blackpoint");
    FILM_DEBUG("imagePipeline image width:  {}", filmulated_image.nc() / 3);
    FILM_DEBUG("imagePipeline image height: {}", filmulated_image.nr());

    BlackWhiteParams blackWhiteParam;
    AbortStatus abort;
    std::tie(valid, abort, blackWhiteParam) = paramManager->claimBlackWhiteParams();
    if (abort == AbortStatus::restart) {
      FILM_WARN("imagePipeline aborted at whitepoint blackpoint");
      return emptyMatrix();
    }

    // Update histograms if necessary to correspond to crop
    if (WithHisto == histo) {
      // grab crop and rotation parameters
      bool updatePrePostFilm = false;
      if (cropHeight != blackWhiteParam.cropHeight || cropAspect != blackWhiteParam.cropAspect
          || cropHoffset != blackWhiteParam.cropHoffset || cropVoffset != blackWhiteParam.cropVoffset
          || rotation != blackWhiteParam.rotation) {
        updatePrePostFilm = true;
      }
      cropHeight = blackWhiteParam.cropHeight;
      cropAspect = blackWhiteParam.cropAspect;
      cropHoffset = blackWhiteParam.cropHoffset;
      cropVoffset = blackWhiteParam.cropVoffset;
      rotation = blackWhiteParam.rotation;
      if (updatePrePostFilm) {
        if (!stealData) {
          histoInterface->updateHistPreFilm(
            pre_film_image, 65535, rotation, cropHeight, cropAspect, cropHoffset, cropVoffset);
        } else {
          histoInterface->updateHistPreFilm(
            stealVictim->pre_film_image, 65535, rotation, cropHeight, cropAspect, cropHoffset, cropVoffset);
        }
        histoInterface->updateHistPostFilm(filmulated_image,
          .0025f,// TODO connect this magic number to the qml
          rotation,
          cropHeight,
          cropAspect,
          cropHoffset,
          cropVoffset);
      }
    }
    matrix<float> rotated_image;

    rotate_image(filmulated_image, rotated_image, blackWhiteParam.rotation);

    if (NoCache == cache)// clean up ram that's not needed anymore in order to
                         // reduce peak consumption
    {
      filmulated_image.set_size(0, 0);
      cacheEmpty = true;
    } else {
      cacheEmpty = false;
    }

    const int imWidth = rotated_image.nc() / 3;
    const int imHeight = rotated_image.nr();

    const float tempHeight = imHeight * max(min(1.0f, blackWhiteParam.cropHeight),
                               0.0f);// restrict domain to 0:1
    const float tempAspect = max(min(10000.0f, blackWhiteParam.cropAspect),
      0.0001f);// restrict aspect ratio
    int width = int(round(min(tempHeight * tempAspect, float(imWidth))));
    int height = int(round(min(tempHeight, imWidth / tempAspect)));
    const float maxHoffset = (1.0f - (float(width) / float(imWidth))) / 2.0f;
    const float maxVoffset = (1.0f - (float(height) / float(imHeight))) / 2.0f;
    const float oddH =
      (!(int(round((imWidth - width) / 2.0)) * 2 == (imWidth - width))) * 0.5f;// it's 0.5 if it's odd, 0 otherwise
    const float oddV =
      (!(int(round((imHeight - height) / 2.0)) * 2 == (imHeight - height))) * 0.5f;// it's 0.5 if it's odd, 0 otherwise
    const float hoffset =
      (round(max(min(blackWhiteParam.cropHoffset, maxHoffset), -maxHoffset) * imWidth + oddH) - oddH) / imWidth;
    const float voffset =
      (round(max(min(blackWhiteParam.cropVoffset, maxVoffset), -maxVoffset) * imHeight + oddV) - oddV) / imHeight;
    int startX = int(round(0.5f * (imWidth - width) + hoffset * imWidth));
    int startY = int(round(0.5f * (imHeight - height) + voffset * imHeight));
    int endX = startX + width - 1;
    int endY = startY + height - 1;

    if (blackWhiteParam.cropHeight <= 0)// it shall be turned off
    {
      startX = 0;
      startY = 0;
      endX = imWidth - 1;
      endY = imHeight - 1;
      width = imWidth;
      height = imHeight;
    }

    matrix<float> cropped_image;
    FILM_DEBUG("crop start: {}", timeDiff(timeRequested));
    std::chrono::steady_clock::time_point crop_time;
    crop_time = std::chrono::steady_clock::now();

    downscale_and_crop(rotated_image, cropped_image, startX, startY, endX, endY, width, height);

    FILM_DEBUG("crop end: {}", timeDiff(crop_time));

    rotated_image.set_size(0, 0);// clean up ram that's not needed anymore

    whitepoint_blackpoint(cropped_image,// filmulated_image,
      contrast_image,
      blackWhiteParam.whitepoint,
      blackWhiteParam.blackpoint);

    valid = paramManager->markBlackWhiteComplete();
    updateProgress(valid, 0.0f);
    [[fallthrough]];
  }
  case partcolorcurve:
    [[fallthrough]];
  case blackwhite:// Do color_curve
  {
    FILM_DEBUG("imagePipeline beginning dummy color curve");
    // It's not gonna abort because we have no color curves yet..
    // Prepare LUT's for individual color processin.g
    lutR.setUnity();
    lutG.setUnity();
    lutB.setUnity();
    colorCurves(contrast_image, color_curve_image, lutR, lutG, lutB);

    if (NoCache == cache) {
      contrast_image.set_size(0, 0);
      cacheEmpty = true;
    } else {
      cacheEmpty = false;
    }

    valid = paramManager->markColorCurvesComplete();
    updateProgress(valid, 0.0f);
    [[fallthrough]];
  }
  case partfilmlikecurve:
    [[fallthrough]];
  case colorcurve:// Do film-like curve
  {
    FILM_DEBUG("imagePipeline beginning film like curve");

    FilmlikeCurvesParams curvesParam;
    AbortStatus abort;
    std::tie(valid, abort, curvesParam) = paramManager->claimFilmlikeCurvesParams();
    if (abort == AbortStatus::restart) {
      FILM_WARN("imagePipeline aborted at color curve");
      return emptyMatrix();
    }

    filmLikeLUT.fill([=](unsigned short in) -> unsigned short {
      float shResult = shadows_highlights(float(in) / 65535.0f,
        curvesParam.shadowsX,
        curvesParam.shadowsY,
        curvesParam.highlightsX,
        curvesParam.highlightsY);
      return ushort(65535 * default_tonecurve(shResult));
    });
    matrix<unsigned short> &film_curve_image = vibrance_saturation_image;
    film_like_curve(color_curve_image, film_curve_image, filmLikeLUT);

    if (NoCache == cache) {
      color_curve_image.set_size(0, 0);
      cacheEmpty = true;
      // film_curve_image is going out of scope anyway.
    } else {
      cacheEmpty = false;
    }

    if (!curvesParam.monochrome) {
      vibrance_saturation(film_curve_image, vibrance_saturation_image, curvesParam.vibrance, curvesParam.saturation);
    } else {
      monochrome_convert(
        film_curve_image, vibrance_saturation_image, curvesParam.bwRmult, curvesParam.bwGmult, curvesParam.bwBmult);
    }

    updateProgress(valid, 0.0f);
    [[fallthrough]];
  }
  default:// output
  {
    FILM_DEBUG("imagePipeline beginning output");
    if (NoCache == cache) {
      // vibrance_saturation_image.set_size(0, 0);
      cacheEmpty = true;
    } else {
      cacheEmpty = false;
    }
    if (WithHisto == histo) { histoInterface->updateHistFinal(vibrance_saturation_image); }
    valid = paramManager->markFilmLikeCurvesComplete();
    updateProgress(valid, 0.0f);

    exifOutput = exifData;
    return vibrance_saturation_image;
  }
  }// End task switch

  FILM_WARN("imagePipeline aborted at end");
  return emptyMatrix();
}

// Saved for posterity: we may want to re-implement this 0.1 second continuation
//  inside of parameterManager.
// bool ImagePipeline::checkAbort(bool aborted)
//{
//     if (aborted && timeDiff(timeRequested) > 0.1)
//     {
//         cout << "ImagePipeline::aborted. valid = " << valid << endl;
//         return true;
//     }
//     else
//     {
//         return false;
//     }
// }

void ImagePipeline::updateProgress(Valid valid, float stepProgress)
{
  double totalTime = numeric_limits<double>::epsilon();
  double totalCompletedTime = 0;
  for (ulong i = 0; i < completionTimes.size(); i++) {
    totalTime += completionTimes[i];
    float fractionCompleted = 0;
    if (i <= valid) fractionCompleted = 1;
    if (i == valid + 1) fractionCompleted = stepProgress;
    // if greater -> 0
    totalCompletedTime += completionTimes[i] * double(fractionCompleted);
  }
  histoInterface->setProgress(float(totalCompletedTime / totalTime));
}

// Do not call this on something that's already been used!
void ImagePipeline::setCache(Cache cacheIn)
{
  if (false == hasStartedProcessing) { cache = cacheIn; }
}

// This swaps the data between pipelines.
// The intended use is for preloading.
void ImagePipeline::swapPipeline(ImagePipeline *swapTarget)
{
  std::swap(valid, swapTarget->valid);
  std::swap(progress, swapTarget->progress);

  std::swap(filename, swapTarget->filename);

  std::swap(cfa, swapTarget->cfa);
  std::swap(xtrans, swapTarget->xtrans);
  std::swap(maxXtrans, swapTarget->maxXtrans);

  std::swap(raw_width, swapTarget->raw_width);
  std::swap(raw_height, swapTarget->raw_height);

  std::swap(camToRGB, swapTarget->camToRGB);
  std::swap(xyzToCam, swapTarget->xyzToCam);
  std::swap(camToRGB4, swapTarget->camToRGB4);

  std::swap(rCamMul, swapTarget->rCamMul);
  std::swap(gCamMul, swapTarget->gCamMul);
  std::swap(bCamMul, swapTarget->bCamMul);
  std::swap(rPreMul, swapTarget->rPreMul);
  std::swap(gPreMul, swapTarget->gPreMul);
  std::swap(bPreMul, swapTarget->bPreMul);
  std::swap(rUserMul, swapTarget->rUserMul);
  std::swap(gUserMul, swapTarget->gUserMul);
  std::swap(bUserMul, swapTarget->bUserMul);

  std::swap(maxValue, swapTarget->maxValue);
  std::swap(colorMaxValue[0], swapTarget->colorMaxValue[0]);
  std::swap(colorMaxValue[1], swapTarget->colorMaxValue[1]);
  std::swap(colorMaxValue[2], swapTarget->colorMaxValue[2]);
  std::swap(isSraw, swapTarget->isSraw);
  std::swap(isNikonSraw, swapTarget->isNikonSraw);
  std::swap(isMonochrome, swapTarget->isMonochrome);
  std::swap(isCR3, swapTarget->isCR3);

  std::swap(cropHeight, swapTarget->cropHeight);
  std::swap(cropAspect, swapTarget->cropAspect);
  std::swap(cropHoffset, swapTarget->cropHoffset);
  std::swap(cropVoffset, swapTarget->cropVoffset);
  std::swap(rotation, swapTarget->rotation);

  std::swap(exifData, swapTarget->exifData);
  std::swap(basicExifData, swapTarget->basicExifData);

  raw_image.swap(swapTarget->raw_image);
  demosaiced_image.swap(swapTarget->demosaiced_image);
  post_demosaic_image.swap(swapTarget->post_demosaic_image);
  nlmeans_nr_image.swap(swapTarget->nlmeans_nr_image);
  impulse_nr_image.swap(swapTarget->impulse_nr_image);
  chroma_nr_image.swap(swapTarget->chroma_nr_image);
  pre_film_image.swap(swapTarget->pre_film_image);
  pre_film_image_small.swap(swapTarget->pre_film_image_small);
  filmulated_image.swap(swapTarget->filmulated_image);
  contrast_image.swap(swapTarget->contrast_image);
  color_curve_image.swap(swapTarget->color_curve_image);
  vibrance_saturation_image.swap(swapTarget->vibrance_saturation_image);
}

// This is used to update the histograms once data is copied on an image change
void ImagePipeline::rerunHistograms()
{
  if (WithHisto == histo) {
    if (valid >= Valid::load) {
      histoInterface->updateHistRaw(raw_image, colorMaxValue, cfa, xtrans, maxXtrans, isSraw, isMonochrome);
    }
    if (valid >= Valid::prefilmulation) {
      histoInterface->updateHistPreFilm(
        pre_film_image, 65535, rotation, cropHeight, cropAspect, cropHoffset, cropVoffset);
    }
    if (valid >= Valid::filmulation) {
      histoInterface->updateHistPostFilm(
        filmulated_image, .0025f, rotation, cropHeight, cropAspect, cropHoffset, cropVoffset);
    }
    if (valid >= Valid::filmlikecurve) { histoInterface->updateHistFinal(vibrance_saturation_image); }
  }
}

// Return the average level of each channel of the image sampled at a 21x21
//  square.
// The square is positioned relative to the image dimensions of the cropped
// image.
void ImagePipeline::sampleWB(const float xPos,
  const float yPos,
  const int rotation,
  const float cropHeight,
  const float cropAspect,
  const float cropVoffset,
  const float cropHoffset,
  float &red,
  float &green,
  float &blue)
{
  if (xPos < 0 || xPos > 1 || yPos < 0 || yPos > 1) {
    red = -1;
    green = -1;
    blue = -1;
    return;
  }

  // recovered_image is what we're looking to sample.
  // It already has the camera multipliers applied, so we have to divide by them
  // later.

  // First we rotate it.
  matrix<float> rotated_image;
  rotate_image(pre_film_image, rotated_image, rotation);

  // Then we crop the recovered image
  // This is copied from the actual image pipeline.
  const int imWidth = rotated_image.nc() / 3;
  const int imHeight = rotated_image.nr();

  const float tempHeight = imHeight * max(min(1.0f, cropHeight), 0.0f);// restrict domain to 0:1
  const float tempAspect = max(min(10000.0f, cropAspect), 0.0001f);// restrict aspect ratio
  int width = int(round(min(tempHeight * tempAspect, float(imWidth))));
  int height = int(round(min(tempHeight, imWidth / tempAspect)));
  const float maxHoffset = (1.0f - (float(width) / float(imWidth))) / 2.0f;
  const float maxVoffset = (1.0f - (float(height) / float(imHeight))) / 2.0f;
  const float oddH =
    (!(int(round((imWidth - width) / 2.0)) * 2 == (imWidth - width))) * 0.5f;// it's 0.5 if it's odd, 0 otherwise
  const float oddV =
    (!(int(round((imHeight - height) / 2.0)) * 2 == (imHeight - height))) * 0.5f;// it's 0.5 if it's odd, 0 otherwise
  const float hoffset = (round(max(min(cropHoffset, maxHoffset), -maxHoffset) * imWidth + oddH) - oddH) / imWidth;
  const float voffset = (round(max(min(cropVoffset, maxVoffset), -maxVoffset) * imHeight + oddV) - oddV) / imHeight;
  int startX = int(round(0.5f * (imWidth - width) + hoffset * imWidth));
  int startY = int(round(0.5f * (imHeight - height) + voffset * imHeight));
  int endX = startX + width - 1;
  int endY = startY + height - 1;

  if (cropHeight <= 0)// it shall be turned off
  {
    startX = 0;
    startY = 0;
    endX = imWidth - 1;
    endY = imHeight - 1;
    width = imWidth;
    height = imHeight;
  }

  matrix<float> cropped_image;

  downscale_and_crop(rotated_image, cropped_image, startX, startY, endX, endY, width, height);

  rotated_image.set_size(0, 0);

  // Now we compute the x position
  const int sampleX = round(xPos * (width - 1));
  const int sampleY = round(yPos * (height - 1));
  const int sampleStartX = max(0, sampleX - 10);
  const int sampleStartY = max(0, sampleY - 10);
  const int sampleEndX = min(width - 1, sampleX + 10);
  const int sampleEndY = min(height - 1, sampleY + 10);

  double rSum = 0;
  double gSum = 0;
  double bSum = 0;
  int count = 0;
  for (int row = sampleStartY; row <= sampleEndY; row++) {
    for (int col = sampleStartX; col <= sampleEndX; col++) {
      rSum += cropped_image(row, col * 3);
      gSum += cropped_image(row, col * 3 + 1);
      bSum += cropped_image(row, col * 3 + 2);
      count++;
    }
  }
  if (count < 1)// some sort of error occurs
  {
    red = -1;
    green = -1;
    blue = -1;
    return;
  }

  // Compute the average and also divide by the camera WB multipliers
  red = rSum / (rUserMul * count);
  green = gSum / (gUserMul * count);
  blue = bSum / (bUserMul * count);
  FILM_DEBUG("custom WB sampled r: {}", red);
  FILM_DEBUG("custom WB sampled g: {}", green);
  FILM_DEBUG("custom WB sampled b: {}", blue);
}

void ImagePipeline::clearInvalid(Valid validIn)
{
  if (validIn < load) { raw_image.set_size(0, 0); }
  if (validIn < demosaic) { demosaiced_image.set_size(0, 0); }
  if (validIn < postdemosaic) { post_demosaic_image.set_size(0, 0); }
  if (validIn < nrnlmeans) { nlmeans_nr_image.set_size(0, 0); }
  if (validIn < nrimpulse) { impulse_nr_image.set_size(0, 0); }
  if (validIn < nrchroma) { chroma_nr_image.set_size(0, 0); }
  if (validIn < prefilmulation) {
    pre_film_image.set_size(0, 0);
    pre_film_image_small.set_size(0, 0);
  }
  if (validIn < filmulation) { filmulated_image.set_size(0, 0); }
  if (validIn < blackwhite) { contrast_image.set_size(0, 0); }
  if (validIn < colorcurve) { color_curve_image.set_size(0, 0); }
  if (validIn < filmlikecurve) { vibrance_saturation_image.set_size(0, 0); }
}
