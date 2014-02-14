#include <QDir>
#include <QtGui/QGuiApplication>
#include <QtQml>
#include "qtquick2applicationviewer.h"
#include <QtSql/QSqlDatabase>
#include <QTranslator>
#include "ui/filmimageprovider.h"
#include "database/sqlmodel.h"
#include "database/filmulatordb.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine("qml/Filmulator-Testing-2/main.qml");

    QTranslator translator;
    translator.load("filmulatortr_la");
    app.installTranslator(&translator);

    //Prepare image provider object
    FilmImageProvider *filmProvider = new FilmImageProvider;
    //Connect it as an image provider so that qml can get the photos
    engine.addImageProvider(QLatin1String("filmy"), filmProvider);
    //Connect it as a Q_OBJECT so that qml can run methods
    engine.rootContext()->setContextProperty("filmProvider",filmProvider);

    //Prepare database connection.
    //This should create a new db file if there was none.
    qDebug() << QSqlDatabase::drivers();
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    qDebug() << db.isValid();
    setupDB(&db);


    SqlModel *organizeModel = new SqlModel;
    organizeModel->organizeSetup();
    engine.rootContext()->setContextProperty("organizeModel",organizeModel);



    QObject *topLevel = engine.rootObjects().value(0);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);
    if (!window ) {
        qWarning("Error: your root item has to be a Window");
        return -1;
    }
    window->show();
    qDebug("hi");

    return app.exec();
}
