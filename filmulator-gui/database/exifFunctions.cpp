#include "exifFunctions.h"
#include <libraw/libraw.h>
#include <iostream>
#include <memory>
#include <QStandardPaths>
#include <QDir>
#include <cmath>

using std::cout;
using std::endl;
using std::max;
using std::min;
using std::unique_ptr;

#define IDATA libraw->imgdata.idata
#define LENS  libraw->imgdata.lens
#define MAKER libraw->imgdata.lens.makernotes
#define OTHER libraw->imgdata.other
#define SIZES libraw->imgdata.sizes

QDateTime exifUtcTime(const std::string fullFilename, const int cameraTZ)
{
    const bool isCR3 = QString::fromStdString(fullFilename).endsWith(".cr3", Qt::CaseInsensitive);

    std::unique_ptr<LibRaw> libraw = std::unique_ptr<LibRaw>(new LibRaw());
    const char *cstrfilename = fullFilename.c_str();
    if (0 != libraw->open_file(cstrfilename))
    {
        cout << "exifLocalDateString: Could not read input file!" << endl;
        return QDateTime();
    }

    QDateTime cameraDateTime;
    if (!isCR3) //we can use exiv2
    {
        //Grab the exif data
        auto exifImage = Exiv2::ImageFactory::open(fullFilename);
        exifImage->readMetadata();
        Exiv2::ExifData exifData = exifImage->exifData();

        QString exifDateTime = QString::fromStdString(exifData["Exif.Image.DateTime"].toString());
        if (exifDateTime.length()==0)
        {
            //leica DNG seems to not have DateTime
            exifDateTime = QString::fromStdString(exifData["Exif.Image.DateTimeOriginal"].toString());
        }

        cameraDateTime = QDateTime::fromString(exifDateTime, "yyyy:MM:dd hh:mm:ss");
    } else { //we have to rely on libraw for CR3
        cameraDateTime = QDateTime::fromTime_t(OTHER.timestamp);
    }

    cameraDateTime.setOffsetFromUtc(cameraTZ);

    return cameraDateTime;
}

QString exifLocalDateString(const std::string fullFilename,
                            const int cameraTZ,
                            const int importTZ,
                            const QString dirConfig)
{
    QDateTime captureLocalDateTime =
            QDateTime::fromTime_t(exifUtcTime(fullFilename, cameraTZ).toTime_t());

    captureLocalDateTime.setOffsetFromUtc(importTZ);

    return captureLocalDateTime.toString(dirConfig);
}

std::string exifDateTimeString(time_t time)
{
    return QDateTime::fromTime_t(time).toString("yyyy:MM:dd hh:mm::ss").toStdString();
}

int exifDefaultRotation(const std::string fullFilename)
{
    const bool isCR3 = QString::fromStdString(fullFilename).endsWith(".cr3", Qt::CaseInsensitive);

    std::unique_ptr<LibRaw> libraw = std::unique_ptr<LibRaw>(new LibRaw());
    const char *cstrfilename = fullFilename.c_str();
    if (0 != libraw->open_file(cstrfilename))
    {
        cout << "exifLocalDateString: Could not read input file!" << endl;
        return 0;
    }

    if (!isCR3) // we can use exiv2
    {
        //Grab the exif data
        auto exifImage = Exiv2::ImageFactory::open(fullFilename);
        exifImage->readMetadata();
        Exiv2::ExifData exifData = exifImage->exifData();

        int exifOrientation;
        try
        {
            exifOrientation = (int) exifData["Exif.Image.Orientation"].value().toLong();
        }
        catch (...)
        {
            exifOrientation = 0;
        }
        switch(exifOrientation)
        {
        case 3://upside down
        {
            return 2;
        }
        case 6://right side down
        {
            return 3;
        }
        case 8://left side down
        {
            return 1;
        }
        default:
        {
            return 0;
        }
        }
    } else { //we need to use libraw
        int exifOrientation = SIZES.flip;
        switch (exifOrientation)
        {
        case 3://upside down
            return 2;
        case 5://needs counterclockwise rotation; left side down?
            return 1;
        case 6://needs clockwise rotation; right side down?
            return 3;
        default:
            return 0;
        }
    }
}

//Returns greatest common denominator. From wikipedia.
unsigned int gcd(unsigned int u, unsigned int v)
{
    // simple cases (termination)
    if (u == v) { return u; }

    if (u == 0) { return v; }

    if (v == 0) { return u; }

    // look for factors of 2
    if (~u & 1) // u is even
    {
        if (v & 1) // v is odd
        {
            return gcd(u >> 1, v);
        }
        else // both u and v are even
        {
            return gcd(u >> 1, v >> 1) << 1;
        }
    }

    if (~v & 1) // u is odd, v is even
    {
        return gcd(u, v >> 1);
    }

    // reduce larger argument
    if (u > v)
    {
        return gcd((u - v) >> 1, v);
    }

    return gcd((v - u) >> 1, u);
}

QString fractionalTv(const float shutterSpeed)
{
    if (shutterSpeed == 0)
    {
        return QString("0/1");
    }
    //Two methods we'll have compete with one another

    // Multiply shutter speed by 6000 then round, then GCD with 6000
    int multNumerator = round(shutterSpeed*6000);
    int multDenominator = 6000;
    int multDivisor = gcd(multNumerator, multDenominator);
    multNumerator /= multDivisor;
    multDenominator /= multDivisor;
    float multError = abs(float(multDenominator)/(shutterSpeed*float(multNumerator))-1);

    // Divide 6000 by shutter speed, then GCD with 6000
    int divNumerator = 6000;
    int divDenominator = round(6000.0f/shutterSpeed);
    int divDivisor = gcd(divNumerator, divDenominator);
    divNumerator /= divDivisor;
    divDenominator /= divDivisor;
    float divError = abs(float(divDenominator)/(shutterSpeed*float(divNumerator))-1);

    if (multError < divError)
    {
        return QString("%1/%2").arg(multNumerator).arg(multDenominator);
    } else {
        return QString("%1/%2").arg(divNumerator).arg(divDenominator);
    }
}

Exiv2::Rational rationalTv(const float shutterSpeed)
{
    if (shutterSpeed == 0)
    {
        return Exiv2::Rational(0, 1);
    }
    //Two methods we'll have compete with one another

    // Multiply shutter speed by 6000 then round, then GCD with 6000
    int multNumerator = round(shutterSpeed*6000);
    int multDenominator = 6000;
    int multDivisor = gcd(multNumerator, multDenominator);
    multNumerator /= multDivisor;
    multDenominator /= multDivisor;
    float multError = abs(float(multDenominator)/(shutterSpeed*float(multNumerator))-1);

    // Divide 6000 by shutter speed, then GCD with 6000
    int divNumerator = 6000;
    int divDenominator = round(6000.0f/shutterSpeed);
    int divDivisor = gcd(divNumerator, divDenominator);
    divNumerator /= divDivisor;
    divDenominator /= divDivisor;
    float divError = abs(float(divDenominator)/(shutterSpeed*float(divNumerator))-1);

    if (multError < divError)
    {
        return Exiv2::Rational(multNumerator, multDenominator);
    } else {
        return Exiv2::Rational(divNumerator, divDenominator);
    }
}

Exiv2::Rational rationalAvFL(const float floatIn)
{
    if (floatIn < 0.01)
    {
        return Exiv2::Rational(0, 1);
    }

    int numerator = round(floatIn*100);
    int denominator = 100;
    int divisor = gcd(numerator, denominator);
    numerator /= divisor;
    denominator /= divisor;
    return Exiv2::Rational(numerator, denominator);
}

int exifRating(const std::string fullFilename)
{
    if (QString::fromStdString(fullFilename).endsWith(".cr3",Qt::CaseInsensitive))
    {
        return 0;
    } else {
        auto image = Exiv2::ImageFactory::open(fullFilename); //CHANGE ME
        image->readMetadata();
        Exiv2::ExifData exifData = image->exifData();
        Exiv2::XmpData xmpData = image->xmpData();

        std::string maker = exifData["Exif.Image.Make"].toString();
        if (maker.compare("Canon") == 0)
        {
            return min(5,max(0,(int) xmpData["Xmp.xmp.Rating"].toLong()));
        }
        return 0;
    }
}

float nikonFocalLength(const unsigned int inputFL)
{
    if (inputFL <=   0) {return 4.5;}
    if (inputFL <=  17) {return 8;}
    if (inputFL <=  25) {return 10;}
    if (inputFL <=  26) {return 10.5;}
    if (inputFL <=  28) {return 11;}
    if (inputFL <=  31) {return 12;}
    if (inputFL <=  35) {return 13;}//a coverage guess; there's no electronic 13mm lens to reference against
    if (inputFL <=  36) {return 14;}
    if (inputFL <=  39) {return 15;}
    if (inputFL <=  41) {return 16;}
    if (inputFL <=  43) {return 17;}
    if (inputFL <=  44) {return 18;}
    if (inputFL <=  47) {return 19;}
    if (inputFL <=  48) {return 20;}
    if (inputFL <=  50) {return 21;}
    if (inputFL <=  55) {return 24;}
    if (inputFL <=  56) {return 25;}
    if (inputFL <=  60) {return 28;}
    if (inputFL <=  63) {return 30;}
    if (inputFL <=  68) {return 35;}
    if (inputFL <=  72) {return 40;}
    if (inputFL <=  76) {return 45;}
    if (inputFL <=  80) {return 50;}
    if (inputFL <=  83) {return 55;}
    if (inputFL <=  85) {return 58;}
    if (inputFL <=  86) {return 60;}
    if (inputFL <=  92) {return 70;}
    if (inputFL <=  94) {return 75;}
    if (inputFL <=  97) {return 80;}
    if (inputFL <=  98) {return 85;}
    if (inputFL <= 100) {return 90;}
    if (inputFL <= 104) {return 100;}
    if (inputFL <= 106) {return 105;}
    if (inputFL <= 111) {return 120;}
    if (inputFL <= 112) {return 125;}
    if (inputFL <= 115) {return 135;}
    if (inputFL <= 116) {return 140;}
    if (inputFL <= 118) {return 150;}
    if (inputFL <= 123) {return 170;}
    if (inputFL <= 124) {return 180;}
    if (inputFL <= 128) {return 200;}
    if (inputFL <= 130) {return 210;}
    if (inputFL <= 134) {return 240;}
    if (inputFL <= 136) {return 250;}
    if (inputFL <= 138) {return 270;}
    if (inputFL <= 142) {return 300;}
    if (inputFL <= 151) {return 400;}
    if (inputFL <= 160) {return 500;}
    if (inputFL <= 166) {return 600;}
    if (inputFL <= 176) {return 800;}
    if (inputFL <= 190) {return 1200;}
    if (inputFL <= 202) {return 1700;}
    return 0;
}

QString nikonAperture(const unsigned int inputAperture)
{
    if (inputAperture <=   7) {return "0.95";}//a coverage guess; I don't know what the Z 50/.95 is
    if (inputAperture <=   8) {return "1.2";}
    if (inputAperture <=  12) {return "1.4";}
    if (inputAperture <=  20) {return "1.8";}
    if (inputAperture <=  24) {return "2";}
    if (inputAperture <=  30) {return "2.4";}
    if (inputAperture <=  32) {return "2.5";}
    if (inputAperture <=  35) {return "2.7";}
    if (inputAperture <=  37) {return "2.8";}
    if (inputAperture <=  42) {return "3.3";}
    if (inputAperture <=  44) {return "3.5";}
    if (inputAperture <=  47) {return "3.8";}
    if (inputAperture <=  50) {return "4";}
    if (inputAperture <=  54) {return "4.5";}
    if (inputAperture <=  56) {return "5";}
    if (inputAperture <=  61) {return "5.6";}
    if (inputAperture <=  64) {return "6.3";}
    if (inputAperture <=  67) {return "6.7";}
    if (inputAperture <=  68) {return "7.2";}
    if (inputAperture <=  72) {return "8";}
    if (inputAperture <=  78) {return "9.5";}
    if (inputAperture <=  84) {return "11";}
    if (inputAperture <=  90) {return "13";}
    if (inputAperture <=  94) {return "15";}
    if (inputAperture <=  96) {return "16";}
    if (inputAperture <= 102) {return "19";}
    if (inputAperture <= 108) {return "22";}
    return "";
}

QString exifLens(const std::string fullFilename)
{
    if (fullFilename.length() == 0)
    {
        return "";
    }

    const bool isCR3 = QString::fromStdString(fullFilename).endsWith(".cr3", Qt::CaseInsensitive);

    //Load the image in libraw
    std::unique_ptr<LibRaw> libraw = std::unique_ptr<LibRaw>(new LibRaw());
    const char *cstrfilename = fullFilename.c_str();
    if (0 != libraw->open_file(cstrfilename))
    {
        cout << "exifLens: Could not read input file!" << endl;
        return "";
    }

    //find what the camera is
    //cout << "IDENTIFYING CAMERA ======================================" << endl;
    std::string camMake = IDATA.make;
    //cout << "Camera make:  " << camMake << endl;
    std::string camModel = IDATA.model;
    //cout << "Camera model: " << camModel << endl;

    //find what the lens is
    //cout << "IDENTIFYING LENS ========================================" << endl;
    std::string lensModel = LENS.Lens;
    //cout << "LENS.Lens: " << lensModel << endl;;
    if (lensModel.length() == 0)
    {
        lensModel = MAKER.Lens;
        if (lensModel.length() > 0)
        {
            //cout << "MAKER.Lens: " << lensModel << endl;
        }
    }
    if (!isCR3) //we can't check stuff needing exiv2 if it's CR3
    {
        //Grab the exif data
        auto exifImage = Exiv2::ImageFactory::open(fullFilename);
        exifImage->readMetadata();
        Exiv2::ExifData exifData = exifImage->exifData();

        if (lensModel.length() == 0)
        {
            lensModel = exifData["Exif.Panasonic.LensType"].toString();
            if (lensModel.length() > 0)
            {
                //cout << "Panasonic.LensType: " << lensModel << endl;
            }
        }
        if (lensModel.length() == 0)
        {
            Exiv2::Exifdatum metadatum = exifData["Exif.NikonLd3.LensIDNumber"];
            if (metadatum.toString().length() > 0)
            {
                lensModel = metadatum.print(&exifImage->exifData());
                //cout << "Exif.NikonLd3.LensIDNumber: " << lensModel << endl;
            }
        }
        if (lensModel.length() == 0)
        {
            Exiv2::Exifdatum metadatum = exifData["Exif.Pentax.LensType"];
            if (metadatum.toString().length() > 0)
            {
                lensModel = metadatum.print(&exifImage->exifData());
                //cout << "Exif.Pentax.LensType: " << lensModel << endl;
            }
        }
        if (lensModel.length() == 0)
        {
            Exiv2::Exifdatum metadatum = exifData["Exif.PentaxDng.LensType"];
            if (metadatum.toString().length() > 0)
            {
                lensModel = metadatum.print(&exifImage->exifData());
                //cout << "Exif.PentaxDng.LensType: " << lensModel << endl;
            }
        }
    }
    return QString::fromStdString(lensModel);
}

QString identifyLens(const std::string fullFilename)
{
    if (fullFilename.length() == 0)
    {
        return "";
    }

    const bool isCR3 = QString::fromStdString(fullFilename).endsWith(".cr3", Qt::CaseInsensitive);

    //Load the image in libraw
    std::unique_ptr<LibRaw> libraw = std::unique_ptr<LibRaw>(new LibRaw());
    const char *cstrfilename = fullFilename.c_str();
    if (0 != libraw->open_file(cstrfilename))
    {
        cout << "identifyLens: Could not read input file!" << endl;
        return "";
    }

    lfDatabase *ldb = new lfDatabase;
    QDir dir = QDir::home();
    QString dirstr = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    dirstr.append("/filmulator/version_2");
    std::string stdstring = dirstr.toStdString();
    ldb->Load(stdstring.c_str());

    //(lens)fun stuff here!

    //find what the camera is
    //cout << "IDENTIFYING CAMERA ======================================" << endl;
    std::string camMake = IDATA.make;
    //cout << "Camera make:  " << camMake << endl;
    std::string camModel = IDATA.model;
    //cout << "Camera model: " << camModel << endl;

    //find what the lens is
    //cout << "IDENTIFYING LENS ========================================" << endl;
    std::string lensModel = LENS.Lens;
    //cout << "LENS.Lens: " << lensModel << endl;;
    if (lensModel.length() == 0)
    {
        lensModel = MAKER.Lens;
        if (lensModel.length() > 0)
        {
            //cout << "MAKER.Lens: " << lensModel << endl;
        }
    }
    if (!isCR3) //we can't check stuff neding exiv2 if it's CR3
    {
        //Grab the exif data
        auto exifImage = Exiv2::ImageFactory::open(fullFilename);
        exifImage->readMetadata();
        Exiv2::ExifData exifData = exifImage->exifData();

        if (lensModel.length() == 0)
        {
            lensModel = exifData["Exif.Panasonic.LensType"].toString();
            if (lensModel.length() > 0)
            {
                //cout << "Panasonic.LensType: " << lensModel << endl;
            }
        }
        if (lensModel.length() == 0)
        {
            Exiv2::Exifdatum metadatum = exifData["Exif.NikonLd3.LensIDNumber"];
            if (metadatum.toString().length() > 0)
            {
                lensModel = metadatum.print(&exifImage->exifData());
                //cout << "Exif.NikonLd3.LensIDNumber: " << lensModel << endl;
            }
        }
        if (lensModel.length() == 0)
        {
            Exiv2::Exifdatum metadatum = exifData["Exif.Pentax.LensType"];
            if (metadatum.toString().length() > 0)
            {
                lensModel = metadatum.print(&exifImage->exifData());
                //cout << "Exif.Pentax.LensType: " << lensModel << endl;
            }
        }
        if (lensModel.length() == 0)
        {
            Exiv2::Exifdatum metadatum = exifData["Exif.PentaxDng.LensType"];
            if (metadatum.toString().length() > 0)
            {
                lensModel = metadatum.print(&exifImage->exifData());
                //cout << "Exif.PentaxDng.LensType: " << lensModel << endl;
            }
        }
    }

    //identify the camera model in the database
    cout << "SEARCHING CAMERA MODELS =================================" << endl;
    const lfCamera * camera = NULL;
    const lfCamera ** cameraList = ldb->FindCamerasExt(NULL, camModel.c_str());
    if (cameraList)
    {
        camera = cameraList[0];
        int i = 0;
        while (cameraList[i])
        {
            cout << "Camera: " << cameraList[i]->Model << endl;
            cout << "Crop factor: " << cameraList[i]->CropFactor << endl;
            cout << "Match score: " << cameraList[i]->Score << endl;
            i++;
        }
    } else {
        cout << "No matching cameras found in database." << endl;
    }
    lf_free(cameraList);

    QString lensName = "";
    cout << "SEARCHING LENS MODELS ===================================" << endl;
    if (lensModel.length() > 0)
    {
        const lfLens ** lensList = ldb->FindLenses(camera, NULL, lensModel.c_str());
        if (lensList)
        {
            //We want to exclude truly shitty matches
            //"70-200" matches various 70-200s with scores of 24/100 or less
            if (lensList[0]->Score > 25)
            {
                lensName = QString(lensList[0]->Model);
            }
            /*
            int i = 0;
            while (lensList[i])
            {
                cout << "Maker: " << lensList[i]->Maker << endl;
                cout << "Model: " << lensList[i]->Model << endl;
                cout << "Match score: " << lensList[i]->Score << endl;
                i++;
            }
            */
        } else {
            cout << "No matching lenses found in database." << endl;
        }
        lf_free(lensList);
    }

    if (ldb != NULL)
    {
        delete ldb;
    }
    return lensName;
}
