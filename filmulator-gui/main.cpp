#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define FILMULATOR_DATADIR ""
#endif

#include <stdlib.h>
#include <QtGui/QGuiApplication>
#include <QtQml>
#include "qtquick2applicationviewer.h"
#include <QtSql/QSqlDatabase>
#include <QTranslator>
#include "ui/filmImageProvider.h"
#include "ui/lensSelectModel.h"
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
    cout << "Have " << argc << " arguments" << endl;
    for (int i = 0; i < argc; i++)
    {
        cout << argv[i] << endl;
    }

    cout << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toStdString() << "creating qapplication" << endl;
    //It cannot properly fall back to Qt Widgets versions of the dialogs if
    // we use a QGuiApplication, which only supports QML stuff.
    //QGuiApplication app(argc, argv);
    QApplication app(argc, argv);

    //This is for the QSettings defaults from things like the qt file dialog and stuff...
    app.setApplicationName("Filmulator");
    app.setOrganizationName("Filmulator");

    QFont sansFont("Sans Serif",9);
    app.setFont(sansFont);

    cout << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toStdString() <<  "creating qqmlapplicationengine" << endl;
    QQmlApplicationEngine engine;

    //Prepare database connection.
    //This should create a new db file if there was none.
    cout << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toStdString() << "connecting to database" << endl;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    if(setupDB(&db) == DBSuccess::failure)
    {
        qWarning("Database prep failed");
        return -1;
    }

    /*
    QTranslator translator;
    cout << QLocale::languageToString(QLocale().language()).toStdString() << endl;
    if (translator.load(QLocale(), QLatin1String("filmulator-gui"), QLatin1String("_"), QLatin1String("qrc:///tl/translations/")))
    {
        cout << "succeeded in loading translation" << endl;
        app.installTranslator(&translator);
        engine.installExtensions(QJSEngine::TranslationExtension);
        engine.setUiLanguage(QLocale::languageToString(QLocale().language()));
    }
    */


    //Create the object for communicating between SQL classes.
    SignalSwitchboard *switchboard = new SignalSwitchboard;

    //Create a settings object for persistent settings.
    cout << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toStdString() << "creating settings object" << endl;
    Settings *settingsObj = new Settings;
    engine.rootContext()->setContextProperty("settings", settingsObj);

    if (settingsObj->getUseSystemLanguage() == false)
    {
        engine.setUiLanguage("English");
        //when more than just German translations are available, have this be filled by another setting
    }

    //Prepare an object for managing the processing parameters.
    cout << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toStdString() << "creating parametermanager" << endl;
    ParameterManager *paramManager = new ParameterManager;
    cout << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toStdString() << "assigning parametermanager property" << endl;
    engine.rootContext()->setContextProperty("paramManager",paramManager);
    cout << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toStdString() << "connecting parametermanager" << endl;
    QObject::connect(paramManager, SIGNAL(updateTableOut(QString, int)),
                     switchboard, SLOT(updateTableIn(QString, int)));

    //Prepare an image provider object.
    cout << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toStdString() << "creating filmimageprovider" << endl;
    FilmImageProvider *filmProvider = new FilmImageProvider(paramManager);
    //Connect it as an image provider so that qml can get the photos
    engine.addImageProvider(QLatin1String("filmy"), filmProvider);
    //Connect it as a Q_OBJECT so that qml can run methods
    engine.rootContext()->setContextProperty("filmProvider",filmProvider);

    qRegisterMetaType<QFileInfo>();

    //Prepare a model for importing.
    cout << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toStdString() << "creating importmodel" << endl;
    ImportModel *importModel = new ImportModel;
    engine.rootContext()->setContextProperty("importModel", importModel);

    //Prepare a model for the organize view.
    cout << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toStdString() << "creating organizemodel" << endl;
    OrganizeModel *organizeModel = new OrganizeModel;
    engine.rootContext()->setContextProperty("organizeModel", organizeModel);
    engine.rootContext()->setContextProperty("dateHistoModel", organizeModel->dateHistogram);
    QObject::connect(switchboard, SIGNAL(updateTableOut(QString,int)),
                     organizeModel, SLOT(updateTable(QString,int)));
    QObject::connect(organizeModel, SIGNAL(updateTableOut(QString,int)),
                     switchboard, SLOT(updateTableIn(QString,int)));

    //Prepare a model for the queue view.
    cout << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toStdString() << "creating queuemodel" << endl;
    QueueModel *queueModel = new QueueModel;
    queueModel->setQueueQuery();
    QObject::connect(switchboard, SIGNAL(updateTableOut(QString, int)),
                     queueModel, SLOT(updateTable(QString, int)));
    QObject::connect(importModel, SIGNAL(enqueueThis(QString)),
                     queueModel, SLOT(enQueue(QString)));
    QObject::connect(organizeModel, SIGNAL(enqueueThis(QString)),
                     queueModel, SLOT(enQueue(QString)));
    engine.rootContext()->setContextProperty("queueModel", queueModel);

    //Prepare a model for the lensfun lens list.
    cout << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toStdString() << "creating lensselectmodel" << endl;
    LensSelectModel *lensModel = new LensSelectModel;
    engine.rootContext()->setContextProperty("lensModel", lensModel);

    QString searchID = "";
    engine.rootContext()->setContextProperty("startOnFilmulate", false);
    if (argc == 2)
    {
        cout << "Importing file!" << endl;
#if (defined(_WIN32) || defined(__WIN32__))
        cout << "main argv: " << argv[1] << endl;
        QString temp = QString::fromLocal8Bit(argv[1]);
        cout << "main argv qstring std: " << temp.toStdString() << endl;
        searchID = importModel->importFileNow(QString::fromLocal8Bit(argv[1]), settingsObj);
#else
        searchID = importModel->importFileNow(QString(argv[1]), settingsObj);
#endif
        if (searchID != "")
        {
            //must be set before loading qml file
            engine.rootContext()->setContextProperty("startOnFilmulate", true);
        } else {
            cout << "Could not import file." << endl;
        }
    }

    cout << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toStdString() << "loading qml file" << endl;
    engine.load("qrc:///qml/qml/filmulator-gui/main.qml");

    if (searchID != "")
    {
        //must be performed after loading qml file
        paramManager->selectImage(searchID);
    }

    cout << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toStdString() << "creating window" << endl;

    QObject *topLevel = engine.rootObjects().value(0);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);
    if (!window) {
        qWarning("Error: your root item has to be a Window");
        return -1;
    }
    window->setIcon(QIcon(":/icons/filmulator64icon.png"));

    cout << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toStdString() << "showing window" << endl;
    window->show();

    cout << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toStdString() << "return" << endl;
    return app.exec();
}
