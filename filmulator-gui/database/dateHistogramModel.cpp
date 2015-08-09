
#include "dateHistogramModel.h"
#include <QDate>
#include <QString>
#include <iostream>

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
    m_modelQuery = QSqlQuery(query);

    //first, query for the oldest day in the database and the current day
    QSqlQuery dateQuery;

    //Today
    string queryString = "SELECT julianday('NOW', '";
    queryString.append(std::to_string(int(timezone)));
    queryString.append(" hours');");
    dateQuery.exec(QString::fromStdString(queryString));
    dateQuery.first();
    double temp_today = ROUND_JULDAY(dateQuery.value(0).toFloat());

    //Oldest day in the database
    double temp_firstDay;
    //For some reason, this does not match the earliest date bin of the histogram.
    //We can only use this relative to the 'today' computed before.
    queryString = "SELECT julianday(min(SearchTable.STcaptureTime), 'unixepoch', '";
    queryString.append(std::to_string(int(timezone)));
    queryString.append(" hours') FROM SearchTable;");
    dateQuery.exec(QString::fromStdString(queryString));
    if (dateQuery.first())
    {
        temp_firstDay = ROUND_JULDAY(dateQuery.value(0).toFloat());
    } else {
        temp_firstDay = temp_today;
    }
    //The row count of all the data we will eventually serve to the view.
    m_rowCount = temp_today - temp_firstDay + 1;

    cout << "m_rowcount1: " << m_rowCount << endl;

    //Now, we tell the internal model to grab stuff.
    cout << m_modelQuery.lastQuery().toStdString() << endl;
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
    if (m_dataVector.capacity() < (m_rowCount + 1)*VECTORCOUNT)
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
    double firstDay = temp_today;//as a fallback
    for (int i = 0; i < queryRowCount; i++)
    {
        //The first column is supposed to be the Julian day.
        QModelIndex julianDayModelIndex = this->index(i,0);
        //The sixth column is supposed to be the count of images on that day.
        QModelIndex countModelIndex = this->index(i,5);
        double julianDay = queryModel.data(julianDayModelIndex).toDouble();
        if (i==0)
        {
            firstDay = ROUND_JULDAY(julianDay);
        }
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
int DateHistogramModel::rowCount(const QModelIndex &parent) const
{
    return m_rowCount;
}
