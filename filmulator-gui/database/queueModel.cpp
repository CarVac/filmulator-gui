#include "queueModel.h"
#include <iostream>

using std::cout;
using std::endl;

QueueModel::QueueModel( QObject *parent ) : SqlModel( parent )
{
    resetIndex();
}

void QueueModel::setQueueQuery()
{
    std::string queryString = "SELECT * ";
    queryString.append( "FROM QueueTable " );
    queryString.append( "ORDER BY " );
    queryString.append( "QueueTable.QTindex ASC;" );

    cout << "queue query: " << queryString << endl;

    setQuery( QSqlQuery( QString::fromStdString( queryString ) ) );
    resetIndex();
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
    emit queueChanged();
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

    emit queueChanged();
}
