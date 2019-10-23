#include "filmImageProvider.h"
#include "../core/filmSim.hpp"
#include <iostream>
#include <omp.h>
using std::cout;
using std::endl;

void FilmImageProvider::updateShortHistogram(Histogram &hist, const matrix<unsigned short>& image)
{
    struct timeval time;
    gettimeofday(&time, nullptr);

    long long lHist[128];
    long long rHist[128];
    long long gHist[128];
    long long bHist[128];

    //clear memory
    //zeroHistogram(hist);
    for(int i = 0; i < 128; i++)
    {
        lHist[i] = 0;
        rHist[i] = 0;
        gHist[i] = 0;
        bHist[i] = 0;
    }
    hist.lHistMax = 1;
    hist.rHistMax = 1;
    hist.gHistMax = 1;
    hist.bHistMax = 1;

    //for(int i = 0; i < image.nr(); i = i + 5)
    //    for(int j = 0; j < image.nc(); j = j + 15)
    #pragma omp parallel for reduction(+: lHist[:128]) reduction(+: rHist[:128]) reduction(+: gHist[:128]) reduction(+: bHist[:128])
    for(int i = 0; i < image.nr(); i = i + 1)
    {
        for(int j = 0; j < image.nc(); j = j + 3)
        {
            unsigned short luma = ushort( 0.2126*image(i,j  )
                                         +0.7151*image(i,j+1)
                                         +0.0722*image(i,j+2));
            //The values start from 0 to 65535, so by dividing by 512 we fit into 128 bins.
            lHist[luma        /512]+= 1;
            rHist[image(i,j  )/512]+= 1;
            gHist[image(i,j+1)/512]+= 1;
            bHist[image(i,j+2)/512]+= 1;
        }
    }
    for(int i = 1; i < 127; i++)
    {
        hist.lHist[i] = lHist[i];
        hist.rHist[i] = rHist[i];
        hist.gHist[i] = gHist[i];
        hist.bHist[i] = bHist[i];
        hist.lHistMax = (hist.lHist[i] > hist.lHistMax) ? hist.lHist[i] : hist.lHistMax;
        hist.rHistMax = (hist.rHist[i] > hist.rHistMax) ? hist.rHist[i] : hist.rHistMax;
        hist.gHistMax = (hist.gHist[i] > hist.gHistMax) ? hist.gHist[i] : hist.gHistMax;
        hist.bHistMax = (hist.bHist[i] > hist.bHistMax) ? hist.bHist[i] : hist.bHistMax;
    }
    cout << "shortHistoTime: " << timeDiff(time) << endl;
}

void FilmImageProvider::updateFloatHistogram(Histogram &hist, const matrix<float>& image, float maximum)
{
    struct timeval time;
    gettimeofday(&time, nullptr);

    long long lHist[128];
    long long rHist[128];
    long long gHist[128];
    long long bHist[128];

    //clear memory
    //zeroHistogram(hist);
    for(int i = 0; i < 128; i++)
    {
        lHist[i] = 0;
        rHist[i] = 0;
        gHist[i] = 0;
        bHist[i] = 0;
    }
    hist.lHistMax = 1;
    hist.rHistMax = 1;
    hist.gHistMax = 1;
    hist.bHistMax = 1;

    //for(int i = 0; i < image.nr(); i = i + 5)
    //    for(int j = 0; j < image.nc(); j = j + 15)
    #pragma omp parallel for reduction(+: lHist[:128]) reduction(+: rHist[:128]) reduction(+: gHist[:128]) reduction(+: bHist[:128])
    for(int i = 0; i < image.nr(); i = i + 1)
    {
        for(int j = 0; j < image.nc(); j = j + 3)
        {
            float luma = 0.2126f*image(i,j)
                        +0.7151f*image(i,j+1)
                        +0.0722f*image(i,j+2);
            //histIndex returns a value from 0 to 127.
            lHist[histIndex(luma        ,maximum)]++;
            rHist[histIndex(image(i,j)  ,maximum)]++;
            gHist[histIndex(image(i,j+1),maximum)]++;
            bHist[histIndex(image(i,j+2),maximum)]++;
        }
    }
    for(int i = 1; i < 127; i++)
    {
        hist.lHist[i] = lHist[i];
        hist.rHist[i] = rHist[i];
        hist.gHist[i] = gHist[i];
        hist.bHist[i] = bHist[i];
        hist.lHistMax = (hist.lHist[i] > hist.lHistMax) ? hist.lHist[i] : hist.lHistMax;
        hist.rHistMax = (hist.rHist[i] > hist.rHistMax) ? hist.rHist[i] : hist.rHistMax;
        hist.gHistMax = (hist.gHist[i] > hist.gHistMax) ? hist.gHist[i] : hist.gHistMax;
        hist.bHistMax = (hist.bHist[i] > hist.bHistMax) ? hist.bHist[i] : hist.bHistMax;
    }
    cout << "floatHistoTime: " << timeDiff(time) << endl;
}

void FilmImageProvider::updateHistRaw(const matrix<float>& image, float maximum,
                                      unsigned cfa[2][2], unsigned xtrans[6][6], int maxXtrans)
{
    struct timeval time;
    gettimeofday(&time, nullptr);

    //long long lHist[128];
    long long rHist[128];
    long long gHist[128];
    long long bHist[128];

    //clear memory
    //zeroHistogram(hist);
    for(int i = 0; i < 128; i++)
    {
        //lHist[i] = 0;
        rHist[i] = 0;
        gHist[i] = 0;
        bHist[i] = 0;
    }
    rawHist.rHistMax = 1;
    rawHist.gHistMax = 1;
    rawHist.bHistMax = 1;

    //for(int i = 0; i < image.nr(); i = i + 5)
    //    for(int j = 0; j < image.nc(); j = j + 15)
    if(maxXtrans == 0)//it's bayer
    {
        #pragma omp parallel for reduction(+: rHist[:128]) reduction(+: gHist[:128]) reduction(+: bHist[:128])
        for(int i = 0; i < image.nr(); i = i + 1)
        {
            for(int j = 0; j < image.nc(); j = j + 1)
            {
                uint color = cfa[i%2][j%2];
                //histIndex returns a value from 0 to 127.
                if(color == 1) {//green is most common
                    gHist[histIndex(image(i,j),maximum)]++;
                } else if (color == 0) {//red
                    rHist[histIndex(image(i,j),maximum)]++;
                } else {//blue
                    bHist[histIndex(image(i,j),maximum)]++;
                }
            }
        }
    } else {//it's xtrans
        #pragma omp parallel for reduction(+: rHist[:128]) reduction(+: gHist[:128]) reduction(+: bHist[:128])
        for(int i = 0; i < image.nr(); i = i + 1)
        {
            for(int j = 0; j < image.nc(); j = j + 1)
            {
                uint color = xtrans[i%6][j%6];
                //histIndex returns a value from 0 to 127.
                if(color == 1) {//green is most common
                    gHist[histIndex(image(i,j),maximum)]++;
                } else if (color == 0) {//red
                    rHist[histIndex(image(i,j),maximum)]++;
                } else {//blue
                    bHist[histIndex(image(i,j),maximum)]++;
                }
            }
        }
    }
    for(int i = 1; i < 127; i++)
    {
        rawHist.rHist[i] = rHist[i];
        rawHist.gHist[i] = gHist[i];
        rawHist.bHist[i] = bHist[i];
        rawHist.rHistMax = (rawHist.rHist[i] > rawHist.rHistMax) ? rawHist.rHist[i] : rawHist.rHistMax;
        rawHist.gHistMax = (rawHist.gHist[i] > rawHist.gHistMax) ? rawHist.gHist[i] : rawHist.gHistMax;
        rawHist.bHistMax = (rawHist.bHist[i] > rawHist.bHistMax) ? rawHist.bHist[i] : rawHist.bHistMax;
    }
    cout << "rawHistoTime: " << timeDiff(time) << endl;
}

inline int FilmImageProvider::histIndex(float value, float maximum)
{
    return max(0,min(127, int(127*value/maximum)));
}

void FilmImageProvider::zeroHistogram(Histogram &hist)
{
    for(int i = 0; i < 128; i++)
    {
        hist.lHist[i] = 0;
        hist.rHist[i] = 0;
        hist.gHist[i] = 0;
        hist.bHist[i] = 0;
    }
    hist.lHistMax = 1;
    hist.rHistMax = 1;
    hist.gHistMax = 1;
    hist.bHistMax = 1;
    return;
}

void FilmImageProvider::updateHistPreFilm(const matrix<float>& image, float maximum)
{
    updateFloatHistogram(preFilmHist, image, maximum);
    emit histPreFilmChanged();
    return;
}

void FilmImageProvider::updateHistPostFilm(const matrix<float>& image, float maximum)
{
    updateFloatHistogram(postFilmHist, image, maximum);
    emit histPostFilmChanged();
    return;
}

void FilmImageProvider::updateHistFinal(const matrix<unsigned short>& image)
{
    updateShortHistogram(finalHist, image);
    emit histFinalChanged();
    return;
}
