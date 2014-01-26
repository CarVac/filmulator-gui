#include "sqlmodel.h"
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QDebug>
#include <iostream>
#include <QtSql/QSqlError>
#include <QtSql/QSqlField>
#include <QtSql/QSqlQuery>

using namespace std;

SqlModel::SqlModel(QObject *parent) :
    QSqlTableModel(parent)
{
}

void SqlModel::setQuery(const QString &query, const QSqlDatabase &db)
{
    QSqlQueryModel::setQuery(query,db);
    this->error = QSqlQueryModel::lastError();
    cout << "error number: " << this->error.number() << endl;
    cout << "error type: " << this->error.type() << endl;
    cout << "error text: " << this->error.text().toStdString() << endl;
    generateRoleNames();
}

void SqlModel::setTable(const QString &query)
{
    QSqlTableModel::setTable(query);
    QSqlTableModel::select();
    generateRoleNames();
}

void SqlModel::generateRoleNames()
{
    cout << record().count() << endl;
    for(int i=0; i<record().count(); i++)
    {
        this->m_roleNames[Qt::UserRole + i + 1] = record().fieldName(i).toLatin1();
        cout << string(record().fieldName(i).toLatin1()) << endl;
    }
}

QVariant SqlModel::data(const QModelIndex &index, int role) const
{
    QVariant value = QSqlTableModel::data(index,role);
    //Roles are numbers. If it's less than the UserRole constant, it's a custom one.
    if(role < Qt::UserRole)
    {
        value = QSqlTableModel::data(index,role);
    }
    else
    {
        int columnIndex = role - Qt::UserRole-1;
        QModelIndex modelIndex = this->index(index.row(),columnIndex);
        value = QSqlTableModel::data(modelIndex, Qt::DisplayRole);
    }
    return value;
}

QHash<int,QByteArray> SqlModel::roleNames() const
{
    return this->m_roleNames;
}

void SqlModel::test_output() const
{
    cout << "Hi!" << endl;
}

void SqlModel::test_addRecord(QString direc, QString filenam, QString exten)
{
    cout << "hello!" << endl;
    QSqlField field0("id", QVariant::Int);
    field0.setValue(18);
    QSqlField field1("dir", QVariant::String);
    field1.setValue(direc);
    QSqlField field2("name", QVariant::String);
    field2.setValue(filenam);
    QSqlField field3("extension", QVariant::String);
    field3.setValue(exten);
    QSqlRecord record1;
    record1.append(field0);
    record1.append(field1);
    record1.append(field2);
    record1.append(field3);
    cout << record1.value("name").toString().toStdString() << endl;
    bool success;
    success = QSqlTableModel::insertRecord(-1, record1);
    //QString queryString = QString::fromStdString(string("insert into customers values (18, '") + direc.toStdString() +
    //        string("','") + filenam.toStdString() + string("','") + exten.toStdString() + string("');"));
    //QSqlQueryModel query;
    //cout << "trying?" << endl;
    //query.setQuery(queryString, this->database());
    //QSqlTableModel::setQuery("insert into customers values (18, '" << direc <<
    //                         "', '" << filenam << "', '" <<
    //                         exten << "');");
    //success = QSqlTableModel::insertRecord(-8, record1);
    cout << "Success? " << success << endl;
    this->error = QSqlQueryModel::lastError();
    cout << "error number: " << this->error.number() << endl;
    cout << "error type: " << this->error.type() << endl;
    cout << "error text: " << this->error.text().toStdString() << endl;
}
