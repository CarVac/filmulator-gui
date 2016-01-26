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
layerMixConst = single(0.5);
layerTimeDivisor = single(20);

numAmps = 5;
numMixes = 10;

sameChannelPeak = zeros(numAmps,numMixes);
otherChannelPeak = zeros(numAmps,numMixes);
curveProfile = zeros(1000,numAmps,numMixes);

pulseAmplitudes = (2*ones(1,numAmps)).^linspace(6,17,numAmps);
layerMixConsts = linspace(0,1,numMixes).^2;
layerMixCoefs = layerMixConsts.^(totalDevelopmentTime/layerTimeDivisor);
for mix = 1:numMixes
    for amp = 1:numAmps
      thisAmplitude = pulseAmplitudes(amp);
      thisLayerMixConst = layerMixConsts(mix);
      
      initialData = single(zeros(1000,1000,10));
      initialData(:,:,1:3) = initialCrystalRadius;
      initialData(:,:,4:6) = 0*crystalsPerPixel*0.00015387105;
      initialData(500,:,4) = thisAmplitude*crystalsPerPixel*0.00015387105;
      initialData(:,:,7:9) = initialSilverSaltDensity;
      initialData(:,:,10)  = initialDeveloperConcentration;
      reservoirConcentration = initialDeveloperConcentration;

      for i = 1:developmentSteps 
         outData = single(zeros(1000,1000,10));
         outReservoirConcentration = single(ones(2,1));
         filmulateIterationGenerator(reservoirConcentration,reservoirThickness, ...
                                     crystalGrowthConst,activeLayerThickness, ...
                                     developerConsumptionConst,silverSaltConsumptionConst, ...
                                     totalDevelopmentTime/developmentSteps,filmArea,sigmaConst, ...
                                     thisLayerMixConst,layerTimeDivisor,true, ...
                                     initialData,outData,outReservoirConcentration);
         initialData = outData;
         reservoirConcentration = outReservoirConcentration(1);
      end

      sameChannelPeak(amp,mix) = outData(500,500,1);
      otherChannelPeak(amp,mix) = mean(outData(500,500,2:3));
      curveProfile(:,amp,mix) = outData(:,500,2);
    end
end
  

% figure(1);
% imagesc(outData(:,:,2));
% 
% xPts = -499:500;
% 
% figure(2);
% baseline = outData(1,500,2);
% peak = outData(500,500,2);
% sigma = 30;
% gaussY = baseline+(peak-baseline)*exp(-xPts.^2/(2*sigma.^2));
% plot(xPts,outData(:,500,2),'b',xPts,gaussY,'r')

figure(2);
mesh(squeeze(curveProfile(1000,:,:)))
title('0 Points');

figure(3);
% plot(pulseAmplitudes,sameChannelPeak-otherChannelPeak,'b',pulseAmplitudes,0.001./(1+(pulseAmplitudes/100000).^(0.75))-0.001,'r');
% plot(layerMixConsts,sameChannelPeak-otherChannelPeak,'b',layerMixConsts, 0.001./(1 + (thisAmplitude/100000).^(0.75) + (layerMixConsts/5000).^(0.3))-0.001,'r');
mesh(sameChannelPeak - squeeze(curveProfile(1000,:,:)))
title('Same channel additional peak');

baseline = 0.00101;
%baseline = squeeze(curveProfile(1000,:,:))
figure(4);
% plot(pulseAmplitudes,otherChannelPeak,'b',pulseAmplitudes,0.00101./(1+(pulseAmplitudes/2000000000).^(0.4)),'r');
mesh((otherChannelPeak - baseline)/(sameChannelPeak(:,2:end) - baseline))
title('Other channel peak ratio');

figure(5);
% plot(pulseAmplitudes,otherChannelPeak,'b',pulseAmplitudes,0.00101./(1+(pulseAmplitudes/2000000000).^(0.4)),'r');
mesh(layerMixCoefs, log2(pulseAmplitudes), otherChannelPeak)
title('Other channel peak ratio');

figure(6);
plot(squeeze(curveProfile(:,end-1,:)));

figure(7);
plot(layerMixCoefs,otherChannelPeak(end,:))

% figure(6);
% plot(sameChannelPeak,otherChannelPeak);