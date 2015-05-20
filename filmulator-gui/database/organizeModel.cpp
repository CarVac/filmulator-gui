#include "organizeModel.h"
#include <iostream>
#include <QStringList>
#include <QDateTime>
#include <QString>

using std::cout;
using std::endl;
using std::max;
using std::min;

OrganizeModel::OrganizeModel(QObject *parent) :
    SqlModel(parent)
{
    tableName = "SearchTable";
    captureSort = 1;
    dateHistogramSet = false;
}

QSqlQuery OrganizeModel::modelQuery()
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
    std::string queryString = "SELECT * ";
    queryString.append("FROM SearchTable ");
    queryString.append("WHERE ");

    //Here we do the filtering.
    //For now, we always do filtering.
    //--------For unsigned ones, if the max____Time is 0, then we don't filter.
    //--------For signed ones, if the max____ is <0, then we don't filter.

    if (1)//maxCaptureTime != 0)
    {
        queryString.append("SearchTable.STcaptureTime <= ");
        queryString.append(std::to_string(maxCaptureTime_i));
        queryString.append(" ");
        queryString.append("AND SearchTable.STcaptureTime >= ");
        queryString.append(std::to_string(minCaptureTime_i));
        queryString.append(" ");
    }
    if (0)//maxImportTime != 0)
    {
        queryString.append("AND SearchTable.STimportTime <= ");
        queryString.append(std::to_string(maxImportTime_i));
        queryString.append(" ");
        queryString.append("AND SearchTable.STimportTime >= ");
        queryString.append(std::to_string(minImportTime_i));
        queryString.append(" ");
    }
    if (0)//maxProcessedTime != 0)
    {
        queryString.append("AND SearchTable.STlastProcessedTime <= ");
        queryString.append(std::to_string(maxProcessedTime_i));
        queryString.append(" ");
        queryString.append("AND SearchTable.STlastProcessedTime >= ");
        queryString.append(std::to_string(minProcessedTime_i));
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

    return QSqlQuery(QString::fromStdString(queryString));
}

QSqlQuery OrganizeModel::dateHistoQuery()
{
    /*
    Here's the query:

    SELECT
        intlist.thedate
        SearchTable.STcaptureDate
        COUNT(SearchTable.STcaptureTime) AS thecount
    FROM
            (SELECT
                date(julianday('NOW','[timezone] hours') - ints) AS thedate
            FROM
                integers9
            WHERE
                ints <= julianday('NOW') - min(julianday(SearchTable.STcaptureTime, 'unixepoch')) FROM SearchTable
            ) AS intlist
        LEFT JOIN
            (SELECT
                date(searchTable.STcaptureTime, 'unixepoch', '[timezone] hours') AS STcaptureDate,
            FROM
                SearchTable
            WHERE
                    SearchTable.STrating <= [maxRating]
                AND
                    SearchTable.STrating >= [minRating])
        ON
            intlist.thedate = SearchTable.STcaptureDate
    GROUP BY
        intlist.thedate
    SORT BY
        intlist.thedate ASC;


        //Here's the updated query:
    SELECT
        thedate,
        STcaptureDate,
        COUNT(STcaptureTime) AS thecount
    FROM
            (SELECT
                date(julianday('NOW','-4 hours') - ints) AS thedate
            FROM
                integers9
            WHERE
                ints <= julianday('NOW') - (SELECT min(julianday(SearchTable.STcaptureTime, 'unixepoch')) FROM SearchTable)
            ) AS intlist
        LEFT JOIN
            (SELECT
                date(searchTable.STcaptureTime, 'unixepoch', '-4 hours') AS STcaptureDate,
                STcaptureTime
            FROM
                SearchTable
            WHERE
                    SearchTable.STrating <= [maxRating]
                AND
                    SearchTable.STrating >= [minRating])
        ON
            thedate = STcaptureDate
    GROUP BY
        thedate
    ORDER BY
        thedate ASC;
    */

    std::string dateHistoString =
                           "SELECT";
    dateHistoString.append("    thedate,");
    dateHistoString.append("    strftime('%Y/%m', thedate) AS yearmonth,");
    dateHistoString.append("    strftime('%m', thedate) AS themonth,");
    dateHistoString.append("    strftime('%d', thedate) AS theday,");
    dateHistoString.append("    thecount ");
    dateHistoString.append("FROM");
    dateHistoString.append("        (SELECT");
    dateHistoString.append("            date(julianday('NOW','");// hours') - ints) AS thedate,");
    dateHistoString.append(std::to_string(int(m_timeZone)));
    dateHistoString.append(" hours') - ints) AS thedate");//it's space sensitive here!
    dateHistoString.append("        FROM");
    dateHistoString.append("            integers4");
    dateHistoString.append("        WHERE");
    dateHistoString.append("            ints <= julianday('NOW') - (SELECT julianday(min(SearchTable.STcaptureTime), 'unixepoch') FROM SearchTable)");
    dateHistoString.append("        ) AS intlist");
    dateHistoString.append("    LEFT JOIN");
    dateHistoString.append("        (SELECT");
    dateHistoString.append("            date(searchTable.STcaptureTime, 'unixepoch', '");
    dateHistoString.append(std::to_string(int(m_timeZone)));
    dateHistoString.append(" hours') AS capturedate,");
    dateHistoString.append("            COUNT(STcaptureTime) as thecount");
    dateHistoString.append("        FROM");
    dateHistoString.append("            SearchTable");
    if (minRating > 0 || maxRating < 5)
    {
    dateHistoString.append("        WHERE");
    dateHistoString.append("                SearchTable.STrating <= ");
    dateHistoString.append(std::to_string(maxRating));
    dateHistoString.append("            AND");
    dateHistoString.append("                SearchTable.STrating >= ");
    dateHistoString.append(std::to_string(minRating));
    }
    dateHistoString.append("        GROUP BY");
    dateHistoString.append("            capturedate)");
    dateHistoString.append("    ON");
    dateHistoString.append("        thedate = capturedate ");
    dateHistoString.append("ORDER BY");
    dateHistoString.append("    thedate ASC;");

    //std::cout << dateHistoString << std::endl;
    return QSqlQuery(QString::fromStdString(dateHistoString));
}

void OrganizeModel::setOrganizeQuery()
{
    setQuery(modelQuery());
}

void OrganizeModel::setDateHistoQuery()
{
    dateHistogram->setQuery(dateHistoQuery());
    dateHistogramSet = true;
}

QString OrganizeModel::thumbDir()
{
    QDir homeDir = QDir::home();
    homeDir.cd(".local/share/filmulator/thumbs");
    return homeDir.absolutePath();
}

void OrganizeModel::setRating(QString searchID, int rating)
{
    QSqlQuery query;
    query.prepare("UPDATE SearchTable SET STrating = ? WHERE STsearchID = ?;");
    query.bindValue(0, QVariant(max(min(rating,5),0)));
    query.bindValue(1, searchID);
    query.exec();
    emit updateTable("SearchTable", 0);//An edit made to the search table.
}

QString OrganizeModel::getDateTimeString(int unixTimeIn)
{
    QDateTime tempTime = QDateTime::fromTime_t(unixTimeIn, Qt::OffsetFromUTC, m_timeZone*3600);
    return tempTime.toString("ddd yyyy-MM-dd HH:mm:ss");
}

//SELECT date(STcaptureTime, 'unixepoch', '-5 hours [the timezone]', COUNT(*) FROM SearchTable GROUP BY date(STcaptureTime, 'unixepoch', '-5 hours');
