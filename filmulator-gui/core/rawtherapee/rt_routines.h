#ifndef RT_ROUTINES_H
#define RT_ROUTINES_H

#include "../matrix.hpp"
#include "../../ui/parameterManager.h"

//RGB_denoise takes lab images in, and prefers a range of 100 rather than 1, as far as I can tell.
//This makes 0-100 a reasonable range for the chroma strength parameter.
void RGB_denoise(int kall,//0 for no tiling
                 matrix<float> &src,
                 matrix<float> &dst,
                 const float chroma,
                 const float redchro,
                 const float bluechro,
                 ParameterManager* paramManager);//for canceling

//This takes lab images.
//chromaFactor is how much more sensitive it is to chroma
void impulse_nr(matrix<float> &image, double thresh, const double chromaFactor);

#endif // RT_ROUTINES_H
