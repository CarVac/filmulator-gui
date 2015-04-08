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
    QSqlQueryModel(parent)
{
    //columnCount = 0;
}

void SqlModel::setQuery( const QSqlQuery &query )
{
    QSqlQueryModel::setQuery( query );
    generateRoleNames();
}

void SqlModel::generateRoleNames()
{
    //cout << record().count() << endl;
    for( int i=0; i<record().count(); i++ )
    {
        this->m_roleNames[ Qt::UserRole + i + 1 ] = record().fieldName( i ).toLatin1();
    }
}

QVariant SqlModel::data( const QModelIndex &index, int role ) const
{
    QVariant value;
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

//This is for receiving "push notifications" of changes to the database.
void SqlModel::updateTable(QString table, int operation)
{
    if (table != tableName) //ignore other tables' updates
    {
        //cout << "SqlModel::UpdateTable: not for " << tableName.toStdString() << endl;
        return;
    }
    if (operation == 0) //an update of data, not addition or removal
    {
        //It doesn't refresh the internal data model unless you reset the query.
        //QSqlQueryModel::setQuery(modelQuery());
        //We don't want to reset the query directly because that leads to bad handling of the change in the views.
        //So we have a virtual function that tells the view that things have changed.
        emitChange();
        //Eventually we'll have to separate the QSqlQueryModel from the model seen by qml.
        //That'll require us to use the dataChanged() method.
    }
    else
    {
        //cout << "SqlModel::UpdateTable: not a data update" << endl;
        return;
    }
}

//We want a proxy model to hold data that can be updated incrementally.

//Changes from this object:
//Update SQL (for all of them)
//Update the proxy model in-place: insertions and deletions.
//Notify when they happen.

//For changes not initiated directly by the user, we need something less direct.
// For example, if the import thread is going on, all of the imported stuff will be simply marked for changes later.
// When things are removed, they should be grayed out everywhere temporarily.
//When there's an update (not insertion or deletion), it shoudl change everywhere. Unless that affects sorting.
