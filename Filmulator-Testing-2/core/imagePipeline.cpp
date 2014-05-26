#include "imagePipeline.h"

ImagePipeline::ImagePipeline( CacheAndHisto cacheAndHistoIn )
{
    cacheAndHistograms = cacheAndHistoIn;
    //interface = &interfaceIn;
}

matrix<unsigned short> ImagePipeline::processImage( ProcessingParameters params, Interface* interface )
{
    //Record when the function was requested. This is so that the function will not give up
    // until a given short time has elapsed.
    gettimeofday( &timeRequested, NULL );

    switch ( valid )
    {
    case none://Load image into buffer
    {
        setValid( load );

    }
    case load://Do demosaic
    {
        //If we're about to do work on a new image, we give up.
        if( checkAbort() )
        {
            return emptyMatrix();
        }
//===================================================
        //Mark that demosaicing has started.
        setValid( demosaic );


        cout << "Opening " << params.filenameList[0] << endl;

        //Reads in the photo.
        if(imload(params.filenameList,
                  params.exposureComp,
                  input_image,
                  params.tiffIn,
                  params.jpegIn,
                  exifData,
                  params.highlights,
                  params.caEnabled))
        {
            setValid( none );
            return emptyMatrix();
        }
    }
    case demosaic://Do pre-filmulation work.
    {
        if( checkAbort() )
        {
            return emptyMatrix();
        }
//============================================================
        //Mark that we've started prefilmulation stuff.
        setValid( prefilmulation );

        //Here we apply the exposure compensation and white balance.
        matrix<float> exposureImage = input_image * pow(2, params.exposureComp[0]);
        //white_balance(exposureImage,pre_film_image,temperature,tint);
        whiteBalance(exposureImage, pre_film_image, params.temperature, params.tint, params.filenameList[0]);

        interface->updateHistPreFilm( pre_film_image, 65535 );

    }
    case prefilmulation://Do filmulation
    {
        //Check to see if what we just did has been invalidated.
        //It'll restart automatically.
        if( checkAbort() )
        {
            return emptyMatrix();
        }
//==============================================================
        //Mark that we've started to filmulate.
        setValid( filmulation );

        //Set up filmulation parameters.
        {
            params.filmParams.initialDeveloperConcentration = 1.0;
            params.filmParams.reservoirThickness = 1000.0;
            params.filmParams.activeLayerThickness = 0.1;
            params.filmParams.crystalsPerPixel = 500.0;
            params.filmParams.initialCrystalRadius = 0.00001;
            params.filmParams.initialSilverSaltDensity = 1.0;
            params.filmParams.developerConsumptionConst = 2000000.0;
            params.filmParams.crystalGrowthConst = 0.00001;
            params.filmParams.silverSaltConsumptionConst = 2000000.0;
            params.filmParams.sigmaConst = 0.2;
            params.filmParams.layerTimeDivisor = 20;
            params.filmParams.rolloffBoundary = 51275;
        }


        //Here we do the film simulation on the image...
        if(filmulate(pre_film_image, filmulated_image, params.filmParams, interface))
        {
            return emptyMatrix();//filmulate returns 1 if it detected an abort
        }

        //Histogram work
        interface->updateHistPostFilm( filmulated_image, .0025 );//TODO connect this magic number to the qml
    }
    case filmulation://Do whitepoint_blackpoint
    {

        //See if the filmulation has been invalidated yet.
        if( checkAbort() )
        {
            return emptyMatrix();
        }
//=============================================================
        //Mark that we've begun clipping the image and converting to unsigned short.
        setValid( whiteblack );
        whitepoint_blackpoint(filmulated_image, contrast_image, params.whitepoint,
                              params.blackpoint);
    }
    case whiteblack: // Do color_curve
    {
        //See if the clipping has been invalidated.
        if ( checkAbort() )
        {
            return emptyMatrix();
        }
//=================================================================
        //Mark that we've begun running the individual color curves.
        setValid( colorcurve );
        lutR.setUnity();
        lutG.setUnity();
        lutB.setUnity();
        colorCurves(contrast_image, color_curve_image, lutR, lutG, lutB);
    }
    case colorcurve://Do film-like curve
    {
        //See if the color curves applied are now invalid.
        if ( checkAbort() )
        {
            return emptyMatrix();
        }
//==================================================================
        //Mark that we've begun applying the all-color tone curve.
        setValid( filmlikecurve );
        filmLikeLUT.fill(
            [=](unsigned short in) -> unsigned short
            {
                float shResult = shadows_highlights( float(in)/65535.0,
                                                     params.shadowsX,
                                                     params.shadowsY,
                                                     params.highlightsX,
                                                     params.highlightsY);
                return 65535*default_tonecurve( shResult );
            }
        );
        matrix<unsigned short> film_curve_image;
        film_like_curve(color_curve_image,film_curve_image,filmLikeLUT);
        vibrance_saturation(film_curve_image,vibrance_saturation_image,params.vibrance,params.saturation);
    }
    case filmlikecurve: //output
    {
        //See if the tonecurve has changed since it was applied.
        if ( checkAbort() )
        {
            return emptyMatrix();
        }
//===================================================================
        //We would mark our progress, but this is the very last step.
        matrix<unsigned short> rotated_image;
        rotate_image(vibrance_saturation_image,rotated_image,params.rotation);

        interface->updateHistFinal( rotated_image );

        return rotated_image;
    }
    }//End task switch

    return emptyMatrix();
}

bool ImagePipeline::checkAbort()
{
    if ( aborted && timeDiff( timeRequested ) > 0.1 )
    {
        pipelineLock.lock();
        aborted = false;
        pipelineLock.unlock();
        return true;
    }
    else
    {
        return false;
    }
}

void ImagePipeline::setValid( Valid validIn )
{
    pipelineLock.lock();
    valid = validIn;
    pipelineLock.unlock();
    return;
}

void ImagePipeline::setLastValid( const ProcessingParameters newParams )
{
    Valid tempValid;

    //First is things that change what are loaded..
    if ( newParams.filenameList != oldParams.filenameList ){ tempValid = none; }
    else if ( newParams.tiffIn != oldParams.tiffIn ){ tempValid = none; }
    else if ( newParams.jpegIn != oldParams.jpegIn ){ tempValid = none; }
    //Next is things that affect demosaicing.
    else if ( newParams.caEnabled != oldParams.caEnabled ){ tempValid = load; }
    else if ( newParams.highlights != oldParams.highlights ){ tempValid = load;}
    //Next is things that affect prefilmulation.
    else if ( newParams.exposureComp != oldParams.exposureComp ){ tempValid = demosaic; }
    else if ( newParams.temperature != oldParams.temperature ){ tempValid = demosaic; }
    else if ( newParams.tint != oldParams.tint ){ tempValid = demosaic; }
    //Next is things that affect filmulation.
    else if ( newParams.filmParams.initialDeveloperConcentration != oldParams.filmParams.initialDeveloperConcentration ){ tempValid = prefilmulation; }
    else if ( newParams.filmParams.reservoirThickness != oldParams.filmParams.reservoirThickness ){ tempValid = prefilmulation; }
    else if ( newParams.filmParams.activeLayerThickness != oldParams.filmParams.activeLayerThickness ){ tempValid = prefilmulation; }
    else if ( newParams.filmParams.crystalsPerPixel != oldParams.filmParams.crystalsPerPixel ){ tempValid = prefilmulation; }
    else if ( newParams.filmParams.initialCrystalRadius != oldParams.filmParams.initialCrystalRadius ){ tempValid = prefilmulation; }
    else if ( newParams.filmParams.initialSilverSaltDensity != oldParams.filmParams.initialSilverSaltDensity ){ tempValid = prefilmulation; }
    else if ( newParams.filmParams.developerConsumptionConst != oldParams.filmParams.developerConsumptionConst ){ tempValid = prefilmulation; }
    else if ( newParams.filmParams.crystalGrowthConst != oldParams.filmParams.crystalGrowthConst ){ tempValid = prefilmulation; }
    else if ( newParams.filmParams.silverSaltConsumptionConst != oldParams.filmParams.silverSaltConsumptionConst ){ tempValid = prefilmulation; }
    else if ( newParams.filmParams.totalDevelTime != oldParams.filmParams.totalDevelTime ){ tempValid = prefilmulation; }
    else if ( newParams.filmParams.agitateCount != oldParams.filmParams.agitateCount ){ tempValid = prefilmulation; }
    else if ( newParams.filmParams.developmentSteps != oldParams.filmParams.developmentSteps ){ tempValid = prefilmulation; }
    else if ( newParams.filmParams.filmArea != oldParams.filmParams.filmArea ){ tempValid = prefilmulation; }
    else if ( newParams.filmParams.sigmaConst != oldParams.filmParams.sigmaConst ){ tempValid = prefilmulation; }
    else if ( newParams.filmParams.layerMixConst != oldParams.filmParams.layerMixConst ){ tempValid = prefilmulation; }
    else if ( newParams.filmParams.layerTimeDivisor != oldParams.filmParams.layerTimeDivisor ){ tempValid = prefilmulation; }
    else if ( newParams.filmParams.rolloffBoundary != oldParams.filmParams.rolloffBoundary ){ tempValid = prefilmulation; }
    //Next is stuff that does contrast.
    else if ( newParams.blackpoint != oldParams.blackpoint ){ tempValid = filmulation; }
    else if ( newParams.whitepoint != oldParams.whitepoint ){ tempValid = filmulation; }
    //next is color curve stuff. Well, there's nothing for now.
    //else if ( newParams.[param] != oldParams.[param] ){ tempValid = whiteblack; }
    //next is other curves.
    else if ( newParams.shadowsX != oldParams.shadowsX ){ tempValid = colorcurve; }
    else if ( newParams.shadowsY != oldParams.shadowsY ){ tempValid = colorcurve; }
    else if ( newParams.highlightsX != oldParams.highlightsX ){ tempValid = colorcurve; }
    else if ( newParams.highlightsY != oldParams.highlightsY ){ tempValid = colorcurve; }
    else if ( newParams.vibrance != oldParams.vibrance ){ tempValid = colorcurve; }
    else if ( newParams.saturation != oldParams.saturation ){ tempValid = colorcurve; }
    //next is rotation.
    else /*if ( newParams.rotation != oldParams.rotation )*/{ tempValid = filmlikecurve; }

    if ( tempValid < valid )
    {
        valid = tempValid;
    }
    return;
}
