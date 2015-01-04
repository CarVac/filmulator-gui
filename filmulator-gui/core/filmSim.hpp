/*
 * This file is part of Filmulator.
 *
 * Copyright 2013 Omer Mano and Carlo Vaccari
 *
 * Filmulator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Filmulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Filmulator. If not, see <http://www.gnu.org/licenses/>
 */

#ifndef FILMSIM_H
#define FILMSIM_H

#include "tiffio.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include "jpeglib.h"
#include <setjmp.h>
#include <exiv2/exiv2.hpp>
#include "lut.hpp"
#include "libraw/libraw.h"
#include "matrix.hpp"
#include <sys/time.h>
#include "interface.h"

#ifdef DOUT
#define dout cout
#else
#define dout 0 && cout
#define NDEBUG
#endif
#include "assert.h" //Included later so NDEBUG has an effect

#ifdef TOUT
#define tout cout
#else
#define tout 0 && cout
#endif

using namespace std;

struct filmulateParams {//TODO: adjust variable names.
    float initialDeveloperConcentration;
    float reservoirThickness;//once reservoir_size
    float activeLayerThickness;//once developer_thickness
    float crystalsPerPixel;
    float initialCrystalRadius;
    float initialSilverSaltDensity;
    float developerConsumptionConst;
    float crystalGrowthConst;
    float silverSaltConsumptionConst;
    float totalDevelTime;//once was int
    int agitateCount;
    int developmentSteps;//once was float; development_resolution
    float filmArea;
    float sigmaConst;
    float layerMixConst;
    float layerTimeDivisor;
    int rolloffBoundary;
};

matrix<float> exposure(matrix<float> input_image, float crystals_per_pixel,
        int rolloff_boundary);

//Equalizes the concentration of developer across the reservoir and all pixels.
void agitate( matrix<float> &developerConcentration, float activeLayerThickness,
              float &reservoirDeveloperConcentration, float reservoirThickness,
              float pixelsPerMillimeter );

//This simulates one step of the development reaction.
void develop( matrix<float> &crystalRad,
              float crystalGrowthConst,
              const matrix<float> &activeCrystalsPerPixel,
              matrix<float> &silverSaltDensity,
              matrix<float> &develConcentration,
              float activeLayerThickness,
              float developerConsumptionConst,
              float silverSaltConsumptionConst,
              float timestep);

void diffuse(matrix<float> &developer_concentration,
        float sigma_const,
        float pixels_per_millimeter,
        float timestep);

//Reading raws with libraw
bool imread( string input_image_filename, matrix<float> &returnmatrix,
             Exiv2::ExifData &exifData, int highlights, bool caEnabled, bool lowQuality );

//Reading tiff files
bool imread_tiff(string input_image_filename, matrix<float> &returnmatrix,
		Exiv2::ExifData &exifData);

//Reading jpeg files
bool imread_jpeg(string input_image_filename, matrix<float> &returnmatrix,
		Exiv2::ExifData &exifData);

bool imload(std::vector<string> input_filename_list,
        std::vector<float> input_exposure_compensation,
        matrix<float> &input_image,
        bool tiff, bool jpeg_in, Exiv2::ExifData &exifData, int highlights,
        bool caEnabled, bool lowQuality );

void layer_mix(matrix<float> &developer_concentration,
               float active_layer_thickness,
               float &reservoir_developer_concentration,
               float reservoir_thickness,
               float layer_mix_const,
               float layer_time_divisor,
               float pixels_per_millimeter,
               float timestep);

bool ppm_read_header( ifstream &input, int &xsize, int &ysize );

bool ppm_read_data( ifstream &input, int xsize, int ysize,
        matrix<float> &returnmatrix);

void imwrite(matrix<float> &densityr, matrix<float> &densityg,
        matrix<float> &densityb, string outputfilename, bool sixteen_bit);

bool merge_exps(matrix<float> &input_image, const matrix<float> &temp_image,
        float &exposure_weight, float initial_exposure_comp,
        float &last_exposure_factor, string filename,
        float input_exposure_comp);

string convert_from_raw(char* raw_filename, int i, string tempdir,
        int highlights);

bool imwrite_tiff(matrix<unsigned short> output, string outputfilename,
                  Exiv2::ExifData exifData);

bool imwrite_jpeg(matrix<unsigned short> &output, string outputfilename,
                  Exiv2::ExifData exifData, int quality);

//Applies the hardcoded post-filmulation tonecurve to the image.
float default_tonecurve( float input );

//Applies the effective tonecurve specified by the two control points to the image.
float shadows_highlights( float input,
                          float shadowsX,
                          float shadowsY,
                          float highlightsX,
                          float highlightsY );

//Computes the slope of the cubic polynomial at time t.
float slopeFromT( float t, float A, float B, float C );

//Computes the x value of the cubic polynomial at time t.
float xFromT( float t, float A, float B, float C, float D );

//Computes the y value of the cubic polynomial at time t.
//It happens to be the same as xFromT
float yFromT( float t, float E, float F, float G, float H );

//Applies the LUT to the extreme values while maintaining the relative position of the middle value.
void film_like_curve( matrix<unsigned short> &input,
                      matrix<unsigned short> &output,
                      LUT &lookup );

//Applies the LUT to the first and last values, interpolating the middle value.
void midValueShift (unsigned short& hi, unsigned short& mid, unsigned short& lo, LUT &lookup);

JSAMPLE dither_round(int full_int);

double timeDiff(struct timeval start);

int read_args(int argc, char* argv[],string &input_configuration,
               std::vector<string> &input_filename_list,
               std::vector<float> &input_exposure_compensation, int &hdr_count,
               bool &hdr, bool &tiff, bool &jpeg_in, bool &set_whitepoint,
               float &whitepoint, bool &jpeg_out, bool &tonecurve_out,
               int &highlights);

void output_file(matrix<unsigned short> &output, vector<string> input_filename_list,
                 bool jpeg_out, Exiv2::ExifData exifData);

void whitepoint_blackpoint(matrix<float> &input, matrix<unsigned short> &output,
                           float whitepoint, float blackpoint);

//Applies LUTs individually to each color.
void colorCurves(matrix<unsigned short> &input, matrix<unsigned short> &output,
                LUT lutR, LUT lutG, LUT lutB);

void rotate_image(matrix<unsigned short> &input, matrix<unsigned short> &output,
                  int rotation);

//Changes the white balance, assuming sRGB D50 input.
void white_balance (matrix<float> &input, matrix<float> &output,
                    double temp, double tone);

//Computes the multipliers for the white balance.
void whiteBalanceMults( double temperature, double tint, std::string inputFilename,
                        double &rMult, double &gMult, double &bMult );

//Uses Nelder-Mead method to find the WB parameters that yield (1,1,1) RGB multipliers.
void optimizeWBMults( std::string inputFilename,
                      double &temperature, double &tint );

//Applies the desired temperature and tint adjustments to the image.
void whiteBalance( matrix<float> &input, matrix<float> &output,
                   double temperature, double tint, std::string inputFilename );

void vibrance_saturation(matrix<unsigned short> &input,
                         matrix<unsigned short> &output,
                         double vibrance, double saturation);

void downscale_and_crop(matrix<float> input, matrix<float> &output, int inputStartX, int inputStartY, int inputEndX, int inputEndY, int outputXSize, int outputYSize);
#endif // FILMSIM_H
