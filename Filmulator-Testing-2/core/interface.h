#ifndef INTERFACE_H
#define INTERFACE_H
#include "matrix.hpp"

enum Valid {none, load, demosaic, prefilmulation, filmulation, whiteblack, colorcurve, filmlikecurve};

enum LogY {no, yes};

struct Histogram {
    long long lHist[128];
    long long rHist[128];
    long long gHist[128];
    long long bHist[128];

    long long lHistMax;
    long long rHistMax;
    long long gHistMax;
    long long bHistMax;
};

class Interface
{
public:
    virtual void updateFilmProgress(float) = 0;
    virtual bool checkAbort() = 0;
    virtual bool checkAbort( Valid ) = 0;
    virtual bool isGUI() = 0;
    virtual void setValid( Valid ) = 0;
    virtual void updateHistPreFilm( const matrix<float> image, float maximum ) = 0;
    virtual void updateHistPostFilm( const matrix<float> image, float maximum ) = 0;
    virtual void updateHistFinal( const matrix<unsigned short> image ) = 0;


};

#endif // INTERFACE_H
