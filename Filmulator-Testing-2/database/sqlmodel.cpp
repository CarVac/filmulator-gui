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
    QDir dir = qDir::home();
    if (!dir.cd(".filmulator"))
    {
        dir.mkdir(".filmulator");
        dir.cd(".filmulator");
    }

    __db = QSqlDatabase::addDatabase("QSQLITE");
    __db.setDatabaseName(dir.filePath("filmulatorDB"));
    //this should create a database if it doesn't already exist.

    //We need to set up 3 tables.
    //1. The master table for searching. This should be small
    //  for speed. It points at the other two.
    QSqlQuery query;
    query.exec("create table if not exists SearchTable ("
               "id varchar(32) primary key,"
               "captureTime integer,"//unix time
               "name varchar,"//name of this instance
               "extension varchar,"//extension of source file
               "sourceHash varchar(32),"//primary key in the file table
               "rating integer,"//for easy culling
               "latitude real,"
               "longitude real,"
               "importTime integer,"//unix time
               "lastProcessedTime integer"//unix time
               );

    //2. The table which carries more file info, like exifs and thumbnail locations.
    query.exec("create table if not exists FileTable ("
               "id varchar(32) primary key," //md5sum of the file
               "filePath varchar,"
               "cameraMake varchar,"
               "cameraModel varchar,"
               "sensitivity varchar,"
               "exposureTime real,"
               "aperture real,"
               "focalLength real,"
               "width integer,"
               "height integer"
               );

    //3. The table which holds processing parameters.
    //     This table should have the same number of entries as the SearchTable.
    query.exec("create table if not exists ProcessingTable ("
               "id varchar(32) primary key,"//should be identical to SearchTable
               "initialDeveloperConcentration real,"
               "reservoirThickness real,"
               "activeLayerThickness real,"
               "crystalsPerPixel real,"
               "initialCrystalRadius real,"
               "initialSilverSaltDensity real,"
               "developerConsumptionConst real,"
               "crystalGrowthConst real,"
               "silverSaltConsumption_const real"
               "totalDevelopment_time real,"
               "agitateCount integer,"
               "developmentResolution integer,"
               "filmArea real,"
               "sigmaConst real,"
               "layerMixConst real,"
               "layerTimeDivisor real,"
               "rolloffBoundary integer"
               "exposureComp real,"
               "whitepoint real,"
               "blackpoint real,"
               "shadowsY real,"
               "highlightsY real"
               );

}
void SqlModel::organizeQuery()
{
    //We can't use the inbuilt relational table stuff; we have to
    // make our own writing functionality, and instead of setting the table,
    // we have to make our own query.

    //First we will prepare a string to feed into the query.
    std::string queryString = "SELECT * ";
    queryString.append("FROM SearchTable, FileTable, ProcessingTable ");
    queryString.append("WHERE ");
    queryString.append("SearchTable.id = ProcessingTable.id ");
    queryString.append("AND SearchTable.sourceHash = FileTable.id ");

    //Here we do the filtering.
    //For unsigned ones, if the max____Time is 0, then we don't filter.
    //For signed ones, if the max____ is <0, then we don't filter.

    //Now we go to the ordering.
    //By default, we will always sort by rating, but we want it to be last
    // in case some other sorting method is chosen.
    //It doesn't really matter what the order is other than that, except that
    // we want the rating first because it has actual categories.
    //Any other stuff will be covered by the filtering.


    QSqlQuery selectQuery;
    selectQuery.setQuery("SELECT * "
             "FROM SearchTable, FileTable, ProcessingTable "
             "WHERE "
             "SearchTable.id = ProcessingTable.id "
             "AND SearchTable.sourceHash = FileTable.id "
             "ORDER BY SearchTable.captureTime DESC"
             )
}
