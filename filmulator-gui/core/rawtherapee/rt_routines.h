#ifndef RT_ROUTINES_H
#define RT_ROUTINES_H

#include "../matrix.hpp"
#include "../../ui/parameterManager.h"

void RGB_denoise(int kall,//0 for no tiling
                 matrix<float> &src,
                 matrix<float> &dst,
                 const float chroma,
                 const float redchro,
                 const float bluechro,
                 ParameterManager* paramManager);//for canceling

#endif // RT_ROUTINES_H
