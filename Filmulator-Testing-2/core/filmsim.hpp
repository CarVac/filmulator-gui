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
    float initial_developer_concentration;
    float reservoir_thickness;//once reservoir_size
    float active_layer_thickness;//once developer_thickness
    float crystals_per_pixel;
    float initial_crystal_radius;
    float initial_silver_salt_density;
    float developer_consumption_const;
    float crystal_growth_const;
    float silver_salt_consumption_const;
    float total_development_time;//once was int
    int agitate_count;
    int development_steps;//once was float; development_resolution
    float film_area;
    float sigma_const;
    float layer_mix_const;
    float layer_time_divisor;
    int rolloff_boundary;
};

bool filmulate(matrix<float> &input_image, matrix<float> &output_density,
               filmulateParams filmParams, Interface* interface);

matrix<float> exposure(matrix<float> input_image, float crystals_per_pixel,
        int rolloff_boundary);

void agitate( matrix<float> &developer_concentration, float active_layer_thickness,
        float &reservoir_developer_concentration, float reservoir_thickness,
        float pixels_per_millimeter );

void develop(matrix<float> &crystal_radius,
        float crystal_growth_const,
        const matrix<float> &active_crystals_per_pixel,
        matrix<float> &silver_salt_density,
        matrix<float> &developer_concentration, float active_layer_thickness,
        float developer_consumption_const, float silver_salt_consumption_const,
        float timestep);

void diffuse(matrix<float> &developer_concentration,
        float sigma_const,
        float pixels_per_millimeter,
        float timestep);

//Reading raws with libraw
bool imread(string input_image_filename, matrix<float> &returnmatrix,
        Exiv2::ExifData &exifData, int highlights, bool caEnabled);

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
        bool caEnabled);

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

void initialize(string input_configuration,
                filmulateParams &filmParams);

bool merge_exps(matrix<float> &input_image, const matrix<float> &temp_image,
        float &exposure_weight, float initial_exposure_comp,
        float &last_exposure_factor, string filename,
        float input_exposure_comp);

string convert_from_raw(char* raw_filename, int i, string tempdir,
        int highlights);

bool imwrite_tiff(matrix<unsigned short> output, string outputfilename,
                  Exiv2::ExifData exifData);

bool imwrite_jpeg(matrix<unsigned short> &output, string outputfilename,
                  Exiv2::ExifData exifData);
        
float default_tonecurve(float input, bool enabled);

float shadows_highlights (float input, float shadowsX, float shadowsY,
                   float highlightsX, float highlightsY);

float slopeFromT (float t, float A, float B, float C);

float xFromT (float t, float A, float B, float C, float D);

float yFromT (float t, float E, float F, float G, float H);

void film_like_curve(matrix<unsigned short> &input,
                      matrix<unsigned short> &output, LUT &lookup);

void RGBTone (unsigned short& r, unsigned short& g, unsigned short& b, LUT &lookup);

JSAMPLE dither_round(int full_int);

double time_diff(struct timeval start);

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


void color_curves(matrix<unsigned short> &input, matrix<unsigned short> &output,
                LUT lutR, LUT lutG, LUT lutB);

void rotate_image(matrix<unsigned short> &input, matrix<unsigned short> &output,
                  int rotation);
#endif // FILMSIM_H
