#include <QtGui/QGuiApplication>
#include "qtquick2applicationviewer.h"
//#include <QtWidgets/QApplication>
#include <QtQml>
#include "core/filmimageprovider.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine("qml/Filmulator-Testing-2/main.qml");
    engine.addImageProvider(QLatin1String("filmy"), new FilmImageProvider);
    QObject *topLevel = engine.rootObjects().value(0);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);
    if (!window ) {
        qWarning("Error: your root item has to be a Window");
        return -1;
    }
    window->show();
    qDebug("hi");

    /*
    QtQuick2ApplicationViewer viewer;
    viewer.setMainQmlFile(QStringLiteral("qml/Filmulator-Testing-2/main.qml"));
    viewer.showExpanded();
    */

    return app.exec();
}
