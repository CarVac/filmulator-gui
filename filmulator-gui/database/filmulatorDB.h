#ifndef FILMULATORDB_H
#define FILMULATORDB_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QDir>

enum DBSuccess {success, failure};

DBSuccess setupDB(QSqlDatabase *db);


#endif // FILMULATORDB_H
