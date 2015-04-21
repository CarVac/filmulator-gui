#include "organizeModel.h"
#include <iostream>
#include <QStringList>
#include <QDateTime>
#include <QString>

using namespace std;

OrganizeModel::OrganizeModel(QObject *parent) :
    SqlModel(parent)
{
    tableName = "SearchTable";
    captureSort = 1;
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

    if (ratingSort == 1){ queryString.append("SearchTable.STRating ASC, "); }
    else if (ratingSort == -1){ queryString.append("SearchTable.STRating DESC, "); }

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

void OrganizeModel::setOrganizeQuery()
{
    setQuery(modelQuery());
}

QString OrganizeModel::thumbDir()
{
    QDir homeDir = QDir::home();
    homeDir.cd(".local/share/filmulator/thumbs");
    return homeDir.absolutePath();
}
