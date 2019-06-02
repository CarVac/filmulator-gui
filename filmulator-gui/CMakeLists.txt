cmake_minimum_required(VERSION 3.5)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

include(DefineCMakeDefaults)

project(Filmulator VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CompilerChecks.cmake)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
#set(CMAKE_AUTOUIC ON)

find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Quick CONFIG REQUIRED)
find_package(Qt5Sql CONFIG REQUIRED)
find_package(Qt5Qml CONFIG REQUIRED)
# find_package(Qt5LinguistTools CONFIG REQUIRED)

find_package(rtprocess CONFIG REQUIRED)
find_package(Exiv2 0.25 REQUIRED)
find_package(libraw REQUIRED)
find_package(JPEG REQUIRED)
find_package(TIFF REQUIRED)
find_package(OpenMP REQUIRED)


# Checks for config.h
include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)


set(filmulator_SRCS
    main.cpp
    core/agitate.cpp
    core/colorCurves.cpp
    core/colorSpaces.cpp
    core/curves.cpp
    core/develop.cpp
    core/diffuse.cpp
    core/exposure.cpp
    core/filmulate.cpp
    core/imagePipeline.cpp
    core/imload.cpp
    core/imread.cpp
    core/imreadJpeg.cpp
    core/imreadTiff.cpp
    core/imwriteJpeg.cpp
    core/imwriteTiff.cpp
    core/layerMix.cpp
    core/mergeExps.cpp
    core/outputFile.cpp
    core/rotateImage.cpp
    core/scale.cpp
    core/timeDiff.cpp
    core/vibranceSaturation.cpp
    core/whiteBalance.cpp
    core/whitepointBlackpoint.cpp
    database/basicSqlModel.cpp
    database/dateHistogramModel.cpp
    database/dbSetup.cpp
    database/exifFunctions.cpp
    database/importModel.cpp
    database/importProperties.cpp
    database/importWorker.cpp
    database/organizeModel.cpp
    database/organizeProperties.cpp
    database/queueModel.cpp
    database/sqlModel.cpp
    database/sqlInsertion.cpp
    database/signalSwitchboard.cpp
    database/database.cpp
    ui/filmImageProvider.cpp
    ui/parameterManager.cpp
    ui/settings.cpp
    ui/thumbWriteWorker.cpp
    ui/updateHistograms.cpp
    qtquick2applicationviewer/qtquick2applicationviewer.cpp
)

qt5_add_resources(filmulator_RSCS
    resources/pixmaps.qrc
)

add_executable(filmulator
    ${filmulator_SRCS}
    ${filmulator_RSCS}
)

target_compile_options(filmulator
    PRIVATE
        ${OpenMP_CXX_FLAGS}
        ${DEFAULT_CXX_COMPILER_FLAGS}
        -DHAVE_CONFIG_H
)

target_include_directories(filmulator
    PRIVATE
        core
        database
        ui
        qtquick2applicationviewer
        ${EXIV2_INCLUDE_DIR}
        ${LIBRAW_INCLUDE_DIR}
        ${JPEG_INCLUDE_DIRS}
        ${TIFF_INCLUDE_DIR}
        ${RTPROCESS_INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(filmulator
    Qt5::Core
    Qt5::Sql
    Qt5::Widgets
    Qt5::Quick
    Qt5::Qml
    ${EXIV2_LIBRARIES}
    ${LIBRAW_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${TIFF_LIBRARIES}
    ${OpenMP_CXX_LIBRARIES}
    ${RTPROCESS_LIBRARIES}
)

install(TARGETS filmulator
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_subdirectory(qml)

