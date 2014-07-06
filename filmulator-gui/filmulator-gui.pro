# Add more folders to ship with the application, here
folder_01.source = qml/filmulator-gui
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp \
    core/agitate.cpp \
    core/curves.cpp \
    core/develop.cpp \
    core/diffuse.cpp \
    core/exposure.cpp \
    core/imload.cpp \
    core/imread.cpp \
    core/filmulate.cpp \
    ui/updateHistograms.cpp \
    ui/settings.cpp \
    core/imagePipeline.cpp \
    core/timeDiff.cpp \
    ui/filmImageProvider.cpp \
    database/dbSetup.cpp \
    database/exifFunctions.cpp \
    database/organizeModel.cpp \
    database/organizeProperties.cpp \
    database/sqlModel.cpp \
    core/colorCurves.cpp \
    core/imreadJpeg.cpp \
    core/imreadTiff.cpp \
    core/imwriteTiff.cpp \
    core/imwriteJpeg.cpp \
    core/layerMix.cpp \
    core/mergeExps.cpp \
    core/outputFile.cpp \
    core/rotateImage.cpp \
    core/vibranceSaturation.cpp \
    core/whiteBalance.cpp \
    core/whitepointBlackpoint.cpp \
    database/sqlInsertion.cpp \
    database/importModel.cpp \
    database/importProperties.cpp \
    database/importWorker.cpp \
    database/queueModel.cpp \
    ui/parameterManager.cpp \
    core/scale.cpp

# Installation path
# target.path =

# Please do not modify the following two lines. Required for deployment.
include(qtquick2applicationviewer/qtquick2applicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    Organize.qml \
    Edit.qml \
    EditTools.qml \
    gui_components/SSlider.qml \
    qml/filmulator-gui/gui_components/ToolSlider.qml \
    qml/filmulator-gui/generateHistogram.js

HEADERS += \
    core/lut.hpp \
    core/matrix.hpp \
    core/interface.h \
    ui/settings.h \
    core/imagePipeline.h \
    ui/filmImageProvider.h \
    database/exifFunctions.h \
    database/organizeModel.h \
    database/filmulatorDB.h \
    database/sqlModel.h \
    core/filmSim.hpp \
    database/sqlInsertion.h \
    database/importModel.h \
    database/importWorker.h \
    database/queueModel.h \
    ui/parameterManager.h


QMAKE_CXXFLAGS += -std=c++11 -DTOUT -O3 -fprefetch-loop-arrays -fopenmp -flto
#QMAKE_CFLAGS_DEBUG += -DTOUT -O3 -fprefetch-loop-arrays -fopenmp
QMAKE_LFLAGS += -std=c++11 -O3 -fopenmp -flto

LIBS += -lpthread -ltiff -lexiv2 -ljpeg -lraw -lgomp

QT += sql core quick

RESOURCES +=
