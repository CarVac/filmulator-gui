function outputCrystalRadii = filmSim(inputImage,filmArea,layerMixConst)
  
  defaultCrystalSize = 0.00101*ones(size(inputImage));
  
  developInfluence = 0.001./(1+(inputImage/100000).^(0.75))-0.001; % Actual peak will be at this + 3*0.0134
  
  developSum = sum(developInfluence,3);
  developSumScaled = developSum*3;
  pixelsPerMM = sqrt(size(inputImage,1)*size(inputImage,2)/filmArea);
  sigma = 0.9*pixelsPerMM;
  f_gauss = fspecial('gaussian',161,double(sigma));
  paddedSum = padarray(developSumScaled,[80 80], 'symmetric');
  totalDevelopmentTime = 100;
  layerTimeDivisor = 20;
  mysteriousDivisor = 20;
  layerMixMultiplier = 1 - layerMixConst^(totalDevelopmentTime/(layerTimeDivisor*mysteriousDivisor));
  diffuseInfluence = repmat(filter2(f_gauss,paddedSum,'valid'),[1 1 3])*layerMixMultiplier;
  
  outputCrystalRadii = defaultCrystalSize + diffuseInfluence + developInfluence;
end