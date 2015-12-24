#include "Halide.h"

using namespace Halide;

#include "RGBtoHSV.cpp"
#include "HSVtoRGB.cpp"

struct BlackWhiteParams {
    float blackpoint;
    float whitepoint;
};

struct FilmlikeCurvesParams {
    float shadowsX;
    float shadowsY;
    float highlightsX;
    float highlightsY;
    float vibrance;
    float saturation;
};

struct OrientationParams {
    int rotation;
};

extern "C" float shadows_highlights(float x,float shadowsX,float shadowsY,
                                    float highlightsX,float highlightsY){
  return x;
}
HalideExtern_5(float,shadows_highlights,float,float,float,float,float);

class postFilmulationGenerator : public Halide::Generator<postFilmulationGenerator> {
  public:
    Param<float> blackPoint;
    Param<float> whitePoint;

    Param<float> shadowsX;
    Param<float> shadowsY;
    Param<float> highlightsX;
    Param<float> highlightsY;

    Param<float> saturation;
    Param<float> vibrance;

    Param<int> rotation;

    ImageParam postFilmulationImage{Float(32), 3, "postFilmulationImage"};

    Var x, y, c;

    Func build() {

      Func blackWhited;
      blackWhited(x,y,c) = clamp(whitePoint*(postFilmulationImage(x,y)-blackPoint),0,1);

      Func LUT;
      Expr maxval = 2^16 - 1;
      LUT(x) = shadows_highlights(cast<float>(x)/(maxval),
                                  shadowsX,
                                  shadowsY,
                                  highlightsX,
                                  highlightsY);
      LUT.bound(x,0,maxval+1);

      Func curved;
      curved(x,y,c) = LUT(floor((maxval)*blackWhited(x,y,c)));

      Func HSVed;
      HSVed = RGBtoHSV(curved);

      Func vibranceSaturated;
      Expr sat   = pow(2, saturation);
      Expr gamma = pow(2,-vibrance);
      vibranceSaturated(x,y,c) =  select(c == 1, // Saturation
                                         clamp(sat*fast_pow(HSVed(x,y,c),gamma),0,1),
                                         HSVed(x,y,c));

      Func RGBed;
      RGBed = HSVtoRGB(vibranceSaturated);

      Func intcast;
      intcast(x,y,c) = cast(UInt(16),RGBed(x,y,c));

      Func rotated;
      int rotation = rotation;
      Expr width  = postFilmulationImage.width();
      Expr height = postFilmulationImage.height();
      rotated(x,y,c) = select(rotation == 2, // Upside down
                                intcast(width-x,height-y,c),
                       select(rotation == 3, // Right side down
                                intcast(y,width-x,c),
                       select(rotation == 1, // Left side down
                                intcast(height-y,x,c),
                       // Default: no rotation
                                intcast(x,y,c))));
      return rotated;
    }
};

RegisterGenerator<postFilmulationGenerator> postFilmulationGenerator{"postFilmulationGenerator"};
