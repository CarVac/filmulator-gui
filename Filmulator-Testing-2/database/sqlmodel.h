#ifndef SQLMODEL_H
#define SQLMODEL_H

#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlQuery>
#include <QDir>
#include <QFile>

//TODO: We need to replace
//a) flags()
//b) setData()
//c) insertRows()
//d) removeRows()
//e) maybe data()
//f) ???

//Look at "Using C++ Models with Qt Quick Views to figure out what else we need.
//It seems that by subclassing QSqlQueryModel we may not need to notify when doing queries...

//Also, we need to replace the convenience functions so that we can more easily set up
//queries. If possible.

class SqlModel : public QSqlQueryModel//QSqlRelationalTableModel
{
    Q_OBJECT
public:
    explicit SqlModel(QObject *parent = 0);


};

#endif // SQLMODEL_H
