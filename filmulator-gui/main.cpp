#include <QtGui/QGuiApplication>
#include <QtQml>
#include "qtquick2applicationviewer.h"
#include <QtSql/QSqlDatabase>
#include <QTranslator>
#include "ui/filmImageProvider.h"
#include "ui/settings.h"
#include "database/importModel.h"
#include "database/organizeModel.h"
#include "database/queueModel.h"
#include "database/filmulatorDB.h"
#include <QMetaType>
#include <QFileInfo>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine("qml/filmulator-gui/main.qml");

    QTranslator translator;
    translator.load("filmulatortr_la");
    app.installTranslator(&translator);

    //Prepare an object for managing the processing parameters.
    ParameterManager *paramManager = new ParameterManager;
    engine.rootContext()->setContextProperty("paramManager",paramManager);

    //Prepare an image provider object.
    FilmImageProvider *filmProvider = new FilmImageProvider(paramManager);
    //Connect it as an image provider so that qml can get the photos
    engine.addImageProvider(QLatin1String("filmy"), filmProvider);
    //Connect it as a Q_OBJECT so that qml can run methods
    engine.rootContext()->setContextProperty("filmProvider",filmProvider);

    qRegisterMetaType<QFileInfo>();

    //Prepare database connection.
    //This should create a new db file if there was none.
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    setupDB(&db);

    //Prepare a model for importing.
    ImportModel *importModel = new ImportModel;
    engine.rootContext()->setContextProperty("importModel", importModel);

    //Prepare a model for the organize view.
    OrganizeModel *organizeModel = new OrganizeModel;
    engine.rootContext()->setContextProperty("organizeModel", organizeModel);
//    std::cout << "Organize row count: " << organizeModel->rowCount() << std::endl;

    //Prepare a model for the queue view.
    QueueModel *queueModel = new QueueModel;
    queueModel->setQueueQuery();
    engine.rootContext()->setContextProperty("queueModel", queueModel);
//    std::cout << "Queue row count: " << queueModel->rowCount() << std::endl;

    //Create a settings object for persistent settings.
    Settings *settingsObj = new Settings;
    engine.rootContext()->setContextProperty("settings", settingsObj);

    QObject *topLevel = engine.rootObjects().value(0);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);
    if (!window) {
        qWarning("Error: your root item has to be a Window");
        return -1;
    }
    window->show();

    return app.exec();
}
