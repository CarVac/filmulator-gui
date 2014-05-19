#include "imagePipeline.h"

ImagePipeline::ImagePipeline( CacheAndHisto cacheAndHistoIn, Interface* interfaceIn )
{
    cacheAndHistograms = cacheAndHistoIn;
    interface = interfaceIn;
}

matrix<unsigned short> ImagePipeline::processImage( ProcessingParameters params )
{
    switch ( valid )
    {
    case none://Load image into buffer
    {
        interface->setValid( load );

    }
    case load://Do demosaic
    {
        //If we're about to do work on a new image, we give up.
        if(interface->checkAbort(load))
            return emptyMatrix();
//===================================================
        //Mark that demosaicing has started.
        interface->setValid( demosaic );


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
            interface->setValid( none );
            return emptyMatrix();
        }
    }
    case demosaic://Do pre-filmulation work.
    {
        if(interface->checkAbort(demosaic))
            return emptyMatrix();
//============================================================
        //Mark that we've started prefilmulation stuff.
        interface->setValid( prefilmulation );

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
        if( interface->checkAbort( prefilmulation ) )
            return emptyMatrix();
//==============================================================
        //Mark that we've started to filmulate.
        interface->setValid( filmulation );

        //Set up filmulation parameters.
        params.filmParams.initialDevelConcentration = 1.0;
        params.filmParams.reservoirThickness = 1000.0;
        params.filmParams.activeLayerThickness = 0.1;
        params.filmParams.crystalsPerPixel = 500.0;
        params.filmParams.initialCrystalRadius = 0.00001;
        params.filmParams.initialSilverSaltDensity = 1.0;
        params.filmParams.develConsumptionConst = 2000000.0;
        params.filmParams.crystalGrowthConst = 0.00001;
        params.filmParams.silvSaltConsumptionConst = 2000000.0;
        params.filmParams.sigmaConst = 0.2;
        params.filmParams.layerTimeDivisor = 20;
        params.filmParams.rolloffBoundary = 51275;


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
        if( interface->checkAbort( filmulation ) )
            return emptyMatrix();
//=============================================================
        //Mark that we've begun clipping the image and converting to unsigned short.
        interface->setValid( whiteblack );
        whitepoint_blackpoint(filmulated_image, contrast_image, params.whitepoint,
                              params.blackpoint);
    }
    case whiteblack: // Do color_curve
    {
        //See if the clipping has been invalidated.
        if ( interface->checkAbort( whiteblack ) )
            return emptyMatrix();
//=================================================================
        //Mark that we've begun running the individual color curves.
        interface->setValid( colorcurve );
        lutR.setUnity();
        lutG.setUnity();
        lutB.setUnity();
        colorCurves(contrast_image, color_curve_image, lutR, lutG, lutB);
    }
    case colorcurve://Do film-like curve
    {
        //See if the color curves applied are now invalid.
        if ( interface->checkAbort( colorcurve ) )
            return emptyMatrix();
//==================================================================
        //Mark that we've begun applying the all-color tone curve.
        interface->setValid( filmlikecurve );
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
        if ( interface->checkAbort( filmlikecurve ) )
            return emptyMatrix();
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
