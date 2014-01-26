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

struct filmulateParams {
    float initial_developer_concentration;
    float reservoir_size;
    float developer_thickness;
    float crystals_per_pixel;
    float initial_crystal_radius;
    float initial_silver_salt_density;
    float developer_consumption_const;
    float crystal_growth_const;
    float silver_salt_consumption_const;
    int total_development_time;
    int agitate_count;
    float development_resolution;
    float film_area;
    float sigma_const;
    float layer_mix_const;
    float layer_time_divisor;
    float std_cutoff;
    int rolloff_boundary;
};

bool filmulate(matrix<float> &input_image, matrix<float> &output_density,
               bool &abort, filmulateParams filmParams);

matrix<float> exposure(matrix<float> input_image, float crystals_per_pixel,
        int rolloff_boundary);

void agitate( matrix<float> &developer_concentration, float developer_thickness,
        float &reservoir_developer_concentration, float reservoir_size,
        float pixels_per_millimeter );

void develop(matrix<float> &crystal_radius,
        float crystal_growth_const,
        const matrix<float> &active_crystals_per_pixel,
        matrix<float> &silver_salt_density,
        matrix<float> &developer_concentration, float developer_thickness,
        float developer_consumption_const, float silver_salt_consumption_const,
        float timestep);

void diffuse(matrix<float> &developer_concentration,
        float sigma_const,
        float pixels_per_millimeter,
        float timestep);

//Reading raws with libraw
bool imread(string input_image_filename, matrix<float> &returnmatrix,
		Exiv2::ExifData &exifData, int highlights);

//Reading tiff files
bool imread_tiff(string input_image_filename, matrix<float> &returnmatrix,
		Exiv2::ExifData &exifData);

//Reading jpeg files
bool imread_jpeg(string input_image_filename, matrix<float> &returnmatrix,
		Exiv2::ExifData &exifData);

bool imload(std::vector<string> input_filename_list,
        std::vector<float> input_exposure_compensation,
        matrix<float> &input_image,
        bool tiff, bool jpeg_in, Exiv2::ExifData &exifData, int highlights);

void layer_mix(matrix<float> &developer_concentration,
               float developer_thickness,
               float &reservoir_developer_concentration,
               float reservoir_size,
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

bool imwrite_tiff(matrix<int> &output_r, matrix<int> &output_g,
        matrix<int> &output_b, string outputfilename, Exiv2::ExifData exifData);

bool imwrite_jpeg(matrix<int> &output_r, matrix<int> &output_g,
        matrix<int> &output_b, string outputfilename, Exiv2::ExifData exifData);
        
int flatcurve(int input);

int tonecurve(int input);

void apply_tone_curve(LUT &lookup,matrix<float> &output_density,
					  matrix<int> &output_r,matrix<int> &output_g,
					  matrix<int> &output_b);

void RGBTone (float& r, float& g, float& b, LUT &lookup);

JSAMPLE dither_round(int full_int);

double time_diff(struct timeval start);

int read_args(int argc, char* argv[],string &input_configuration,
               std::vector<string> &input_filename_list,
               std::vector<float> &input_exposure_compensation, int &hdr_count,
               bool &hdr, bool &tiff, bool &jpeg_in, bool &set_whitepoint,
               float &whitepoint, bool &jpeg_out, bool &tonecurve_out,
               int &highlights);

void output_file(matrix<int> &output_r, matrix<int> &ouput_g,
                        matrix<int> &output_b,
                        vector<string> input_filename_list, bool jpeg_out,
                        Exiv2::ExifData exifData);

void postprocess(matrix<float> &output_density, bool set_whitepoint,
                        float whitepoint, bool tonecurve_out, float std_cutoff,
                        matrix<int> &output_r, matrix<int> &output_g,
                        matrix<int> &output_b);
