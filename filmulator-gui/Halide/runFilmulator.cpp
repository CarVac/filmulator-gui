#include <iostream>
#include <Halide.h>
//#include "readPNG.h"
//#include "savePNG.h"
#include "halideFilmulate.h"
#include "develop.h"
#include "diffuse.h"
#include "calcLayerMix.h"
#include "calcReservoirConcentration.h"
#include "performLayerMix.h"
#include "setupFilmulator.h"
#include "generateFilmulatedImage.h"
#include "clock.h"

using namespace std;

using Halide::Image;
#include "image_io.h"

int main(){

    int error;
    /*buffer_t inputImage = {0};
    error = readPNG("imageName.png",&inputImage);
    if (error)
      cout << "png error" << endl;
      */
    Halide::Image<float> input = load<float>("P1040567.png");
    Halide::Buffer inputBufferClass = (Halide::Buffer) input;
    buffer_t* inputImage = inputBufferClass.raw_buffer();
    int width = input.width();
    int height = input.height();

    float reservoirConcentration = 1;
    float reservoirThickness = 1000;
    float activeLayerThickness = 0.1;
    float crystalsPerPixel = 500;
    float initialCrystalRadius = 0.00001;
    float initialSilverSaltDensity = 1;
    float developerConsumptionConst = 2000000.0;
    float crystalGrowthConst = 0.00001;
    float silverSaltConsumptionConst = 2000000.0;
    float totalDevelTime = 100;
    int agitateCount = 0;
    int developmentSteps = 12;
    float filmArea = 864;
    float sigmaConst = 0.2;
    float layerMixConst = 0.2;
    float layerTimeDivisor = 20;
    float rolloffBoundary = 51275;

    buffer_t filmulationData = {0};
    float* filmulationDataMemory = new float[10*width*height];
    filmulationData.host = (uint8_t*)filmulationDataMemory;
    filmulationData.stride[0] = 1;
    filmulationData.stride[1] = width;
    filmulationData.stride[2] = width*height;
    filmulationData.extent[0] = width;
    filmulationData.extent[1] = height;
    filmulationData.extent[2] = 10;
    filmulationData.elem_size = 4;
    error = setupFilmulator(inputImage,crystalsPerPixel,initialCrystalRadius,
                            initialSilverSaltDensity, reservoirConcentration,
                            rolloffBoundary,&filmulationData);
    if (error)
      cout << "setup error" << endl;

    buffer_t devConc = {0};
    float* devConcMemory = new float[width*height];
    devConc.host = (uint8_t*)devConcMemory;
    devConc.stride[0] = 1;
    devConc.stride[1] = width;
    devConc.extent[0] = width;
    devConc.extent[1] = height;
    devConc.elem_size = 4;

    buffer_t devMoved = {0};
    float* devMovedMemory = new float[width*height];
    devMoved.host = (uint8_t*)devMovedMemory;
    devMoved.stride[0] = 1;
    devMoved.stride[1] = width;
    devMoved.extent[0] = width;
    devMoved.extent[1] = height;
    devMoved.elem_size = 4;

    buffer_t resBuffer = {0};
    float resMemory;
    resBuffer.host = (uint8_t*)&resMemory;
    resBuffer.stride[0] = 1;
    resBuffer.stride[1] = 1;
    resBuffer.extent[0] = 1;
    resBuffer.extent[1] = 1;
    resBuffer.elem_size = 4;

    double totalDevelopTime = 0;
    double totalDiffuseTime = 0;
    current_time();
    for (int i = 0; i < (developmentSteps-0); i++)
    {
      float timeStep = totalDevelTime/float(developmentSteps);

      double beforeDevelopTime = current_time();
      error = develop(&filmulationData,activeLayerThickness,crystalGrowthConst,
                      developerConsumptionConst,silverSaltConsumptionConst,
                      timeStep,&filmulationData);
      if (error)
        cout << "development error on iteration " << i << endl;
      totalDevelopTime += current_time() - beforeDevelopTime;

      double beforeDiffuseTime = current_time();
      error = diffuse(&filmulationData,filmArea,sigmaConst,timeStep,&devConc);
      if (error)
        cout << "diffuse error on iteration " << i << endl;
      totalDiffuseTime += current_time() - beforeDiffuseTime;

      error = calcLayerMix(&devConc,layerMixConst,layerTimeDivisor,
                           reservoirConcentration,timeStep,&devMoved);
      if (error)
        cout << "calcLayer error on iteration " << i << endl;

      error = calcReservoirConcentration(&devMoved,activeLayerThickness,
                                         filmArea,reservoirConcentration,
                                         reservoirThickness,&resBuffer);
      if (error)
        cout << "calcRes error on iteration " << i << endl;

      reservoirConcentration = resMemory;

      error = performLayerMix(&devMoved,&devConc,&filmulationData,&filmulationData);
      if (error)
        cout << "perfLayer error on iteration " << i << endl;
    }
    cout << "filmulation time: " << current_time() << "ms" << endl;
    cout << "Develop time    : " << totalDevelopTime << "ms" << endl;
    cout << "Diffuse time    : " << totalDiffuseTime << "ms" << endl;
    buffer_t outputImage = {0};
    uint8_t* outputImageMemory= new uint8_t[3*width*height];
    outputImage.host = outputImageMemory;
    outputImage.stride[0] = 1;
    outputImage.stride[1] = width;
    outputImage.stride[2] = width*height;
    outputImage.extent[0] = width;
    outputImage.extent[1] = height;
    outputImage.extent[2] = 3;
    outputImage.elem_size = 1;
    error = generateFilmulatedImage(&filmulationData,&outputImage);
    if (error)
      cout << "output error" << endl;

    /*error = savePNG(&outputImage);
    if (error)
      cout << "save error" << endl;
      */
    Image<uint8_t> output(&outputImage);
    save(output,"filmulationOutput.png");

    return 0;
}
