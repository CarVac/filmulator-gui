# Add more folders to ship with the application, here
folder_01.source = qml/filmulator-gui
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

# Additional import path used to resolve QML modules in Creator\'s code model
QML_IMPORT_PATH =

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp \
    core/agitate.cpp \
    core/colorCurves.cpp \
    core/colorSpaces.cpp \
    core/curves.cpp \
    core/develop.cpp \
    core/diffuse.cpp \
    core/exposure.cpp \
    core/filmulate.cpp \
    core/imagePipeline.cpp \
    core/imload.cpp \
    core/imread.cpp \
    core/imreadJpeg.cpp \
    core/imreadTiff.cpp \
    core/imwriteJpeg.cpp \
    core/imwriteTiff.cpp \
    core/layerMix.cpp \
    core/mergeExps.cpp \
    core/outputFile.cpp \
    core/rotateImage.cpp \
    core/scale.cpp \
    core/timeDiff.cpp \
    core/vibranceSaturation.cpp \
    core/whiteBalance.cpp \
    core/whitepointBlackpoint.cpp \
    database/basicSqlModel.cpp \
    database/dateHistogramModel.cpp \
    database/dbSetup.cpp \
    database/exifFunctions.cpp \
    database/importModel.cpp \
    database/importProperties.cpp \
    database/importWorker.cpp \
    database/organizeModel.cpp \
    database/organizeProperties.cpp \
    database/queueModel.cpp \
    database/sqlModel.cpp \
    database/sqlInsertion.cpp \
    database/signalSwitchboard.cpp \
    ui/filmImageProvider.cpp \
    ui/parameterManager.cpp \
    ui/settings.cpp \
    ui/thumbWriteWorker.cpp \
    ui/updateHistograms.cpp \
    database/database.cpp

lupdate_only {
SOURCES += qml/filmulator-gui/*.qml \
    qml/filmulator-gui/gui_components/*.qml
}

TRANSLATIONS = translations/filmulator-gui_de.ts

# Installation path
unix:target.path = /usr/lib/filmulator-gui
unix:desktop.path = /usr/share/applications

unix:desktop.files += ./filmulator_gui.desktop

# win32 {
# target.path = ???
# desktop.path = ???
# }
win32:INCLUDEPATH += /usr/include

unix {
script.extra = move_script; install -m 755 -p filmulator
extra.path = /usr/bin
}

# Please do not modify the following two lines. Required for deployment.
include(qtquick2applicationviewer/qtquick2applicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    qml/filmulator-gui/colors.js\
    qml/filmulator-gui/generateHistogram.js \
    qml/filmulator-gui/getRoot.js\
    filmulator

HEADERS += \
    core/filmSim.hpp \
    core/imagePipeline.h \
    core/interface.h \
    core/lut.hpp \
    core/matrix.hpp \
    database/backgroundQueue.h \
    database/basicSqlModel.h \
    database/dateHistogramModel.h \
    database/exifFunctions.h \
    database/filmulatorDB.h \
    database/importModel.h \
    database/importWorker.h \
    database/organizeModel.h \
    database/queueModel.h \
    database/signalSwitchboard.h \
    database/sqlInsertion.h \
    database/sqlModel.h \
    ui/filmImageProvider.h \
    ui/parameterManager.h \
    ui/settings.h \
    ui/thumbWriteWorker.h \
    database/database.hpp


QMAKE_CXXFLAGS += -std=c++14 -DTOUT -O3 -fprefetch-loop-arrays -fopenmp -fno-strict-aliasing -ffast-math
#QMAKE_CFLAGS_DEBUG += -DTOUT -O3 -fprefetch-loop-arrays -fopenmp
QMAKE_LFLAGS += -std=c++14 -O3 -fopenmp

LIBS += -lpthread -ltiff -lexiv2 -ljpeg -lraw_r -lgomp -lrtprocess

QT += sql core quick qml widgets

INSTALLS += desktop extra

RESOURCES += \
    resources/pixmaps.qrc
