#include "queueModel.h"
#include <iostream>

using std::cout;
using std::endl;

QueueModel::QueueModel(QObject *parent) : SqlModel(parent)
{
    tableName = "QueueTable";
    resetIndex();
}

void QueueModel::setQueueQuery()
{
    std::string queryString = "SELECT * ";
    queryString.append("FROM QueueTable ");
    queryString.append("ORDER BY ");
    queryString.append("QueueTable.QTindex ASC;");

    cout << "queue query: " << queryString << endl;

    setQuery(QSqlQuery(QString::fromStdString(queryString)));
    resetIndex();
}

void QueueModel::resetIndex()
{
    QSqlQuery query;
    query.exec("SELECT MAX(QTindex) FROM QueueTable;");
    query.next();
    index = query.value( 0 ).toInt() + 1;
}

void QueueModel::deQueue(QString searchID)
{
    QSqlQuery query;
    query.prepare("DELETE FROM QueueTable WHERE QTsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();
    resetIndex();
    emit queueChanged();
}

void QueueModel::enQueue(QString searchID)
{
    QSqlQuery query;
    query.prepare("SELECT STimportTime,STlastProcessedTime FROM SearchTable WHERE STsearchID=?;");
    query.bindValue(0, searchID);
    query.exec();
    query.next();
    int importTime = query.value(0).toInt();
    int lastProcessedTime = query.value(1).toInt();
    bool edited = importTime < lastProcessedTime;
    query.prepare("INSERT OR IGNORE INTO QueueTable VALUES (?,?,?,?);");
    query.bindValue(0, index);
    query.bindValue(1, edited);
    query.bindValue(2, false);
    query.bindValue(3, searchID);
    query.exec();
    index++;

    emit queueChanged();
    //emit updateTableOut("QueueTable", 1);
}

void QueueModel::clearQueue()
{
    QSqlQuery query;
    query.exec("DELETE FROM QueueTable");
    resetIndex();
    emit queueChanged();
}
