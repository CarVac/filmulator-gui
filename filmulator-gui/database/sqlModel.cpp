#include "sqlModel.h"
#include <iostream>
#include <QStringList>
#include <exiv2/exiv2.hpp>
#include <QCryptographicHash>
#include <QSqlRecord>
#include <QModelIndex>
#include <QDebug>

using namespace std;

SqlModel::SqlModel(QObject *parent) :
//    QSqlRelationalTableModel(parent)
    QSqlQueryModel(parent)
{
    columnCount = 0;
}

void SqlModel::setQuery( const QSqlQuery &query )
{
    QSqlQueryModel::setQuery( query );
    generateRoleNames();
}

void SqlModel::generateRoleNames()
{
    //cout << record().count() << endl;
    columnCount = record().count();
    for( int i=0; i<record().count(); i++ )
    {
        this->m_roleNames[ Qt::UserRole + i + 1 ] = record().fieldName( i ).toLatin1();
        //cout << "SqlModel::generateRoleNames: " << string( record().fieldName( i ).toLatin1() ) << endl;
    }
}

QVariant SqlModel::data( const QModelIndex &index, int role ) const
//THIS NEEDS REDOING WITH SQL QUERY MODEL
{
    QVariant value = QSqlQueryModel::data( index, role );
    //Roles are numbers. If it's greater than the UserRole constant, it's a custom one.
    if( role < Qt::UserRole )
    {
        value = QSqlQueryModel::data( index,role );
        //cout << "SqlModel::data: nonUserRole" << endl;
    }
    else
    {
        int columnIndex = role - Qt::UserRole - 1;
        QModelIndex modelIndex = this->index( index.row(), columnIndex );
        value = QSqlQueryModel::data( modelIndex, Qt::DisplayRole );
        //cout << "SqlModel::data row: " << index.row() << " column: " << columnIndex << endl;
        //cout << "SqlModel::data row count: " << this->rowCount() << endl;
    }
    return value;
}

QHash<int,QByteArray> SqlModel::roleNames() const
{
    return m_roleNames;
}

//This is for "push notification" of changes to the database.
void SqlModel::updateTable(QString table, int operation)
{
    if (table != tableName)
    {
        //cout << "SqlModel::UpdateTable: not for " << tableName.toStdString() << endl;
        return;
    }
    if (operation == 0) //(and it's this table)
    {
        //cout << "SqlModel::Update this table: " << tableName.toStdString() << endl;
        //cout << "last row: " << rowCount() - 1 << " last col: " << columnCount - 1 << endl;
        //emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount - 1));
        emit dataChanged(QModelIndex(), QModelIndex());
    }
    else
    {
        //cout << "SqlModel::UpdateTable: not a data update" << endl;
        return;
    }
}
