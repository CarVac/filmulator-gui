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
    core/nlmeans/bisecting_kmeans.cpp \
    core/nlmeans/calcC1chanT.cpp \
    core/nlmeans/calcW_generic.cpp \
    core/nlmeans/expandDims.cpp \
    core/nlmeans/highDimBoxFilter.cpp \
    core/nlmeans/kMeansNLMApprox.cpp \
    core/nlmeans/splitCluster.cpp \
    core/outputFile.cpp \
    core/rawtherapee/FTblockDN.cc \
    core/rawtherapee/boxblur.cc \
    core/rawtherapee/cplx_wavelet_dec.cc \
    core/rawtherapee/gauss.cc \
    core/rawtherapee/impulse_denoise.cc \
    core/rawtherapee/labimage.cc \
    core/rotateImage.cpp \
    core/scale.cpp \
    core/timeDiff.cpp \
    core/vibranceSaturation.cpp \
    core/whiteBalance.cpp \
    core/whitepointBlackpoint.cpp \
    database/basicSqlModel.cpp \
    database/cJSON.c \
    database/camconst.cpp \
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
    database/rawproc_lensfun/lensfun_dbupdate.cpp \
    ui/filmImageProvider.cpp \
    ui/lensSelectModel.cpp \
    ui/parameterManager.cpp \
    ui/settings.cpp \
    ui/thumbWriteWorker.cpp \
    ui/updateHistograms.cpp \
    database/database.cpp

lupdate_only {
SOURCES += qml/filmulator-gui/*.qml \
    qml/filmulator-gui/gui_components/*.qml
}

# Installation path
unix:target.path = /usr/lib/filmulator-gui
unix:desktop.path = /usr/share/applications

unix:desktop.files += ./filmulator_gui.desktop

# win32 {
# target.path = ???
# desktop.path = ???
# }
win32:INCLUDEPATH += /usr/include
win32:LIBS += -L/usr/lib

unix {
script.extra = move_script; install -m 755 -p filmulator
extra.path = /usr/bin
INCLUDEPATH += ../eigen
LIBS += -L/usr/local/lib
}

# Please do not modify the following two lines. Required for deployment.
include(qtquick2applicationviewer/qtquick2applicationviewer.pri)
qtcAddDeployment()

#OTHER_FILES += \
#    qml/filmulator-gui/colors.js\
#    qml/filmulator-gui/generateHistogram.js \
#    qml/filmulator-gui/getRoot.js\
#    filmulator

HEADERS += \
    core/filmSim.hpp \
    core/imagePipeline.h \
    core/interface.h \
    core/lut.hpp \
    core/matrix.hpp \
    core/nlmeans/nlmeans.hpp \
    core/rawtherapee/boxblur.h \
    core/rawtherapee/cplx_wavelet_dec.h \
    core/rawtherapee/cplx_wavelet_filter_coeffs.h \
    core/rawtherapee/cplx_wavelet_level.h \
    core/rawtherapee/gauss.h \
    core/rawtherapee/helpersse2.h \
    core/rawtherapee/labimage.h \
    core/rawtherapee/opthelper.h \
    core/rawtherapee/rt_math.h \
    core/rawtherapee/rt_routines.h \
    core/rawtherapee/sleef.h \
    core/rawtherapee/sleefsseavx.h \
    database/backgroundQueue.h \
    database/basicSqlModel.h \
    database/cJSON.h \
    database/camconst.h \
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
    database/rawproc_lensfun/lensfun_dbupdate.h \
    ui/filmImageProvider.h \
    ui/lensSelectModel.h \
    ui/parameterManager.h \
    ui/settings.h \
    ui/thumbWriteWorker.h \
    database/database.hpp


QMAKE_CXXFLAGS += -std=c++17 -DTOUT -O3 -fopenmp -fprefetch-loop-arrays -fno-strict-aliasing -ffast-math -DLF_GIT
macx: {
QMAKE_CXXFLAGS += -Xpreprocessor -lomp -I/opt/local/include
}
unix:!macx {
}

#QMAKE_CFLAGS_DEBUG += -DTOUT -O3 -fprefetch-loop-arrays -fopenmp
QMAKE_LFLAGS += -std=c++17 -O3
macx: {
QMAKE_LFLAGS += -lomp
}
unix:!macx {
QMAKE_LFLAGS += -fopenmp
}

LIBS += -ltiff -lexiv2 -ljpeg -lraw_r -lrtprocess -llensfun -lcurl -larchive
macx: {
LIBS += -L /opt/local/lib /opt/local/lib/libomp.dylib
}

QT += sql core quick qml widgets

CONFIG += qtquickcompiler

INSTALLS += desktop extra

RESOURCES += \
    qml.qrc \
    resources/pixmaps.qrc
    
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
