#include <QtGui/QGuiApplication>
#include <QtQml>
#include "qtquick2applicationviewer.h"
#include <QtSql/QSqlDatabase>
#include <QTranslator>
#include "ui/filmImageProvider.h"
#include "ui/settings.h"
#include "database/importModel.h"
#include "database/organizeModel.h"
#include "database/filmulatorDB.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine("qml/filmulator-gui/main.qml");

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
//    qDebug() << QSqlDatabase::drivers();
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
//    qDebug() << db.isValid();
    setupDB(&db);

    ImportModel *importModel = new ImportModel;
    engine.rootContext()->setContextProperty( "importModel", importModel );

    OrganizeModel *organizeModel = new OrganizeModel;
    engine.rootContext()->setContextProperty( "organizeModel", organizeModel );

    Settings *settingsObj = new Settings;
    engine.rootContext()->setContextProperty( "settings", settingsObj );

    QObject *topLevel = engine.rootObjects().value(0);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);
    if (!window ) {
        qWarning("Error: your root item has to be a Window");
        return -1;
    }
    window->show();

    return app.exec();
}
