# Add more folders to ship with the application, here
folder_01.source = qml/Filmulator-Testing-2
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
    core/imread_jpeg.cpp \
    core/imread_tiff.cpp \
    core/imwrite_jpeg.cpp \
    core/imwrite_tiff.cpp \
    core/initialize.cpp \
    core/layer_mix.cpp \
    core/merge_exps.cpp \
    core/output_file.cpp \
    core/time_diff.cpp \
    core/filmulate.cpp \
    core/whitepoint_blackpoint.cpp \
    core/color_curves.cpp \
    ui/filmimageprovider.cpp \
    ui/updateHistograms.cpp \
    database/sqlmodel.cpp \
    database/dbsetup.cpp \
    database/organizeproperties.cpp \
    core/rotate_image.cpp \
    core/white_balance.cpp \
    core/vibrance_saturation.cpp \
    database/organizemodel.cpp \
    database/organizeinsertion.cpp \
    database/exiffunctions.cpp

# Installation path
# target.path =

# Please do not modify the following two lines. Required for deployment.
include(qtquick2applicationviewer/qtquick2applicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    Organize.qml \
    gui_components/SSlider.qml \
    qml/Filmulator-Testing-2/gui_components/ToolSlider.qml \
    qml/Filmulator-Testing-2/generateHistogram.js

HEADERS += \
    core/filmsim.hpp \
    core/lut.hpp \
    core/matrix.hpp \
    database/sqlmodel.h \
    core/interface.h \
    ui/filmimageprovider.h \
    database/filmulatordb.h \
    database/organizemodel.h \
    database/exiffunctions.h


QMAKE_CXXFLAGS += -std=c++11 -DTOUT -O3 -fprefetch-loop-arrays -fopenmp -flto
#QMAKE_CFLAGS_DEBUG += -DTOUT -O3 -fprefetch-loop-arrays -fopenmp
QMAKE_LFLAGS += -std=c++11 -O3 -fopenmp -flto

LIBS += -lpthread -ltiff -lexiv2 -ljpeg -lraw -lgomp

QT += sql core quick

RESOURCES +=
