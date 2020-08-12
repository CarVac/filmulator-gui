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

void fileInsert(const QString hash,
                const QString fullFilename);

QString createNewProfile(const QString fileHash,
                         const QString fileName,
                         const QDateTime captureTime,
                         const QDateTime importTime,
                         const std::string fullFilename);

#endif // SQLINSERTION_H
