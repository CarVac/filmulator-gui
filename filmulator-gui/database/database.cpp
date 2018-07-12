#include "database.hpp"

#include <QDir>
#include <QThread>
#include <QStandardPaths>
#include <QString>
#include <iostream>

//Get a unique database connection for each thread.
QSqlDatabase getDB()
{
    //Unique ID for each thread
    const QString threadID = QString::number((quintptr) QThread::currentThreadId(),16);

    QSqlDatabase db = QSqlDatabase::database(threadID);
    //Check if there's already an open connection
    if (db.isOpen() && db.isValid())
    {
        return db;
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", threadID);
    }

    //Now point it at the correct location
    QDir dir = QDir::home();
    QString dirstr = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    dirstr.append("/filmulator");
    if (!dir.cd(dirstr))
    {
        if (!dir.mkpath(dirstr))
        {
            std::cout << "Could not create database directory" << std::endl;
            return db;
        }
        else
        {
            std::cout << "Making database directory" << std::endl;
            dir.cd(dirstr);
        }
    }
    db.setDatabaseName(dir.absoluteFilePath("filmulatorDB"));
    if (!db.open())
    {
        std::cout << "AAAH database is not opening!" << std::endl;
    }
    //this should create the database if it doesn't already exist.
    return db;
}
