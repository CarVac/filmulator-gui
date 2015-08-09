#ifndef EXIFFUNCTIONS_H
#define EXIFFUNCTIONS_H

#include <exiv2/exiv2.hpp>
#include <QString>
#include <QDateTime>

/*Returns the unix time of the capture*/
QDateTime exifUtcTime(Exiv2::ExifData exifData, const int cameraTZ);

/*Returns the configured directory string based on the capture-local date.*/
QString exifLocalDateString(Exiv2::ExifData exifData,
                             const int cameraTZ,
                             const int importTZ,
                             const QString dirConfig);

/*Returns the orientation of the image according to the camera.
 * 0 = normal
 * 1 = left side down
 * 2 = upside down
 * 3 = right side down
 * */
int exifDefaultRotation(Exiv2::ExifData exifData);

/*Returns the manufacturer field of the exif.*/
QString exifMake(Exiv2::ExifData exifData);

/*Returns the camera name field of the exif.*/
QString exifModel(Exiv2::ExifData exifData);

/*Returns the ISO sensitivity field of the exif.*/
float exifIso(Exiv2::ExifData exifData);

/*Returns the fractional shutter speed field of the exif.*/
QString exifTv(Exiv2::ExifData exifData);

/*Returns the aperture number field of the exif.*/
float exifAv(Exiv2::ExifData exifData);

/*Returns the focal length field of the exif.*/
float exifFl(Exiv2::ExifData exifData);

/*Returns the rating of the image from in-camera flagging.
 * If there's no rating, it just gives it 0.
 * For Canons, it does it based on the stars.
 * TODO: for Nikon, give one start for 'protected' images (I think)
 * TODO: others.
 * */
int exifRating(Exiv2::ExifData exifData, Exiv2::XmpData xmpData);

#endif // EXIFFUNCTIONS_H
