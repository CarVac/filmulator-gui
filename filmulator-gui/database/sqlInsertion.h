#ifndef SQLINSERTION_H
#define SQLINSERTION_H

#include <exiv2/exiv2.hpp>
#include <QDateTime>
#include <QString>
#include <iostream>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QDir>

void fileInsert( const QString hash,
                 const QString filePathName,
                 Exiv2::ExifData exifData);
void createNewProfile( const QString fileHash,
                       const QString fileName,
                       const QString absoluteFilePath,
                       const int captureTime,
                       Exiv2::ExifData exifData);

#endif // SQLINSERTION_H
