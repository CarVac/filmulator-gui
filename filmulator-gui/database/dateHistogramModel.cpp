
#include "dateHistogramModel.h"
#include <QDate>
#include <QString>
#include <iostream>
#include <QSqlRecord>

#define ROUND_JULDAY(a) round(a)
//#define ROUND_JULDAY(a) floor(a)
#define VECTORCOUNT 2

using namespace std;

DateHistogramModel::DateHistogramModel(QObject *parent) :
    BasicSqlModel(parent)
{
    m_rowCount = 0;
}

/*
 * This function takes in the binning values:
 *  the timezone
 *  the minRating
 *  the maxRating
 * and sets up a dense dataset from the sparse one returned by the query.
 *
 * The query must have columns for:
 *  The julian day of one of the photos on any given grouped day (julday)
 *  The full yyyy/MM/dd string (thedate)
 *  The yyyy/MM string (yearmonth)
 *  The MM string (themonth)
 *  The dd string (theday)
 *  The count (thecount)
 * in the above order.
 */
void DateHistogramModel::setQuery(const int timezone,
                                  const int minRating,
                                  const int maxRating)
{
    beginResetModel();

    //Set up the query.
    std::string dateHistoString =
                           "SELECT";
    dateHistoString.append("    julianday(unixtime, 'unixepoch', '");
    dateHistoString.append(std::to_string(int(timezone)));
    dateHistoString.append(" hours') AS julday");
    dateHistoString.append("   ,thedate");
    dateHistoString.append("   ,strftime('%Y/%m', thedate) AS yearmonth");
    dateHistoString.append("   ,strftime('%m', thedate) AS themonth");
    dateHistoString.append("   ,strftime('%d', thedate) AS theday");
    dateHistoString.append("   ,thecount ");
    dateHistoString.append("FROM");
    dateHistoString.append("    (SELECT");
    dateHistoString.append("        date(SearchTable.STcaptureTime, 'unixepoch', '");
    dateHistoString.append(std::to_string(int(timezone)));
    dateHistoString.append(" hours') AS thedate");//This SQL is apparently whitespace sensitive.
    dateHistoString.append("       ,COUNT(STcaptureTime) AS thecount");
    dateHistoString.append("       ,STcaptureTime AS unixtime");
    dateHistoString.append("    FROM");
    dateHistoString.append("        SearchTable");
    if (minRating > 0 || maxRating < 5)
    {
    dateHistoString.append("    WHERE");
    dateHistoString.append("            SearchTable.STrating <= ");
    dateHistoString.append(std::to_string(maxRating));
    dateHistoString.append("        AND");
    dateHistoString.append("            SearchTable.STrating >= ");
    dateHistoString.append(std::to_string(minRating));
    }
    dateHistoString.append("    GROUP BY");
    dateHistoString.append("        thedate)");
    dateHistoString.append("ORDER BY");
    dateHistoString.append("    thedate ASC;");
    m_modelQuery = QSqlQuery(QString::fromStdString(dateHistoString));

    QSqlQuery todayQuery;
    //Today
    string queryString = "SELECT julianday('NOW', '";
    queryString.append(std::to_string(int(timezone)));
    queryString.append(" hours');");
    todayQuery.exec(QString::fromStdString(queryString));
    todayQuery.first();
    double today = ROUND_JULDAY(todayQuery.value(0).toFloat());

    //Now we set up the query to get the earliest day
    //We don't do any filtering on this.
    std::string dateHistoString2 =
                            "SELECT";
    dateHistoString2.append("    julianday(unixtime, 'unixepoch', '");
    dateHistoString2.append(std::to_string(int(timezone)));
    dateHistoString2.append(" hours') AS julday");
    dateHistoString2.append("   ,thedate");
    dateHistoString2.append("   ,strftime('%Y/%m', thedate) AS yearmonth");
    dateHistoString2.append("   ,strftime('%m', thedate) AS themonth");
    dateHistoString2.append("   ,strftime('%d', thedate) AS theday");
    dateHistoString2.append("   ,thecount ");
    dateHistoString2.append("FROM");
    dateHistoString2.append("    (SELECT");
    dateHistoString2.append("        date(SearchTable.STcaptureTime, 'unixepoch', '");
    dateHistoString2.append(std::to_string(int(timezone)));
    dateHistoString2.append(" hours') AS thedate");//This SQL is apparently whitespace sensitive.
    dateHistoString2.append("       ,COUNT(STcaptureTime) AS thecount");
    dateHistoString2.append("       ,STcaptureTime AS unixtime");
    dateHistoString2.append("    FROM");
    dateHistoString2.append("        SearchTable");
    dateHistoString2.append("    GROUP BY");
    dateHistoString2.append("        thedate)");
    dateHistoString2.append("ORDER BY");
    dateHistoString2.append("    thedate ASC;");
    QSqlQuery dateQuery = QSqlQuery(QString::fromStdString(dateHistoString2));

    //Oldest day in the database
    double firstDay;
    dateQuery.exec();
    if (dateQuery.first())
    {
        QSqlRecord rec = dateQuery.record();
        int nameCol = rec.indexOf("julday");
        firstDay = min(today, ROUND_JULDAY(dateQuery.value(nameCol).toDouble()));
    }
    else//the db is empty
    {
        firstDay = today;
    }

    //Now we set up the query to get the latest day
    //We don't do any filtering on this.
    std::string dateHistoString3 =
                            "SELECT";
    dateHistoString3.append("    julianday(unixtime, 'unixepoch', '");
    dateHistoString3.append(std::to_string(int(timezone)));
    dateHistoString3.append(" hours') AS julday");
    dateHistoString3.append("   ,thedate");
    dateHistoString3.append("   ,strftime('%Y/%m', thedate) AS yearmonth");
    dateHistoString3.append("   ,strftime('%m', thedate) AS themonth");
    dateHistoString3.append("   ,strftime('%d', thedate) AS theday");
    dateHistoString3.append("   ,thecount ");
    dateHistoString3.append("FROM");
    dateHistoString3.append("    (SELECT");
    dateHistoString3.append("        date(SearchTable.STcaptureTime, 'unixepoch', '");
    dateHistoString3.append(std::to_string(int(timezone)));
    dateHistoString3.append(" hours') AS thedate");//This SQL is apparently whitespace sensitive.
    dateHistoString3.append("       ,COUNT(STcaptureTime) AS thecount");
    dateHistoString3.append("       ,STcaptureTime AS unixtime");
    dateHistoString3.append("    FROM");
    dateHistoString3.append("        SearchTable");
    dateHistoString3.append("    GROUP BY");
    dateHistoString3.append("        thedate)");
    dateHistoString3.append("ORDER BY");
    dateHistoString3.append("    thedate DESC;");
    QSqlQuery dateQuery2 = QSqlQuery(QString::fromStdString(dateHistoString3));

    //Oldest day in the database
    double lastDay;
    dateQuery2.exec();
    if (dateQuery2.first())
    {
        QSqlRecord rec = dateQuery2.record();
        int nameCol = rec.indexOf("julday");
        lastDay = max(today, ROUND_JULDAY(dateQuery2.value(nameCol).toDouble()));
    } else {
        lastDay = today;
    }
    //The row count of all the data we will eventually serve to the view.
    m_rowCount = lastDay - firstDay + 1;

    cout << "m_rowcount1: " << m_rowCount << endl;

    //Now, we tell the internal model to grab stuff.
    queryModel.setQuery(m_modelQuery);
    while (queryModel.canFetchMore())
    {
        queryModel.fetchMore();
    }
    generateRoleNames();

    //The row count of just the retrieved data
    int queryRowCount = queryModel.rowCount();

    //Now, make the vector the proper size.
    //It'll be 2x the number of days.
    //The first column will be the unix time, and the second will be the
    //DateHistogramModel::data() will fill in the rest of the info.

    //Preallocate the vector.
    if ((int) m_dataVector.capacity() < (m_rowCount + 1)*VECTORCOUNT)
    {
        cout << "m_rowcount: " << m_rowCount << endl;
        m_dataVector.reserve((m_rowCount + 1)*VECTORCOUNT);
    }
    else
    {
        //do nothing. We don't really need to shrink it.
    }

    //Loop over the days in the vector, initializing to zero
    for (int i = 0; i < m_rowCount; i++)
    {
        m_dataVector[i*VECTORCOUNT] = 0;
        m_dataVector[i*VECTORCOUNT+1] = 0;
    }


    //Loop over the days in the QSqlQueryModel.
    //Write the count in for the appropriate day.
    for (int i = 0; i < queryRowCount; i++)
    {
        //The first column is supposed to be the Julian day.
        QModelIndex julianDayModelIndex = this->index(i,0);
        //The sixth column is supposed to be the count of images on that day.
        QModelIndex countModelIndex = this->index(i,5);
        double julianDay = queryModel.data(julianDayModelIndex).toDouble();
        int dayIndex = ROUND_JULDAY(julianDay)-firstDay;
        int imageCount = queryModel.data(countModelIndex).toInt();
        m_dataVector[dayIndex*VECTORCOUNT+1] = imageCount;
    }

    //Fill in the empty days with the appropriate value, now that we know firstDay.
    //temp_firstDay isn't to be trusted, except relative to today.
    for (int i = 0; i < m_rowCount; i++)
    {
        m_dataVector[i*VECTORCOUNT] = firstDay + i;
    }
    endResetModel();
}

/*
 * This pulls data from our newly constructed data vector.
 */
QVariant DateHistogramModel::data(const QModelIndex &index, int role) const
{
    QVariant value;
    if (role < Qt::UserRole)
    {
        value = 0;
    }
    else
    {//It's one of our columns.
        if (index.row() >= m_rowCount)
        {
            value = 0;
        }
        else
        {
            qint64 julDay = ROUND_JULDAY(m_dataVector[index.row()*VECTORCOUNT + 0]);
            QDate date = QDate::fromJulianDay(julDay);
            if (role == Qt::UserRole + 1 + 0)
            {//The first column is the Julian day.
                value = m_dataVector[index.row()*VECTORCOUNT + 0];
            }
            else if (role == Qt::UserRole + 1 + 1)
            {//The second column is the full date string.
                value = date.toString("yyyy/MM/dd");
            }
            else if (role == Qt::UserRole + 1 + 2)
            {//The third column is the yearmonth string.
                value = date.toString("yyyy/MM");
            }
            else if (role == Qt::UserRole + 1 + 3)
            {//The fourth column is just the month string.
                value = date.toString("MM");
            }
            else if (role == Qt::UserRole + 1 + 4)
            {//The fifth column is just the day string.
                value = date.toString("dd");
            }
            else if (role == Qt::UserRole + 1 + 5)
            {//The sixth column is the count.
                value = m_dataVector[index.row()*VECTORCOUNT + 1];
            }
            else
            {
                value = 0;
            }
        }
    }
    return value;
}

/*
 * This gets the row count. Not the same as what came from SQL.
 */
int DateHistogramModel::rowCount(const QModelIndex& /*parent*/) const
{
    return m_rowCount;
}
