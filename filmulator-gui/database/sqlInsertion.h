#ifndef SQLINSERTION_H
#define SQLINSERTION_H

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <exiv2/exiv2.hpp>
#include <iostream>

void fileInsert(const QString hash, const QString fullFilename);

QString createNewProfile(const QString fileHash,
  const QString fileName,
  const QDateTime captureTime,
  const QDateTime importTime,
  const std::string fullFilename,
  const bool noThumbnail);

#endif// SQLINSERTION_H
