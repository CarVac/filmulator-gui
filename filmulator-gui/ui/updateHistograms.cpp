#include "filmImageProvider.h"
#include <omp.h>

void FilmImageProvider::updateShortHistogram(Histogram &hist, const matrix<unsigned short>& image)
{
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
    long long lHistMax = 1;
    long long rHistMax = 1;
    long long gHistMax = 1;
    long long bHistMax = 1;

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
        lHistMax = (hist.lHist[i] > lHistMax) ? hist.lHist[i] : lHistMax;
        rHistMax = (hist.rHist[i] > rHistMax) ? hist.rHist[i] : rHistMax;
        gHistMax = (hist.gHist[i] > gHistMax) ? hist.gHist[i] : gHistMax;
        bHistMax = (hist.bHist[i] > bHistMax) ? hist.bHist[i] : bHistMax;
    }
    //don't forget to copy 0 and 127
    hist.lHist[0] = lHist[0];
    hist.rHist[0] = rHist[0];
    hist.gHist[0] = gHist[0];
    hist.bHist[0] = bHist[0];
    hist.lHist[127] = lHist[127];
    hist.rHist[127] = rHist[127];
    hist.gHist[127] = gHist[127];
    hist.bHist[127] = bHist[127];

    //shrink the height just a smidge
    hist.lHistMax = lHistMax * 1.1f;
    hist.rHistMax = rHistMax * 1.1f;
    hist.gHistMax = gHistMax * 1.1f;
    hist.bHistMax = bHistMax * 1.1f;

    //mark histogram as populated
    hist.empty = false;
}

void FilmImageProvider::updateFloatHistogram(Histogram &hist, const matrix<float>& image, const float maximum,
                                             const int rotation,
                                             const float cropHeight, const float cropAspect,
                                             const float cropHoffset, const float cropVoffset)
{
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
    long long lHistMax = 1;
    long long rHistMax = 1;
    long long gHistMax = 1;
    long long bHistMax = 1;

    //calculate crop
    int imWidth  = image.nc()/3;
    int imHeight = image.nr();

    if (rotation == 1 || rotation == 3)//sideways
    {
        imHeight = image.nc()/3;
        imWidth  = image.nr();
    }

    const float tempHeight = imHeight*max(min(1.0f, cropHeight),0.0f);//restrict domain to 0:1
    const float tempAspect = max(min(10000.0f, cropAspect),0.0001f);//restrict aspect ratio
    int width  = int(round(min(tempHeight*tempAspect,float(imWidth))));
    int height = int(round(min(tempHeight, imWidth/tempAspect)));
    const float maxHoffset = (1.0f-(float(width)  / float(imWidth) ))/2.0f;
    const float maxVoffset = (1.0f-(float(height) / float(imHeight)))/2.0f;
    const float oddH = (!(int(round((imWidth  - width )/2.0))*2 == (imWidth  - width )))*0.5f;//it's 0.5 if it's odd, 0 otherwise
    const float oddV = (!(int(round((imHeight - height)/2.0))*2 == (imHeight - height)))*0.5f;//it's 0.5 if it's odd, 0 otherwise
    const float hoffset = (round(max(min(cropHoffset, maxHoffset), -maxHoffset) * imWidth  + oddH) - oddH)/imWidth;
    const float voffset = (round(max(min(cropVoffset, maxVoffset), -maxVoffset) * imHeight + oddV) - oddV)/imHeight;
    int tempStartX = int(round(0.5f*(imWidth  - width ) + hoffset*imWidth));
    int tempStartY = int(round(0.5f*(imHeight - height) + voffset*imHeight));
    int tempEndX = tempStartX + width  - 1;
    int tempEndY = tempStartY + height - 1;

    if (cropHeight <= 0)//it shall be turned off
    {
        tempStartX = 0;
        tempStartY = 0;
        tempEndX = imWidth  - 1;
        tempEndY = imHeight - 1;
        width  = imWidth;
        height = imHeight;
    }

    int startX;
    int startY;
    int endX;
    int endY;
    int lastXindex = imWidth-1;
    int lastYindex = imHeight-1;

    //Rotate the coordinates to match what later happens to the image
    if (rotation == 0) //normal
    {
        startX = tempStartX;
        startY = tempStartY;
        endX = tempEndX;
        endY = tempEndY;
    } else if (rotation == 1) //left side down
    {
        startX = lastYindex - tempEndY;
        endX = lastYindex - tempStartY;
        startY = tempStartX;
        endY = tempEndX;
    } else if (rotation == 2) //upside down
    {
        startX = lastXindex - tempEndX;
        endX = lastXindex - tempStartX;
        startY = lastYindex - tempEndY;
        endY = lastYindex - tempStartY;
    } else// if (rotation == 3) //right side down
    {
        startX = tempStartY;
        endX = tempEndY;
        startY = lastXindex - tempEndX;
        endY = lastXindex - tempStartX;
    }

    #pragma omp parallel for reduction(+: lHist[:128]) reduction(+: rHist[:128]) reduction(+: gHist[:128]) reduction(+: bHist[:128])
    for(int i = startY; i < endY; i = i + 1)
    {
        for(int j = startX*3; j < endX*3; j = j + 3)
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
        lHistMax = (hist.lHist[i] > lHistMax) ? hist.lHist[i] : lHistMax;
        rHistMax = (hist.rHist[i] > rHistMax) ? hist.rHist[i] : rHistMax;
        gHistMax = (hist.gHist[i] > gHistMax) ? hist.gHist[i] : gHistMax;
        bHistMax = (hist.bHist[i] > bHistMax) ? hist.bHist[i] : bHistMax;
    }
    //don't forget to copy 0 and 127
    hist.lHist[0] = lHist[0];
    hist.rHist[0] = rHist[0];
    hist.gHist[0] = gHist[0];
    hist.bHist[0] = bHist[0];
    hist.lHist[127] = lHist[127];
    hist.rHist[127] = rHist[127];
    hist.gHist[127] = gHist[127];
    hist.bHist[127] = bHist[127];

    //shrink the height just a smidge
    hist.lHistMax = lHistMax * 1.1f;
    hist.rHistMax = rHistMax * 1.1f;
    hist.gHistMax = gHistMax * 1.1f;
    hist.bHistMax = bHistMax * 1.1f;

    //mark histogram as populated
    hist.empty = false;
}

void FilmImageProvider::updateHistRaw(const matrix<float>& image, const float maximum,
                                      unsigned cfa[2][2], unsigned xtrans[6][6], int maxXtrans, bool isRGB, bool isMonochrome)
{
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
    long long rHistMax = 1;
    long long gHistMax = 1;
    long long bHistMax = 1;

    //for(int i = 0; i < image.nr(); i = i + 5)
    //    for(int j = 0; j < image.nc(); j = j + 15)
    if (isRGB)
    {
        #pragma omp parallel for reduction(+: rHist[:128]) reduction(+: gHist[:128]) reduction(+: bHist[:128])
        for(int i = 0; i < image.nr(); i = i + 1)
        {
            for(int j = 0; j < image.nc(); j = j + 3)
            {
                rHist[histIndex(image(i,j  ),maximum)]++;
                gHist[histIndex(image(i,j+1),maximum)]++;
                bHist[histIndex(image(i,j+2),maximum)]++;
            }
        }
    }
    else if (isMonochrome)//is 6/6/6 channels but not RGB
    {
        #pragma omp parallel for reduction(+: rHist[:128]) reduction(+: gHist[:128]) reduction(+: bHist[:128])
        for(int i = 0; i < image.nr(); i = i + 1)
        {
            for(int j = 0; j < image.nc(); j = j + 1)
            {
                rHist[histIndex(image(i,j),maximum)]++;
                gHist[histIndex(image(i,j),maximum)]++;
                bHist[histIndex(image(i,j),maximum)]++;
            }
        }
    }
    else if (maxXtrans == 0)//it's bayer
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
    }
    else //it's xtrans
    {
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
        rHistMax = (rawHist.rHist[i] > rHistMax) ? rawHist.rHist[i] : rHistMax;
        gHistMax = (rawHist.gHist[i] > gHistMax) ? rawHist.gHist[i] : gHistMax;
        bHistMax = (rawHist.bHist[i] > bHistMax) ? rawHist.bHist[i] : bHistMax;
    }
    //don't forget to copy 0 and 127
    rawHist.rHist[0] = rHist[0];
    rawHist.gHist[0] = gHist[0];
    rawHist.bHist[0] = bHist[0];
    rawHist.rHist[127] = rHist[127];
    rawHist.gHist[127] = gHist[127];
    rawHist.bHist[127] = bHist[127];

    //shrink the height just a smidge
    rawHist.rHistMax = rHistMax * 1.1f;
    rawHist.gHistMax = gHistMax * 1.1f;
    rawHist.bHistMax = bHistMax * 1.1f;

    //mark histogram as populated
    rawHist.empty = false;
}

inline int FilmImageProvider::histIndex(float value, const float maximum)
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

void FilmImageProvider::updateHistPreFilm(const matrix<float>& image, const float maximum,
                                          const int rotation,
                                          const float cropHeight, const float cropAspect,
                                          const float cropHoffset, const float cropVoffset)
{
    updateFloatHistogram(preFilmHist, image, maximum,
                         rotation,
                         cropHeight, cropAspect, cropHoffset, cropVoffset);
    emit histPreFilmChanged();
    return;
}

void FilmImageProvider::updateHistPostFilm(const matrix<float>& image, const float maximum,
                                           const int rotation,
                                           const float cropHeight, const float cropAspect,
                                           const float cropHoffset, const float cropVoffset)
{
    updateFloatHistogram(postFilmHist, image, maximum,
                         rotation,
                         cropHeight, cropAspect, cropHoffset, cropVoffset);
    emit histPostFilmChanged();
    return;
}

void FilmImageProvider::updateHistFinal(const matrix<unsigned short>& image)
{
    updateShortHistogram(finalHist, image);
    emit histFinalChanged();
    return;
}
