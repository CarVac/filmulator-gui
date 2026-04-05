#ifndef FILMULATORDB_H
#define FILMULATORDB_H

#include <QDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

enum DBSuccess { success, failure };

DBSuccess setupDB(QSqlDatabase *db);


#endif// FILMULATORDB_H
