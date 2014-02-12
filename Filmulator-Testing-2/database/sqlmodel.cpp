#include "sqlmodel.h"
#include <QDebug>
#include <iostream>
#include <QtSql/QSqlQuery>

using namespace std;

SqlModel::SqlModel(QObject *parent) :
    QSqlRelationalTableModel(parent)
{
}

void SqlModel::organizeSetup()
{
    if (__queuemodel)
    {
        return NULL;
    }
    __organizeModel = true;
    __queueModel = false;
}

void SqlModel::organizeQuery()
{
    //We can't use the inbuilt relational table stuff; we have to
    // make our own writing functionality, and instead of setting the table,
    // we have to make our own query.

    //This doesn't support tags yet, unfortunately.
    //Tags will need to each have their own table pointing at SearchTable id's.
    //Then, we would have to add the tag table into the FROM statement
    //and another equality statement there.

    //First we will prepare a string to feed into the query.
    std::string queryString = "SELECT * ";
    queryString.append("FROM SearchTable, FileTable, ProcessingTable ");
    queryString.append("WHERE ");
    queryString.append("SearchTable.id = ProcessingTable.id ");
    queryString.append("AND SearchTable.sourceHash = FileTable.id ");

    //Here we do the filtering.
    //For unsigned ones, if the max____Time is 0, then we don't filter.
    //For signed ones, if the max____ is <0, then we don't filter.
    if (maxCaptureTime != 0)
    {
        queryString.append("AND SearchTable.captureTime <= " +
                           std::string(maxCaptureTime) + " ");
        queryString.append("AND SearchTable.captureTime >= " +
                           std::string(minCaptureTime) + " ");
    }
    if (maxImportTime != 0)
    {
        queryString.append("AND SearchTable.importTime <= " +
                           std::string(maxImportTime) + " ");
        queryString.append("AND SearchTable.importTime >= " +
                           std::string(minImportTime) + " ");
    }
    if (maxProcessedTime != 0)
    {
        queryString.append("AND SearchTable.lastProcessedTime <= " +
                           std::string(maxProcessedTime) + " ");
        queryString.append("AND SearchTable.lastProcessedTime >= " +
                           std::string(minProcessedTime) + " ");
    }
    if (maxRating >= 0)
    {
        queryString.append("AND SearchTable.rating <= " +
                           std::string(maxRating) + " ");
        queryString.append("AND SearchTable.rating >= " +
                           std::string(minRating) + " ");
    }

    //Now we go to the ordering.
    //By default, we will always sort by captureTime and filename,
    //but we want them to be last in case some other sorting method is chosen.
    //It doesn't really matter what the order is other than that, except that
    // we want the rating first because it has actual categories.
    //Any other stuff will have been covered by the filtering.

    //First we need to actually write ORDER BY
    queryString.append("ORDER BY ");

    if (ratingSort == 1){queryString.append("SearchTable.Rating ASC, ");}
    else if (ratingSort == -1){queryString.append("SearchTable.Rating DESC, ");}

    if (processedSort == 1){queryString.append("SearchTable.lastProcessedTime ASC, ");}
    else if (processedSort == -1){queryString.append("SearchTable.lastProcessedTime DESC, ");}

    if (importSort == 1){queryString.append("SearchTable.importTime ASC, ");}
    else if (importSort == -1){queryString.append("searchTable.importTime DESC, ");}

    if (captureSort == 1)
    {
        queryString.append("SearchTable.captureTime ASC, ");
        queryString.append("SearchTable.filename ASC;");
    }
    else if (captureSort == -1)
    {
        queryString.append("SearchTable.captureTime DESC, ");
        queryString.append("SearchTable.filename DESC;");
    }

    setQuery(queryString);
}
