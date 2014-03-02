#include "filmimageprovider.h"

void FilmImageProvider::updateShortHistogram(histogram &hist, const matrix<unsigned short> image, int &roll)
{
    for(int i = 0; i < 128; i++)
    {
        hist.lHist[i] = 0;
        hist.rHist[i] = 0;
        hist.gHist[i] = 0;
        hist.bHist[i] = 0;
    }
    for(int i = 0; i < image.nr(); i = i + 5)
        for(int j = 0; j < image.nc(); j = j + 15)
        {
            unsigned short luma = 0.2126*image(i,j  )
                                 +0.7151*image(i,j+1)
                                 +0.0722*image(i,j+2);
            //The values start from 0 to 65535, so by dividing by 512 we fit into 128 bins.
            hist.lHist[luma        /512]++;
            hist.rHist[image(i,j  )/512]++;
            hist.gHist[image(i,j+1)/512]++;
            hist.bHist[image(i,j+2)/512]++;
        }
    hist.lHistMax = 0;
    hist.rHistMax = 0;
    hist.gHistMax = 0;
    hist.bHistMax = 0;
    for(int i = 1; i < 127; i++)
    {
        hist.lHistMax = (hist.lHist[i] > hist.lHistMax) ? hist.lHist[i] : hist.lHistMax;
        hist.rHistMax = (hist.rHist[i] > hist.rHistMax) ? hist.rHist[i] : hist.rHistMax;
        hist.gHistMax = (hist.gHist[i] > hist.gHistMax) ? hist.gHist[i] : hist.gHistMax;
        hist.bHistMax = (hist.bHist[i] > hist.bHistMax) ? hist.bHist[i] : hist.bHistMax;
    }
    roll++;
}

void FilmImageProvider::updateFloatHistogram(histogram &hist, const matrix<float> image, float maximum, int &roll)
{
    for(int i = 0; i < 128; i++)
    {
        hist.lHist[i] = 0;
        hist.rHist[i] = 0;
        hist.gHist[i] = 0;
        hist.bHist[i] = 0;
    }
    for(int i = 0; i < image.nr(); i = i + 5)
        for(int j = 0; j < image.nc(); j = j + 15)
        {
            float luma = 0.2126*image(i,j)
                        +0.7151*image(i,j+1)
                        +0.0722*image(i,j+2);
            //histIndex returns a value from 0 to 127.
            hist.lHist[histIndex(luma        ,maximum)]++;
            hist.rHist[histIndex(image(i,j)  ,maximum)]++;
            hist.gHist[histIndex(image(i,j+1),maximum)]++;
            hist.bHist[histIndex(image(i,j+2),maximum)]++;
        }
    hist.lHistMax = 0;
    hist.rHistMax = 0;
    hist.gHistMax = 0;
    hist.bHistMax = 0;
    for(int i = 1; i < 127; i++)
    {
        hist.lHistMax = (hist.lHist[i] > hist.lHistMax) ? hist.lHist[i] : hist.lHistMax;
        hist.rHistMax = (hist.rHist[i] > hist.rHistMax) ? hist.rHist[i] : hist.rHistMax;
        hist.gHistMax = (hist.gHist[i] > hist.gHistMax) ? hist.gHist[i] : hist.gHistMax;
        hist.bHistMax = (hist.bHist[i] > hist.bHistMax) ? hist.bHist[i] : hist.bHistMax;

    }
    roll++;
}

inline int FilmImageProvider::histIndex(float value, float maximum)
{
    return min(127, int(127*value/maximum));
}
