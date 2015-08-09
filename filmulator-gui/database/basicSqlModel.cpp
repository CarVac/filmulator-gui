#include "basicSqlModel.h"
#include <iostream>

BasicSqlModel::BasicSqlModel(QObject *parent) :
    SqlModel(parent)
{
    //
}

void BasicSqlModel::setQuery(const QSqlQuery &query)
{
    m_modelQuery = query;
    SqlModel::setQuery(query);
}
