clear;
clc;

initialDeveloperConcentration = single(1);
reservoirThickness = single(1000);
activeLayerThickness = single(0.1);
crystalsPerPixel = single(500);
initialCrystalRadius = single(0.00001);
initialSilverSaltDensity = single(1);
developerConsumptionConst = single(2000000);
crystalGrowthConst = single(0.00001);
silverSaltConsumptionConst = single(2000000);
totalDevelopmentTime = single(100);
agitateCount = single(1);
developmentSteps = single(12);
filmArea = single(864);
sigmaConst = single(0.2);
layerMixConst = single(0.2);
layerTimeDivisor = single(20);

thisAmplitude = 2^15;
inputImage = (2^14)*ones(1000,1000,3);
inputImage(:,500,1) = thisAmplitude;

% inputImage = single(imread('~/Downloads/P1020340.ppm'));

[numRows, numCols, ~] = size(inputImage);

simCrystals = filmSim(inputImage,filmArea,layerMixConst);
simOutput = inputImage.*simCrystals.^2; 

initialData = single(zeros(numRows,numCols,10));
initialData(:,:,1:3) = initialCrystalRadius;
initialData(:,:,4:6) = inputImage*crystalsPerPixel*0.00015387105;
initialData(:,:,7:9) = initialSilverSaltDensity;
initialData(:,:,10)  = initialDeveloperConcentration;
reservoirConcentration = initialDeveloperConcentration;

for i = 1:developmentSteps 
   outData = single(zeros(numRows,numCols,10));
   outReservoirConcentration = single(ones(2,1));
   filmulateIterationGenerator(reservoirConcentration,reservoirThickness, ...
                               crystalGrowthConst,activeLayerThickness, ...
                               developerConsumptionConst,silverSaltConsumptionConst,...
                               totalDevelopmentTime/developmentSteps,filmArea,sigmaConst, ...
                               layerMixConst,layerTimeDivisor,true, ...
                               initialData,outData,outReservoirConcentration);
   initialData = outData;
   reservoirConcentration = outReservoirConcentration(1);
end
realCrystals = outData(:,:,1:3);
realOutput = inputImage.*realCrystals.^2;

figure(1);
plot(1:11,realCrystals(500,495:505,1),1:11,simCrystals(500,495:505,1))

figure(2);
plot(1:1000,realCrystals(500,:,3),1:1000,simCrystals(500,:,3))