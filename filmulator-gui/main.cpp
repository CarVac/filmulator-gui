#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define FILMULATOR_DATADIR ""
#endif

#include "core/logging.h"
#include "database/filmulatorDB.h"
#include "database/importModel.h"
#include "database/organizeModel.h"
#include "database/queueModel.h"
#include "database/signalSwitchboard.h"
#include "qtquick2applicationviewer/qtquick2applicationviewer.h"
#include "ui/filmImageProvider.h"
#include "ui/lensSelectModel.h"
#include "ui/settings.h"
#include <QFileInfo>
#include <QFont>
#include <QIcon>
#include <QMetaType>
#include <QTranslator>
#include <QtGui/QGuiApplication>
#include <QtQml>
#include <QtSql/QSqlDatabase>
#include <QtWidgets/QApplication>
#include <stdlib.h>

#ifdef ENABLE_SPIX_TESTING
#include <Spix/AnyRpcServer.h>
#include <Spix/QtQmlBot.h>
#endif

int main(int argc, char *argv[])
{
  // Force resource initialization for static builds
  Q_INIT_RESOURCE(qml);
  Q_INIT_RESOURCE(pixmaps);

  Filmulator::init_logging();

  bool testMode = false;
  for (int i = 1; i < argc; i++) {
    if (QString(argv[i]) == "--test-mode") {
      testMode = true;
      break;
    }
  }

  FILM_INFO("Have {} arguments", argc);
  for (int i = 0; i < argc; i++) { FILM_DEBUG("  {}", argv[i]); }

  FILM_INFO("Creating QApplication");
  // It cannot properly fall back to Qt Widgets versions of the dialogs if
  //  we use a QGuiApplication, which only supports QML stuff.
  // QGuiApplication app(argc, argv);
  QApplication app(argc, argv);

  // This is for the QSettings defaults from things like the qt file dialog and
  // stuff...
  if (testMode) {
    app.setApplicationName("FilmulatorTest");
    app.setOrganizationName("FilmulatorTest");
  } else {
    app.setApplicationName("Filmulator");
    app.setOrganizationName("Filmulator");
  }

  QFont sansFont("Sans Serif", 9);
  app.setFont(sansFont);

  FILM_INFO("Creating QQmlApplicationEngine");
  QQmlApplicationEngine engine;

  // Prepare database connection.
  // This should create a new db file if there was none.
  FILM_INFO("Connecting to database");
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  if (setupDB(&db) == DBSuccess::failure) {
    qWarning("Database prep failed");
    return -1;
  }

  /*
  QTranslator translator;
  cout << QLocale::languageToString(QLocale().language()).toStdString() << endl;
  if (translator.load(QLocale(), QLatin1String("filmulator-gui"),
  QLatin1String("_"), QLatin1String("qrc:///tl/translations/")))
  {
      cout << "succeeded in loading translation" << endl;
      app.installTranslator(&translator);
      engine.installExtensions(QJSEngine::TranslationExtension);
      engine.setUiLanguage(QLocale::languageToString(QLocale().language()));
  }
  */

  // Create the object for communicating between SQL classes.
  SignalSwitchboard *switchboard = new SignalSwitchboard;

  // Create a settings object for persistent settings.
  FILM_INFO("Creating settings object");
  Settings *settingsObj = new Settings;
  engine.rootContext()->setContextProperty("settings", settingsObj);

  if (settingsObj->getUseSystemLanguage() == false) {
    engine.setUiLanguage("English");
    // when more than just German translations are available, have this be
    // filled by another setting
  }

  // Prepare an object for managing the processing parameters.
  FILM_INFO("Creating ParameterManager");
  ParameterManager *paramManager = new ParameterManager;
  FILM_INFO("Assigning ParameterManager property");
  engine.rootContext()->setContextProperty("paramManager", paramManager);
  FILM_INFO("Connecting ParameterManager");
  QObject::connect(paramManager, SIGNAL(updateTableOut(QString, int)), switchboard, SLOT(updateTableIn(QString, int)));

  // Prepare an image provider object.
  FILM_INFO("Creating FilmImageProvider");
  FilmImageProvider *filmProvider = new FilmImageProvider(paramManager);
  // Connect it as an image provider so that qml can get the photos
  engine.addImageProvider(QLatin1String("filmy"), filmProvider);
  // Connect it as a Q_OBJECT so that qml can run methods
  engine.rootContext()->setContextProperty("filmProvider", filmProvider);

  qRegisterMetaType<QFileInfo>();

  // Prepare a model for importing.
  FILM_INFO("Creating ImportModel");
  ImportModel *importModel = new ImportModel;
  engine.rootContext()->setContextProperty("importModel", importModel);

  // Prepare a model for the organize view.
  FILM_INFO("Creating OrganizeModel");
  OrganizeModel *organizeModel = new OrganizeModel;
  engine.rootContext()->setContextProperty("organizeModel", organizeModel);
  engine.rootContext()->setContextProperty("dateHistoModel", organizeModel->dateHistogram);
  QObject::connect(switchboard, SIGNAL(updateTableOut(QString, int)), organizeModel, SLOT(updateTable(QString, int)));
  QObject::connect(organizeModel, SIGNAL(updateTableOut(QString, int)), switchboard, SLOT(updateTableIn(QString, int)));

  // Prepare a model for the queue view.
  FILM_INFO("Creating QueueModel");
  QueueModel *queueModel = new QueueModel;
  queueModel->setQueueQuery();
  QObject::connect(switchboard, SIGNAL(updateTableOut(QString, int)), queueModel, SLOT(updateTable(QString, int)));
  QObject::connect(importModel, SIGNAL(enqueueThis(QString)), queueModel, SLOT(enQueue(QString)));
  QObject::connect(organizeModel, SIGNAL(enqueueThis(QString)), queueModel, SLOT(enQueue(QString)));
  engine.rootContext()->setContextProperty("queueModel", queueModel);

  // Prepare a model for the lensfun lens list.
  FILM_INFO("Creating LensSelectModel");
  LensSelectModel *lensModel = new LensSelectModel;
  engine.rootContext()->setContextProperty("lensModel", lensModel);

  QString searchID = "";
  engine.rootContext()->setContextProperty("startOnFilmulate", false);
  if (argc == 2 && !testMode) {
    QString arg = QString(argv[1]);
    if (!arg.startsWith("-")) {
      FILM_INFO("Importing file!");
#if (defined(_WIN32) || defined(__WIN32__))
      FILM_DEBUG("main argv: {}", argv[1]);
      QString temp = QString::fromLocal8Bit(argv[1]);
      FILM_DEBUG("main argv qstring std: {}", temp.toStdString());
      searchID = importModel->importFileNow(QString::fromLocal8Bit(argv[1]), settingsObj);
#else
      searchID = importModel->importFileNow(QString(argv[1]), settingsObj);
#endif
      if (searchID != "") {
        // must be set before loading qml file
        engine.rootContext()->setContextProperty("startOnFilmulate", true);
      } else {
        FILM_WARN("Could not import file.");
      }
    }
  }

  FILM_INFO("Loading QML file");
  engine.load("qrc:///qml/qml/filmulator-gui/main.qml");

  if (searchID != "") {
    // must be performed after loading qml file
    paramManager->selectImage(searchID);
  }

  FILM_INFO("Creating window");

  QObject *topLevel = engine.rootObjects().value(0);
  QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);
  if (!window) {
    qWarning("Error: your root item has to be a Window");
    return -1;
  }
  window->setIcon(QIcon(":/icons/filmulator64icon_square.png"));

  FILM_INFO("Showing window");
  window->show();

#ifdef ENABLE_SPIX_TESTING
  spix::AnyRpcServer *server = nullptr;
  spix::QtQmlBot *bot = nullptr;
  if (testMode) {
    FILM_INFO("Starting Spix E2E test server on port 9000...");
    server = new spix::AnyRpcServer();
    bot = new spix::QtQmlBot();
    bot->runTestServer(*server);
    FILM_INFO("Spix setup complete.");
  }
#endif

  FILM_INFO("Return");
  int ret = app.exec();

#ifdef ENABLE_SPIX_TESTING
  if (server) delete server;
  if (bot) delete bot;
#endif

  return ret;
}
