#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <QSqlDatabase>

//Get a thread-specific connection to the database
QSqlDatabase getDB();


#endif // DATABASE_HPP
