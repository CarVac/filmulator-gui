#include "filmimageprovider.h"

void FilmImageProvider::updateShortHistogram(histogram &hist, const matrix<unsigned short> image, int &roll)
{
    for(int i = 0; i < 128; i++)
    {
        hist.allHist[i*4+0] = 0;//Luma
        hist.allHist[i*4+1] = 0;//R
        hist.allHist[i*4+2] = 0;//G
        hist.allHist[i*4+3] = 0;//B
    }
    for(int i = 0; i < image.nr(); i = i + 5)
        for(int j = 0; j < image.nc(); j = j + 15)
        {
            unsigned short luma = 0.2126*image(i,j)
                                 +0.7151*image(i,j+1)
                                 +0.0722*image(i,j+2);
            hist.allHist[luma/512*4+0]++;
            hist.allHist[image(i,j)/512*4+1]++;
            hist.allHist[image(i,j+1)/512*4+2]++;
            hist.allHist[image(i,j+2)/512*4+3]++;
        }
    hist.histMax[0] = 0;//Luma
    hist.histMax[1] = 0;//R
    hist.histMax[2] = 0;//G
    hist.histMax[3] = 0;//B
    for(int i = 1; i < 127; i++)
    {
        hist.histMax[0] = (hist.allHist[i*4+0] > hist.histMax[0]) ? hist.allHist[i*4+0] : hist.histMax[0];
        hist.histMax[1] = (hist.allHist[i*4+1] > hist.histMax[1]) ? hist.allHist[i*4+1] : hist.histMax[1];
        hist.histMax[2] = (hist.allHist[i*4+2] > hist.histMax[2]) ? hist.allHist[i*4+2] : hist.histMax[2];
        hist.histMax[3] = (hist.allHist[i*4+3] > hist.histMax[3]) ? hist.allHist[i*4+3] : hist.histMax[3];

    }
    roll++;
}

void FilmImageProvider::updateFloatHistogram(histogram &hist, const matrix<float> image, float maximum, int &roll)
{
    for(int i = 0; i < 128; i++)
    {
        hist.allHist[i*4+0] = 0;//Luma
        hist.allHist[i*4+1] = 0;//R
        hist.allHist[i*4+2] = 0;//G
        hist.allHist[i*4+3] = 0;//B
    }
    for(int i = 0; i < image.nr(); i = i + 5)
        for(int j = 0; j < image.nc(); j = j + 15)
        {
            float luma = 0.2126*image(i,j)
                        +0.7151*image(i,j+1)
                        +0.0722*image(i,j+2);
            hist.allHist[histIndex(luma        ,maximum)*4+0]++;
            hist.allHist[histIndex(image(i,j)  ,maximum)*4+1]++;
            hist.allHist[histIndex(image(i,j+1),maximum)*4+2]++;
            hist.allHist[histIndex(image(i,j+2),maximum)*4+3]++;
        }
    hist.histMax[0] = 0;//Luma
    hist.histMax[1] = 0;//R
    hist.histMax[2] = 0;//G
    hist.histMax[3] = 0;//B
    for(int i = 1; i < 127; i++)
    {
        hist.histMax[0] = (hist.allHist[i*4+0] > hist.histMax[0]) ? hist.allHist[i*4+0] : hist.histMax[0];
        hist.histMax[1] = (hist.allHist[i*4+1] > hist.histMax[1]) ? hist.allHist[i*4+1] : hist.histMax[1];
        hist.histMax[2] = (hist.allHist[i*4+2] > hist.histMax[2]) ? hist.allHist[i*4+2] : hist.histMax[2];
        hist.histMax[3] = (hist.allHist[i*4+3] > hist.histMax[3]) ? hist.allHist[i*4+3] : hist.histMax[3];

    }
    roll++;
}

inline int histIndex(float value, float maximum)
{
    return min(127, int(value/maximum));
}
