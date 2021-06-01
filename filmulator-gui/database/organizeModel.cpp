#include "organizeModel.h"
#include "../database/database.hpp"
#include <iostream>
#include <QStringList>
#include <QDateTime>
#include <QString>
#include <QStandardPaths>
#include <QSqlError>

using std::cout;
using std::endl;
using std::max;
using std::min;

OrganizeModel::OrganizeModel(QObject *parent) :
    SqlModel(parent)
{
    tableName = "SearchTable";
    captureSort = 1;
    ratingSort = 0;
    processedSort = 0;
    importSort = 0;
    dateHistogramSet = false;
    startCaptureDate = QDate::currentDate();
    endCaptureDate = startCaptureDate;
    minCaptureDate = startCaptureDate;
    maxCaptureDate = startCaptureDate;
    QDateTime morning = QDateTime(minCaptureDate, QTime(0,0,0,0), Qt::OffsetFromUTC, m_timeZone*3600);
    QDateTime evening = QDateTime(maxCaptureDate, QTime(23,59,59,999), Qt::OffsetFromUTC, m_timeZone*3600);
    minCaptureTime = morning.toSecsSinceEpoch();
    maxCaptureTime = evening.toSecsSinceEpoch();
}

QSqlQuery OrganizeModel::modelQuery()
{
    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();
    return QSqlQuery(adaptableModelQuery(false), db);
}

QString OrganizeModel::adaptableModelQuery(const bool searchIDOnly)
{
    //We can't use the inbuilt relational table stuff; we have to
    // make our own writing functionality, and instead of setting the table,
    // we have to make our own query.

    //This doesn't support tags yet, unfortunately.
    //Tags will need to each have their own table pointing at SearchTable id's.
    //Then, we would have to add the tag table into the FROM statement
    //and another equality statement there.

    //First we will prepare a string to feed into the query.
    //We only really care about info in the searchtable.
    std::string queryString = "SELECT ";
    if (searchIDOnly)
    {
        queryString.append("STsearchID ");
    }  else {
        queryString.append("* ");
    }
    queryString.append("FROM SearchTable ");
    queryString.append("WHERE ");

    //Here we do the filtering.
    //For now, we always do filtering.
    //--------For unsigned ones, if the max____Time is 0, then we don't filter.
    //--------For signed ones, if the max____ is <0, then we don't filter.

    if (1)//maxCaptureTime != 0)
    {
        queryString.append("SearchTable.STcaptureTime <= ");
        queryString.append(std::to_string(maxCaptureTime));
        queryString.append(" ");
        queryString.append("AND SearchTable.STcaptureTime >= ");
        queryString.append(std::to_string(minCaptureTime));
        queryString.append(" ");
    }
    if (0)//maxImportTime != 0)
    {
        queryString.append("AND SearchTable.STimportTime <= ");
        queryString.append(std::to_string(maxImportTime));
        queryString.append(" ");
        queryString.append("AND SearchTable.STimportTime >= ");
        queryString.append(std::to_string(minImportTime));
        queryString.append(" ");
    }
    if (0)//maxProcessedTime != 0)
    {
        queryString.append("AND SearchTable.STlastProcessedTime <= ");
        queryString.append(std::to_string(maxProcessedTime));
        queryString.append(" ");
        queryString.append("AND SearchTable.STlastProcessedTime >= ");
        queryString.append(std::to_string(minProcessedTime));
        queryString.append(" ");
    }
    if (1)//maxRating >= 0)
    {
        queryString.append("AND SearchTable.STrating <= ");
        queryString.append(std::to_string(maxRating));
        queryString.append(" ");
        queryString.append("AND SearchTable.STrating >= ");
        queryString.append(std::to_string(minRating));
        queryString.append(" ");
    }

    //Now we go to the ordering.
    //By default, we will always sort by captureTime and filename,
    //but we want them to be last in case some other sorting method is chosen.
    //It doesn't really matter what the order is other than that, except that
    // we want the rating first because it has actual categories.
    //Any other stuff will have been covered by the filtering.

    //First we need to actually write ORDER BY
    queryString.append("ORDER BY ");

    if (ratingSort == 1){ queryString.append("SearchTable.STrating ASC, "); }
    else if (ratingSort == -1){ queryString.append("SearchTable.STrating DESC, "); }

    if (processedSort == 1){ queryString.append("SearchTable.STlastProcessedTime ASC, "); }
    else if (processedSort == -1){ queryString.append("SearchTable.STlastProcessedTime DESC, "); }

    if (importSort == 1){ queryString.append("SearchTable.STimportTime ASC, "); }
    else if (importSort == -1){ queryString.append("SearchTable.STimportTime DESC, "); }

    if (captureSort == 1)
    {
        queryString.append("SearchTable.STcaptureTime ASC, ");
        queryString.append("SearchTable.STfilename ASC;");
    }
    else //if (captureSort == -1)
    {
        queryString.append("SearchTable.STcaptureTime DESC, ");
        queryString.append("SearchTable.STfilename DESC;");
    }

    return QString::fromStdString(queryString);
}

void OrganizeModel::setOrganizeQuery()
{
    setQuery(modelQuery());
    while(queryModel.canFetchMore())
    {
        queryModel.fetchMore();
    }
//    cout << "organize row count: " << rowCount() << endl;
    m_imageCount = queryModel.rowCount();
    emit imageCountChanged();
}

void OrganizeModel::setDateHistoQuery()
{
    dateHistogram->setQuery(m_timeZone, minRating, maxRating);
    dateHistogramSet = true;
}

QString OrganizeModel::thumbDir()
{
    QDir homeDir(QDir::homePath());
    QString dirstr = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    dirstr.append("/filmulator/thumbs");
    if (homeDir.cd(dirstr))
    {
    } else {
        if (homeDir.mkpath(dirstr))
        {
            if (homeDir.cd(dirstr))
            {
            } else {
                cout << "cannot create thumb directory" << endl;
            }
        } else {
        }
    }
    return dirstr;
}

void OrganizeModel::setRating(const QString searchID, const int rating)
{
    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();

    QSqlQuery query(db);
    query.prepare("UPDATE SearchTable SET STrating = ? WHERE STsearchID = ?;");
    query.bindValue(0, QVariant(max(min(rating,5),-6)));
    query.bindValue(1, searchID);
    if (query.exec())
    {
        //cout << "success" << endl;
    } else {
        cout << "OrganizeMOdel::setRating failure" << endl;
        cout << query.lastError().text().toStdString() << endl;
    }
    query.finish();

    emit updateTableOut("SearchTable", 0);//An edit made to the search table.
    emit updateTableOut("QueueTable", 0);//The queue now reads rating from searchtable.
}

void OrganizeModel::incrementRating(const QString searchID, const int ratingChange)
{
    if (searchID == "")
    {
        return;
    }
    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();

    QSqlQuery query(db);
    query.prepare("SELECT STrating FROM SearchTable WHERE STsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();
    query.next();
    int rating = query.value(0).toInt();

    if (rating < 0)
    {
        //don't change negative ratings unless rating change is positive, and then start it at -1
        if (ratingChange > 0)
        {
            rating = -1 + ratingChange;
        }
    } else if (rating + ratingChange < 0)
    {
        rating = -1;
    } else {
        rating = rating + ratingChange;
    }

    query.prepare("UPDATE SearchTable SET STrating = ? WHERE STsearchID = ?;");
    query.bindValue(0, QVariant(max(min(rating,5),-6)));
    query.bindValue(1, searchID);
    query.exec();
    emit updateTableOut("SearchTable", 0);//An edit made to the search table.
    emit updateTableOut("QueueTable", 0);//The queue now reads rating from searchtable.
}

void OrganizeModel::markDeletion(QString searchID)
{
    //Each thread needs a unique database connection
    QSqlDatabase db = getDB();

    QSqlQuery query(db);
    query.prepare("UPDATE SearchTable SET STrating = -1 - STrating WHERE STsearchID = ?;");
    query.bindValue(0, searchID);
    query.exec();
    query.finish();
    emit updateTableOut("SearchTable", 0);//An edit made to the search table.
    emit updateTableOut("QueueTable", 0);//The queue now reads rating from searchtable.
}

QString OrganizeModel::getDateTimeString(qint64 unixTimeIn)
{
    QDateTime tempTime;
    tempTime.setOffsetFromUtc(m_timeZone*3600);
    tempTime.setSecsSinceEpoch(unixTimeIn);
    return tempTime.toString("ddd yyyy-MM-dd HH:mm:ss");
}
