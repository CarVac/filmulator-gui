
#include "dateHistogramModel.h"
#include <QDate>
#include <QString>
#include <iostream>

#define ROUND_JULDAY(a) round(a)
#define VECTORCOUNT 2

using namespace std;

DateHistogramModel::DateHistogramModel(QObject *parent) :
    BasicSqlModel(parent)
{
    m_rowCount = 0;
}

/*
 * This function takes in a query (because the selection criteria might be complicated)
 * and the timezone (because it needs that for the time binning
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
void DateHistogramModel::setQuery(const QSqlQuery &query, const int timezone)
{
    beginResetModel();
    m_modelQuery = query;

    //first, query for the oldest day in the database and the current day
    QSqlQuery dateQuery;

    //Today
    string queryString = "SELECT julianday('NOW', '";
    queryString.append(std::to_string(timezone));
    queryString.append(" hours');");
    dateQuery.exec(QString::fromStdString(queryString));
    dateQuery.first();
    m_today = ROUND_JULDAY(dateQuery.value(0).toFloat());

    //Oldest day in the database
    queryString = "SELECT julianday(min(SearchTable.STcaptureTime), 'unixepoch', '";
    queryString.append(std::to_string(timezone));
    queryString.append(" hours') FROM SearchTable;");
    dateQuery.exec(QString::fromStdString(queryString));
    if (dateQuery.first())
    {
        m_firstDay = ROUND_JULDAY(dateQuery.value(0).toFloat());
    } else {
        m_firstDay = m_today;
    }
    //The row count of all the data we will eventually serve to the view.
    m_rowCount = m_today - m_firstDay + 1;

    //Now, we tell the internal model to grab stuff.
    queryModel.setQuery(query);
    while (queryModel.canFetchMore())
    {
        queryModel.fetchMore();
    }
    generateRoleNames();

    //The row count of just the retrieved data
    int rowCount = queryModel.rowCount();

    //Now, make the vector the proper size.
    //It'll be 2x the number of days.
    //The first column will be the unix time, and the second will be the
    //DateHistogramModel::data() will fill in the rest of the info.

    //Preallocate the vector.
    if (m_dataVector.capacity() < m_rowCount*VECTORCOUNT)
    {
        m_dataVector.reserve(m_rowCount*VECTORCOUNT);
    }
    else
    {
        //do nothing. We don't really need to shrink it.
    }

    //Loop over the days in the vector, filling in the julian day and the 0 count
    for (int i = 0; i < m_rowCount; i++)
    {
        m_dataVector[i*VECTORCOUNT] = m_firstDay + i;
        m_dataVector[i*VECTORCOUNT+1] = 0;
    }

    //Loop over the days in the QSqlQueryModel.
    //Write the count in for the appropriate day.
    for (int i = 0; i < rowCount; i++)
    {
        //The first column is supposed to be the Julian day.
        QModelIndex julianDayModelIndex = this->index(i,0);
        //The sixth column is supposed to be the count of images on that day.
        QModelIndex countModelIndex = this->index(i,5);
        double julianDay = queryModel.data(julianDayModelIndex).toDouble();
        int dayIndex = ROUND_JULDAY(julianDay)-m_firstDay;
        int imageCount = queryModel.data(countModelIndex).toInt();
        m_dataVector[dayIndex*VECTORCOUNT+1] = imageCount;
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
int DateHistogramModel::rowCount(const QModelIndex &parent) const
{
    return m_rowCount;
}
