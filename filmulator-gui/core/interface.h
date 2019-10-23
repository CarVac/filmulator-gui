#ifndef INTERFACE_H
#define INTERFACE_H
#include "matrix.hpp"

enum LogY {no, yes};

struct Histogram {
    long long lHist[128];
    long long rHist[128];
    long long gHist[128];
    long long bHist[128];

    float lHistMax;
    float rHistMax;
    float gHistMax;
    float bHistMax;

    bool empty = true;
};

class Interface
{
public:
    virtual void setProgress(float){}
    virtual void updateHistRaw(const matrix<float>& /*image*/, float /*maximum*/, unsigned /*cfa*/[2][2], unsigned /*xtrans*/[6][6], int /*maxXtrans)*/){}
    virtual void updateHistPreFilm(const matrix<float>& /*image*/, float /*maximum*/){}
    virtual void updateHistPostFilm(const matrix<float>& /*image*/, float /*maximum*/){}
    virtual void updateHistFinal(const matrix<unsigned short>& /*image*/){}


};

#endif // INTERFACE_H
