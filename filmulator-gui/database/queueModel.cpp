#include "queueModel.h"
#include <iostream>

QueueModel::QueueModel( QObject *parent ) : SqlModel( parent )
{
    resetIndex();
}

void QueueModel::setQueueQuery()
{
    std::string queryString = "SELECT * ";
    queryString.append( "FROM QueueTable, SearchTable, ProcTable, FileTable " );
    queryString.append( "WHERE " );
    queryString.append( "QueueTable.QTsearchID = SearchTable.STsearchID " );
    queryString.append( "AND QueueTable.QTsearchID = ProcessingTable.ProcTprocID " );
    queryString.append( "AND SearchTable.STsourceHash = FileTable.FTfileID " );
    queryString.append( "ORDER BY " );
    queryString.append( "QueueTable.QTindex ASC; " );

    setQuery( QSqlQuery( QString::fromStdString( queryString ) ) );
}

void QueueModel::resetIndex()
{
    QSqlQuery query;
    query.exec( "SELECT MAX(QTindex) FROM QueueTable;" );
    query.next();
    index = query.value( 0 ).toInt() + 1;
}

void QueueModel::deQueue( int deleteIndex )
{
    QSqlQuery query;
    query.prepare( "DELETE FROM QueueTable WHERE QTindex = ?;" );
    query.bindValue( 0, deleteIndex);
    query.exec();
    resetIndex();
}

void QueueModel::enQueue( QString searchID )
{
    QSqlQuery query;
    query.prepare( "INSERT OR IGNORE INTO QueueTable VALUES (?,?,?,?);" );
    query.bindValue( 0, index );
    query.bindValue( 1, false );
    query.bindValue( 2, false );
    query.bindValue( 3, searchID );
    query.exec();
    index++;
}
