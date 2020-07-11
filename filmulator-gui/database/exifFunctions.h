#ifndef EXIFFUNCTIONS_H
#define EXIFFUNCTIONS_H

#include <exiv2/exiv2.hpp>
#include <lensfun/lensfun.h>
#include <QString>
#include <QDateTime>

/*Returns the unix time of the capture*/
QDateTime exifUtcTime(const std::string fullFilename, const int cameraTZ);

/*Returns the configured directory string based on the capture-local date.*/
QString exifLocalDateString(const std::string fullFilename,
                             const int cameraTZ,
                             const int importTZ,
                             const QString dirConfig);

/*Returns yyyy:MM::dd hh:mm:ss from time_t*/
std::string exifDateTimeString(time_t time);

/*Returns the orientation of the image according to the camera.
 * 0 = normal
 * 1 = left side down
 * 2 = upside down
 * 3 = right side down
 * */
int exifDefaultRotation(const std::string fullFilename);

/*Returns the fractional shutter speed field of the exif.*/
QString fractionalTv(const float shutterSpeed);

/*Returns an exiv2 rational shutter speed for use in writing exif*/
Exiv2::Rational rationalTv(const float shutterSpeed);

/*Returns an exiv2 rational value for use in writing exif focal length and aperture*/
Exiv2::Rational rationalAvFL(const float floatIn);

/*Returns the rating of the image from in-camera flagging.
 * If there's no rating, it just gives it 0.
 * For Canons, it does it based on the stars.
 * TODO: for Nikon, give one start for 'protected' images (I think)
 * TODO: others.
 * */
int exifRating(const std::string fullFilename);

/*Translates Nikon metadata focal length from its log representation to mm*/
float nikonFocalLength(const unsigned int inputFL);

/*Translates Nikon metadata aperture from its log representation to fno*/
QString nikonAperture(const unsigned int inputAperture);

/*Find the camera and lens model from image metadata, given the file*/
QString exifLens(const std::string fullFilename);

/*Find a matching lens model from the lensfun database*/
QString identifyLens(const std::string fullFilename);

/*Calculate the greatest common denominator*/
unsigned int gcd(unsigned int u, unsigned int v);

#endif // EXIFFUNCTIONS_H
