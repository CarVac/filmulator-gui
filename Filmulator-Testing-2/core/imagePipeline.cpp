#include "imagePipeline.h"

ImagePipeline::ImagePipeline( CacheAndHisto cacheAndHistoIn, QuickQuality qualityIn )
{
    cacheHisto = cacheAndHistoIn;
    quality = qualityIn;

    completionTimes.resize(Valid::count);
    completionTimes[Valid::none] = 0;
    completionTimes[Valid::load] = 5;
    completionTimes[Valid::demosaic] = 50;
    completionTimes[Valid::prefilmulation] = 5;
    completionTimes[Valid::filmulation] = 50;
    completionTimes[Valid::whiteblack] = 10;
    completionTimes[Valid::colorcurve] = 10;
    completionTimes[Valid::filmlikecurve] = 10;

}

matrix<unsigned short> ImagePipeline::processImage( ProcessingParameters params,
                                                    Interface* interface_in,
                                                    bool &aborted,
                                                    Exiv2::ExifData &exifOutput )
{
    //Record when the function was requested. This is so that the function will not give up
    // until a given short time has elapsed.
    gettimeofday( &timeRequested, NULL );
    interface = interface_in;

    setLastValid( params );
    oldParams = params;

    switch ( valid )
    {
    case none://Load image into buffer
    {
        setValid( load );

    }
    case load://Do demosaic
    {
        //If we're about to do work on a new image, we give up.
        if( checkAbort( aborted ) )
        {
            return emptyMatrix();
        }

        cout << "Opening " << params.filenameList[0] << endl;

        //Reads in the photo.
        if(imload(params.filenameList,
                  params.exposureComp,
                  input_image,
                  params.tiffIn,
                  params.jpegIn,
                  exifData,
                  params.highlights,
                  params.caEnabled,
                  LowQuality == quality ) )
        {
            setValid( none );
            return emptyMatrix();
        }

        cout << "ImagePipeline::processImage: Demosaic complete." << endl;
        setValid( demosaic );
    }
    case demosaic://Do pre-filmulation work.
    {
        if( checkAbort( aborted ) )
        {
            return emptyMatrix();
        }

        //Here we apply the exposure compensation and white balance.
        matrix<float> exposureImage = input_image * pow(2, params.exposureComp[0]);
        whiteBalance(exposureImage, pre_film_image, params.temperature, params.tint, params.filenameList[0]);

        if ( NoCacheNoHisto == cacheHisto )
        {
            input_image.set_size( 0, 0 );
        }
        else//( BothCacheAndHisto == cacheHisto)
        {
            //Histogram work
            interface->updateHistPreFilm( pre_film_image, 65535 );
        }

        cout << "ImagePipeline::processImage: Prefilmulation complete." << endl;
        setValid( prefilmulation );

    }
    case prefilmulation://Do filmulation
    {
        if( checkAbort( aborted ) )
        {
            return emptyMatrix();
        }

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
        if( filmulate( pre_film_image, filmulated_image, params.filmParams, this, aborted ) )
        {
            return emptyMatrix();//filmulate returns 1 if it detected an abort
        }

        if ( NoCacheNoHisto == cacheHisto )
        {
            pre_film_image.set_size( 0, 0 );
        }
        else
        {
            //Histogram work
            interface->updateHistPostFilm( filmulated_image, .0025 );//TODO connect this magic number to the qml
        }

        cout << "ImagePipeline::processImage: Filmulation complete." << endl;
        setValid( filmulation );

    }
    case filmulation://Do whitepoint_blackpoint
    {

        //See if the filmulation has been invalidated yet.
        if( checkAbort( aborted ) )
        {
            return emptyMatrix();
        }

        whitepoint_blackpoint(filmulated_image, contrast_image, params.whitepoint,
                              params.blackpoint);

        if ( NoCacheNoHisto == cacheHisto )
        {
            filmulated_image.set_size( 0, 0 );
        }

        setValid( whiteblack );
    }
    case whiteblack: // Do color_curve
    {
        //See if the clipping has been invalidated.
        if ( checkAbort( aborted ) )
        {
            return emptyMatrix();
        }

        //Prepare LUT's for individual color processin.g
        lutR.setUnity();
        lutG.setUnity();
        lutB.setUnity();
        colorCurves(contrast_image, color_curve_image, lutR, lutG, lutB);

        if ( NoCacheNoHisto == cacheHisto )
        {
            contrast_image.set_size( 0, 0 );
        }

        setValid( colorcurve );
    }
    case colorcurve://Do film-like curve
    {
        //See if the color curves applied are now invalid.
        if ( checkAbort( aborted ) )
        {
            return emptyMatrix();
        }

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

        if ( NoCacheNoHisto == cacheHisto )
        {
            color_curve_image.set_size( 0, 0 );
            //film_curve_image is going out of scope anyway.
        }

        setValid( filmlikecurve );
    }
    case filmlikecurve: //output
    {
        //See if the tonecurve has changed since it was applied.
        if ( checkAbort( aborted ) )
        {
            return emptyMatrix();
        }
        matrix<unsigned short> rotated_image;
        rotate_image(vibrance_saturation_image,rotated_image,params.rotation);

        if ( NoCacheNoHisto == cacheHisto )
        {
            vibrance_saturation_image.set_size( 0, 0 );
        }
        else
        {
            interface->updateHistFinal( rotated_image );
        }

        exifOutput = exifData;
        return rotated_image;
    }
    }//End task switch

    return emptyMatrix();
}

bool ImagePipeline::checkAbort( bool aborted )
{
    if ( aborted && timeDiff( timeRequested ) > 0.1 )
    {
        aborted = false;
        return true;
    }
    else
    {
        return false;
    }
}

void ImagePipeline::setValid( Valid validIn )
{
    valid = validIn;
    updateProgress(0);
    return;
}

//This function determines at what stage in the pipeline the function's old parameters were valid to.
//Then, it sets valid to the minimum of its last valid computation and the just-calculated validity.
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
        setValid(tempValid);
    }
    return;
}

void ImagePipeline::updateProgress(float CurrFractionCompleted)
{
    double totalTime = numeric_limits<double>::epsilon();
    double totalCompletedTime = 0;
    for(int i = 0; i < completionTimes.size(); i++)
    {
        totalTime += completionTimes[i];
        float fractionCompleted = 0;
        if(i <= valid)
            fractionCompleted = 1;
        if(i == valid + 1)
            fractionCompleted = CurrFractionCompleted;
        //if greater -> 0
        totalCompletedTime += completionTimes[i]*fractionCompleted;
    }
    cout << "progress: " << totalCompletedTime/totalTime << endl;
    interface->setProgress(totalCompletedTime/totalTime);
}
