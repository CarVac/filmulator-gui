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
    QAbstractTableModel(parent)
{
    //columnCount = 0;
}

void SqlModel::setQuery(const QSqlQuery &query)
{
    beginResetModel();
    //QSqlQueryModel::setQuery( query );
    queryModel.setQuery(query);
    while (queryModel.canFetchMore())
    {
        queryModel.fetchMore();
    }
    generateRoleNames();
    endResetModel();
}

void SqlModel::generateRoleNames()
{
    //cout << record().count() << endl;
    for(int i=0; i<queryModel.record().count(); i++)
    {
        this->m_roleNames[Qt::UserRole + i + 1] = queryModel.record().fieldName(i).toLatin1();
    }
}

QVariant SqlModel::data(const QModelIndex &index, int role) const
{
    QVariant value;
    //Roles are numbers. If it's greater than the UserRole constant, it's a custom one.
    if( role < Qt::UserRole )
    {
        value = queryModel.data( index,role );
        //cout << "SqlModel::data: nonUserRole" << endl;
    }
    else
    {
        int columnIndex = role - Qt::UserRole - 1;
        QModelIndex modelIndex = this->index(index.row(), columnIndex);
        value = queryModel.data( modelIndex, Qt::DisplayRole );
        //cout << "SqlModel::data row: " << index.row() << " column: " << columnIndex << endl;
        //cout << "SqlModel::data row count: " << this->rowCount() << endl;
    }
    return value;
}

int SqlModel::columnCount(const QModelIndex &parent) const
{
    return queryModel.columnCount(parent);
}

int SqlModel::rowCount(const QModelIndex &parent) const
{
    return queryModel.rowCount(parent);
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
        queryModel.setQuery(modelQuery());
        //We don't want to reset the query directly because that leads to bad handling of the change in the views.
        //So we have a virtual function that tells the view that things have changed.
        emit dataChanged(createIndex(0,0),createIndex(rowCount(),columnCount()));
        //emitChange();
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
