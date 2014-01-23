#include <QtGui/QGuiApplication>
#include "qtquick2applicationviewer.h"
//#include <QtWidgets/QApplication>
#include <QtQml>
#include "core/filmimageprovider.h"
#include <QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlTableModel>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine("qml/Filmulator-Testing-2/main.qml");

    QTranslator translator;
    translator.load("filmulatortr_la");
    app.installTranslator(&translator);

    FilmImageProvider *filmProvider = new FilmImageProvider;

//    engine.addImageProvider(QLatin1String("filmy"), new FilmImageProvider);
    engine.addImageProvider(QLatin1String("filmy"), filmProvider);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/home/carvac/filmulator/sql/photodb1");
        //point at database location.
    bool ok = db.open();
    if(ok)
        qDebug("Database opened");
    else
        qDebug("Database not opened");

    QSqlTableModel *model1 = new QSqlTableModel;
    model1->setTable("locations");

    engine.rootContext()->setContextProperty("filmProvider",filmProvider);


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
