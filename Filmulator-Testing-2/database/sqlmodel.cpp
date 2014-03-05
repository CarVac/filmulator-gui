#include "sqlmodel.h"
#include <iostream>
#include <QStringList>
#include <exiv2/exiv2.hpp>
#include <QCryptographicHash>

using namespace std;

SqlModel::SqlModel(QObject *parent) :
    QSqlQueryModel(parent)
//    QSqlRelationalTableModel(parent)
{
}

