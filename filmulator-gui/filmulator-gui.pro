# Add more folders to ship with the application, here
folder_01.source = qml/filmulator-gui
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

# Additional import path used to resolve QML modules in Creator's code model
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
    ui/updateHistograms.cpp

lupdate_only {
SOURCES += qml/filmulator-gui/*.qml \
    qml/filmulator-gui/gui_components/*.qml
}

TRANSLATIONS = translations/filmulator-gui_de.ts

# Installation path
unix:target.path = /usr/lib/filmulator-gui
unix:desktop.path = /usr/share/applications

unix:desktop.files += ./filmulator_gui.desktop

unix {
script.extra = move_script; install -m 755 -p filmulator 
extra.path = /usr/bin
extra.
}

# Please do not modify the following two lines. Required for deployment.
include(qtquick2applicationviewer/qtquick2applicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    qml/filmulator-gui/main.qml \
    qml/filmulator-gui/Edit.qml \
    qml/filmulator-gui/EditTools.qml \
    qml/filmulator-gui/Import.qml \
    qml/filmulator-gui/Organize.qml \
    qml/filmulator-gui/Queue.qml \
    qml/filmulator-gui/Settings.qml \
    qml/filmulator-gui/gui_components/FilmProgressBar.qml \
    qml/filmulator-gui/gui_components/ImportDirEntry.qml \
    qml/filmulator-gui/gui_components/ImportFileEntry.qml \
    qml/filmulator-gui/gui_components/ImportTextEntry.qml \
    qml/filmulator-gui/gui_components/OrganizeDelegate.qml \
    qml/filmulator-gui/gui_components/QueueDelegate.qml \
    qml/filmulator-gui/gui_components/SlipperySlider.qml \
    qml/filmulator-gui/gui_components/ToolButton.qml \
    qml/filmulator-gui/gui_components/ToolButtonStyle.qml \
    qml/filmulator-gui/gui_components/ToolCalendar.qml \
    qml/filmulator-gui/gui_components/ToolRadioButton.qml \
    qml/filmulator-gui/gui_components/ToolRadioButtonStyle.qml \
    qml/filmulator-gui/gui_components/ToolSlider.qml \
    qml/filmulator-gui/gui_components/ToolSwitch.qml \
    qml/filmulator-gui/gui_components/ToolTip.qml \
    qml/filmulator-gui/generateHistogram.js \
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
    ui/thumbWriteWorker.h


QMAKE_CXXFLAGS += -std=c++11 -DTOUT -O3 -fprefetch-loop-arrays -fopenmp -fno-strict-aliasing -ffast-math
#QMAKE_CFLAGS_DEBUG += -DTOUT -O3 -fprefetch-loop-arrays -fopenmp
QMAKE_LFLAGS += -std=c++11 -O3 -fopenmp

LIBS += -lpthread -ltiff -lexiv2 -ljpeg -lraw_r -lgomp

QT += sql core quick qml widgets

INSTALLS += desktop extra

RESOURCES += \
    resources/pixmaps.qrc
