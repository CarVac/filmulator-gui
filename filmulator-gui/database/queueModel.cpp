#include "queueModel.h"
#include <iostream>

using std::cout;
using std::endl;

QueueModel::QueueModel(QObject *parent) : SqlModel(parent)
{
    maxIndex = 0;
    tableName = "QueueTable";
    resetIndex();
}

QSqlQuery QueueModel::modelQuery()
{
    QString queryString = "SELECT QueueTable.QTindex AS QTindex ";
    queryString.append("         ,QueueTable.QTprocessed AS QTprocessed ");
    queryString.append("         ,QueueTable.QTexported AS QTexported ");
    queryString.append("         ,QueueTable.QToutput AS QToutput ");
    queryString.append("         ,QueueTable.QTsearchID AS QTsearchID ");
    queryString.append("         ,SearchTable.STrating AS STrating ");
    queryString.append("FROM QueueTable ");
    queryString.append("INNER JOIN SearchTable "
                       "WHERE SearchTable.STsearchID=QueueTable.QTsearchID ");
    queryString.append("ORDER BY ");
    queryString.append("QueueTable.QTindex ASC;");

    QSqlQuery tempQuery(queryString);
    return tempQuery;
}

void QueueModel::setQueueQuery()
{
    setQuery(modelQuery());
    resetIndex();
}

void QueueModel::resetIndex()
{
    QSqlQuery query;
    query.exec("SELECT COUNT(QTindex) FROM QueueTable;");
    query.next();
    maxIndex = query.value( 0 ).toInt();
}

void QueueModel::deQueue(const QString searchID)
{
    QSqlQuery query;
    query.exec("BEGIN TRANSACTION;");
    query.prepare("SELECT QTindex FROM QueueTable WHERE QTsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();
    query.next();
    const int indexRemoved = query.value(0).toInt();
    query.prepare("DELETE FROM QueueTable WHERE QTsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();

    //Update what the largest index is.
    resetIndex();

    //Everything after the index in question needs to be decremented.
    query.prepare("UPDATE QueueTable SET QTindex = QTindex - 1 WHERE QTindex > ?;");
    query.bindValue(0, indexRemoved);
    query.exec();
    query.exec("COMMIT TRANSACTION");

    //Tell the model which row was removed
    beginRemoveRows(QModelIndex(),indexRemoved,indexRemoved);
    queryModel.setQuery(modelQuery());
    while (queryModel.canFetchMore())
    {
        queryModel.fetchMore();
    }
    endRemoveRows();
    //Now tell it to update the indices of all of the rows above that.
    //We update everything just for thoroughness's sake.
    emit dataChanged(createIndex(0,0),createIndex(rowCount(),columnCount()));

}

void QueueModel::enQueue(const QString searchID)
{
    //First check to see if it's already in the queue.
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM QueueTable WHERE QTsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();
    query.next();
    const bool alreadyInqueue = query.value(0).toInt() == 1;

    if (alreadyInqueue)
    {
        //do nothingg
    }
    else
    {

        query.exec("BEGIN TRANSACTION;");
        query.prepare("SELECT STimportTime,STlastProcessedTime FROM SearchTable WHERE STsearchID=?;");
        query.bindValue(0, searchID);
        query.exec();
        query.next();
        const int importTime = query.value(0).toInt();
        const int lastProcessedTime = query.value(1).toInt();

        //When edited and import times were the same, this sometimes led to false positives.
        //I subtract 1 from the lastProcessedTime to give it some buffer for (floating point?) error.
        const bool edited = importTime < (lastProcessedTime-1);
        query.prepare("INSERT OR IGNORE INTO QueueTable "
                      "(QTindex, QTprocessed, QTexported, QToutput, QTsearchID) "
                      "VALUES (?,?,?,?,?);");
        query.bindValue(0, maxIndex);
        query.bindValue(1, edited);
        query.bindValue(2, false);
        query.bindValue(3, false);
        query.bindValue(4, searchID);
        query.exec();
        query.exec("END TRANSACTION;");

        //Tell the model which row was added, then fetch the new data.
        beginInsertRows(QModelIndex(),maxIndex,maxIndex);
        queryModel.setQuery(modelQuery());
        while (queryModel.canFetchMore())
        {
            queryModel.fetchMore();
        }
        endInsertRows();

        //Increment the index.
        maxIndex++;
    }
}

void QueueModel::clearQueue()
{
    QSqlQuery query;
    beginRemoveRows(QModelIndex(),0,maxIndex-1);
    query.exec("DELETE FROM QueueTable");
    resetIndex();
    endRemoveRows();
}

//Move moves the item with searchID to destIndex,
// shifting the others along the way.
//It determines the source index from the database.
void QueueModel::move(const QString searchID, const int destIndex)
{
    QSqlQuery query;

    //Get the source index.
    query.prepare("SELECT QTindex FROM QueueTable  WHERE QTsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();
    query.next();
    const int sourceIndex = query.value(0).toInt();

    query.exec("BEGIN TRANSACTION;");
    if (sourceIndex < destIndex) //Move one to the right, shift the rest leftward
    {
        query.prepare("UPDATE QueueTable "
                      "SET QTindex = QTindex - 1 "
                      "WHERE QTindex > ? AND QTindex <= ?;");
        query.bindValue(0, sourceIndex);
        query.bindValue(1, destIndex);
        query.exec();
        query.prepare("UPDATE QueueTable "
                      "SET QTindex = ? "
                      "WHERE QTsearchID = ?;");
        query.bindValue(0, destIndex);
        query.bindValue(1, searchID);
        query.exec();
    }
    else if (sourceIndex > destIndex) //move one to the left, shift the rest rightward
    {
        query.prepare("UPDATE QueueTable "
                      "SET QTindex = QTindex + 1 "
                      "WHERE QTindex => ? AND QTindex < ?;");
        query.bindValue(0, destIndex);
        query.bindValue(1, sourceIndex);
        query.exec();
        query.prepare("UPDATE QueueTable "
                      "SET QTindex = ? "
                      "WHERE QTsearchID = ?;");
        query.bindValue(0, destIndex);
        query.bindValue(1, searchID);
        query.exec();
    }
    else
    {
        //Do nothing
    }
    query.exec("END TRANSACTION");
    //Now tell it to update all of the indices.
    //We update everything just for thoroughness's sake.
    emit dataChanged(createIndex(0,0),createIndex(rowCount(),columnCount()));
}

void QueueModel::markSaved(const QString searchID)
{
    QSqlQuery query;
    query.prepare("UPDATE QueueTable "
                  "SET QTexported = 1 "
                  "WHERE QTsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();
    updateAll();
}
