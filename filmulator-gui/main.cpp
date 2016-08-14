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
#include "database/signalSwitchboard.h"
#include <QMetaType>
#include <QFileInfo>
#include <QIcon>
#include <QtWidgets/QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    //It cannot properly fall back to Qt Widgets versions of the dialogs if
    // we use a QGuiApplication, which only supports QML stuff.
    //QGuiApplication app(argc, argv);
    QApplication app(argc, argv);

    QFont sansFont("Sans Serif",9);
    app.setFont(sansFont);
    QQmlApplicationEngine engine;

    QTranslator translator;
    translator.load("filmulatortr_la");
    app.installTranslator(&translator);

    //Prepare database connection.
    //This should create a new db file if there was none.
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    setupDB(&db);

    //Create the object for communicating between SQL classes.
    SignalSwitchboard *switchboard = new SignalSwitchboard;

    //Create a settings object for persistent settings.
    Settings *settingsObj = new Settings;
    engine.rootContext()->setContextProperty("settings", settingsObj);

    //Prepare an object for managing the processing parameters.
    ParameterManager *paramManager = new ParameterManager;
    engine.rootContext()->setContextProperty("paramManager",paramManager);
    QObject::connect(paramManager, SIGNAL(updateTableOut(QString, int)),
                     switchboard, SLOT(updateTableIn(QString, int)));

    //Prepare an image provider object.
    FilmImageProvider *filmProvider = new FilmImageProvider(paramManager);
    //Connect it as an image provider so that qml can get the photos
    engine.addImageProvider(QLatin1String("filmy"), filmProvider);
    //Connect it as a Q_OBJECT so that qml can run methods
    engine.rootContext()->setContextProperty("filmProvider",filmProvider);

    qRegisterMetaType<QFileInfo>();

    //Prepare a model for importing.
    ImportModel *importModel = new ImportModel;
    engine.rootContext()->setContextProperty("importModel", importModel);

    //Prepare a model for the organize view.
    OrganizeModel *organizeModel = new OrganizeModel;
    engine.rootContext()->setContextProperty("organizeModel", organizeModel);
    engine.rootContext()->setContextProperty("dateHistoModel", organizeModel->dateHistogram);
    QObject::connect(switchboard, SIGNAL(updateTableOut(QString,int)),
                     organizeModel, SLOT(updateTable(QString,int)));
    QObject::connect(organizeModel, SIGNAL(updateTableOut(QString,int)),
                     switchboard, SLOT(updateTableIn(QString,int)));

    //Prepare a model for the queue view.
    QueueModel *queueModel = new QueueModel;
    queueModel->setQueueQuery();
    QObject::connect(switchboard, SIGNAL(updateTableOut(QString, int)),
                     queueModel, SLOT(updateTable(QString, int)));
    QObject::connect(importModel, SIGNAL(enqueueThis(QString)),
                     queueModel, SLOT(enQueue(QString)));
    engine.rootContext()->setContextProperty("queueModel", queueModel);

    engine.load(QUrl::fromLocalFile("qml/filmulator-gui/main.qml"));

    QObject *topLevel = engine.rootObjects().value(0);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);
    if (!window) {
        qWarning("Error: your root item has to be a Window");
        return -1;
    }
    window->setIcon(QIcon(":/icons/filmulator64icon.svg"));
    window->show();

    return app.exec();
}
