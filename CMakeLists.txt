cmake_minimum_required(VERSION 3.21)

# Custom cmake modules (always needed for DefineCMakeDefaults, CompilerChecks, etc.)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

include(DefineCMakeDefaults)

project(Filmulator VERSION 0.12.0)

include(GNUInstallDirs)
include(CompilerChecks.cmake)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
#set(CMAKE_AUTOUIC ON)
# Tell automoc to not process generated source files
# cmake_policy(SET CMP0071 OLD) //trying out new?
cmake_policy(SET CMP0071 NEW)

find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Quick CONFIG REQUIRED)
find_package(Qt5Sql CONFIG REQUIRED)
find_package(Qt5Qml CONFIG REQUIRED)
find_package(Qt5QuickCompiler CONFIG REQUIRED)
find_package(Qt5QmlImportScanner REQUIRED)
# find_package(Qt5LinguistTools CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)

# find a bunch of packages (vcpkg provides CONFIG mode packages)
find_package(rtprocess CONFIG REQUIRED)
find_package(exiv2 CONFIG REQUIRED)
find_package(libraw CONFIG REQUIRED)
find_package(JPEG REQUIRED)
find_package(TIFF REQUIRED)
find_package(OpenMP REQUIRED)
set(_lensfun_found FALSE)
find_package(lensfun QUIET)
if(lensfun_FOUND)
    set(_lensfun_found TRUE)
endif()
if(NOT _lensfun_found)
    pkg_check_modules(LENSFUN QUIET IMPORTED_TARGET lensfun)
    if(LENSFUN_FOUND)
        set(_lensfun_found TRUE)
    endif()
endif()
if(NOT _lensfun_found)
    message(FATAL_ERROR "lensfun not found (CMake module or pkg-config).")
endif()
if(lensfun_FOUND)
    message(STATUS "Lensfun found via FindLENSFUN.cmake.")
elseif(LENSFUN_FOUND)
    message(STATUS "Lensfun found via pkg-config.")
endif()
find_package(CURL CONFIG REQUIRED)
find_package(LibArchive REQUIRED)
find_package(ZLIB REQUIRED)

# SET( EIGEN3_INCLUDE_DIR "$ENV{EIGEN3_INCLUDE_DIR}" )
# IF( NOT EIGEN3_INCLUDE_DIR )
#     MESSAGE( FATAL_ERROR "Please point the environment variable EIGEN3_INCLUDE_DIR to the include directory of your Eigen3 installation.")
# ENDIF()
# INCLUDE_DIRECTORIES ( "${EIGEN3_INCLUDE_DIR}" )

# Checks for config.h
include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(filmulator_SRCS
    filmulator-gui/main.cpp
    filmulator-gui/core/agitate.cpp
    filmulator-gui/core/colorCurves.cpp
    filmulator-gui/core/colorSpaces.cpp
    filmulator-gui/core/curves.cpp
    filmulator-gui/core/develop.cpp
    filmulator-gui/core/diffuse.cpp
    filmulator-gui/core/exposure.cpp
    filmulator-gui/core/filmulate.cpp
    filmulator-gui/core/imagePipeline.cpp
    filmulator-gui/core/imload.cpp
    filmulator-gui/core/imread.cpp
    filmulator-gui/core/imreadJpeg.cpp
    filmulator-gui/core/imreadTiff.cpp
    filmulator-gui/core/imwriteJpeg.cpp
    filmulator-gui/core/imwriteTiff.cpp
    filmulator-gui/core/layerMix.cpp
    filmulator-gui/core/mergeExps.cpp
    filmulator-gui/core/outputFile.cpp
    filmulator-gui/core/rotateImage.cpp
    filmulator-gui/core/scale.cpp
    filmulator-gui/core/timeDiff.cpp
    filmulator-gui/core/vibranceSaturation.cpp
    filmulator-gui/core/whiteBalance.cpp
    filmulator-gui/core/whitepointBlackpoint.cpp
    filmulator-gui/core/nlmeans/bisecting_kmeans.cpp
    filmulator-gui/core/nlmeans/calcC1chanT.cpp
    filmulator-gui/core/nlmeans/calcW_generic.cpp
    filmulator-gui/core/nlmeans/expandDims.cpp
    filmulator-gui/core/nlmeans/highDimBoxFilter.cpp
    filmulator-gui/core/nlmeans/kMeansNLMApprox.cpp
    filmulator-gui/core/nlmeans/splitCluster.cpp
    filmulator-gui/core/rawtherapee/boxblur.cc
    filmulator-gui/core/rawtherapee/cplx_wavelet_dec.cc
    filmulator-gui/core/rawtherapee/FTblockDN.cc
    filmulator-gui/core/rawtherapee/gauss.cc
    filmulator-gui/core/rawtherapee/impulse_denoise.cc
    filmulator-gui/core/rawtherapee/labimage.cc
    filmulator-gui/database/basicSqlModel.cpp
    filmulator-gui/database/camconst.cpp
    filmulator-gui/database/cJSON.c
    filmulator-gui/database/dateHistogramModel.cpp
    filmulator-gui/database/dbSetup.cpp
    filmulator-gui/database/exifFunctions.cpp
    filmulator-gui/database/importModel.cpp
    filmulator-gui/database/importProperties.cpp
    filmulator-gui/database/importWorker.cpp
    filmulator-gui/database/organizeModel.cpp
    filmulator-gui/database/organizeProperties.cpp
    filmulator-gui/database/queueModel.cpp
    filmulator-gui/database/sqlModel.cpp
    filmulator-gui/database/sqlInsertion.cpp
    filmulator-gui/database/signalSwitchboard.cpp
    filmulator-gui/database/database.cpp
    filmulator-gui/database/rawproc_lensfun/lensfun_dbupdate.cpp
    filmulator-gui/ui/filmImageProvider.cpp
    filmulator-gui/ui/lensSelectModel.cpp
    filmulator-gui/ui/parameterManager.cpp
    filmulator-gui/ui/settings.cpp
    filmulator-gui/ui/thumbWriteWorker.cpp
    filmulator-gui/ui/updateHistograms.cpp
    filmulator-gui/qtquick2applicationviewer/qtquick2applicationviewer.cpp
)

#qt5_add_resources(filmulator_RSCS
qtquick_compiler_add_resources(filmulator_RSCS
    filmulator-gui/qml.qrc
    filmulator-gui/resources/pixmaps.qrc
)

if(APPLE)
    add_executable(filmulator MACOSX_BUNDLE
        ${filmulator_SRCS}
        ${filmulator_RSCS}
    )
elseif(WIN32)
    add_executable(filmulator # WIN32 # comment out WIN32 to get a terminal window
        ${filmulator_SRCS}
        ${filmulator_RSCS}
    )
    target_link_directories(filmulator PRIVATE ${GLIB2_LIBRARY_DIRS})
else()
    add_executable(filmulator
        ${filmulator_SRCS}
        ${filmulator_RSCS}
    )
endif()
target_compile_definitions(filmulator PRIVATE)



target_compile_options(filmulator
    PRIVATE
        ${DEFAULT_CXX_COMPILER_FLAGS}
        -DHAVE_CONFIG_H
        $<$<CXX_COMPILER_ID:MSVC>:/openmp:experimental>
)

set_target_properties(filmulator PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Note: Modern CMake imported targets handle include directories automatically
# when linked via target_link_libraries. Only local directories need explicit listing.
target_include_directories(filmulator
    PRIVATE
        filmulator-gui/core
        filmulator-gui/database
        filmulator-gui/ui
        filmulator-gui/qtquick2applicationviewer
        ${CMAKE_CURRENT_BINARY_DIR}
)
#glib2 needed for static lensfun on windows above and below
#libraw_include_dirs for pointing windows build at it

if(TARGET PkgConfig::LENSFUN)
    set(LENSFUN_TARGET PkgConfig::LENSFUN)
elseif(DEFINED LENSFUN_LIBRARY)
    set(LENSFUN_TARGET ${LENSFUN_LIBRARY})
else()
    message(FATAL_ERROR "lensfun target not found")
endif()

# Use modern CMake imported targets for proper include/link handling
target_link_libraries(filmulator
    PRIVATE
        Qt5::Core
        Qt5::Sql
        Qt5::Widgets
        Qt5::Quick
        Qt5::Qml
        Exiv2::exiv2lib       # exiv2 modern target
        libraw::raw_r         # libraw modern target
        JPEG::JPEG            # CMake built-in JPEG target
        TIFF::TIFF            # TIFF modern target
        ${LENSFUN_TARGET}
        PkgConfig::GLIB
        CURL::libcurl         # CURL modern target
        LibArchive::LibArchive
        ZLIB::ZLIB            # ZLIB modern target
        OpenMP::OpenMP_CXX
        rtprocess::rtprocess
)

    # let's see where all this stuff is...
    #message(STATUS "EXIV2_VERSION=${EXIV2_VERSION}")
    #message(STATUS "CURL_INCLUDE_DIRS=${CURL_INCLUDE_DIRS}")
    #message(STATUS "RTPROCESS_INCLUDE_DIR=${rtprocess_INCLUDE_DIR}")
    #message(STATUS "EXIV2_INCLUDE_DIR=${EXIV2_INCLUDE_DIR}")
    #message(STATUS "LIBRAW_INCLUDE_DIR=${LIBRAW_INCLUDE_DIR}")
    #message(STATUS "JPEG_INCLUDE_DIRS=${JPEG_INCLUDE_DIRS}")
    #message(STATUS "TIFF_INCLUDE_DIR=${TIFF_INCLUDE_DIR}")
    #message(STATUS "LENSFUN_INCLUDE_DIR=${LENSFUN_INCLUDE_DIR}")
    #message(STATUS "LibArchive_INCLUDE_DIRS=${LibArchive_INCLUDE_DIRS}")
    #message(STATUS "LibArchive_LIBRARIES=${LibArchive_LIBRARIES}")
    #message(STATUS "LENSFUN_LIBRARY=${LENSFUN_LIBRARY}")
    #message(STATUS "OpenMP_CXX_LIBRARIES=${OpenMP_CXX_LIBRARIES}")
    #message(STATUS "TIFF_LIBRARIES=${TIFF_LIBRARIES}")
    #message(STATUS "JPEG_LIBRARIES=${JPEG_LIBRARIES}")
    #message(STATUS "LIBRAW_LIBRARIES=${LIBRAW_LIBRARIES}")
    #message(STATUS "EXIV2_LIBRARIES=${EXIV2_LIBRARIES}")
    #message(STATUS "CURL_LIBRARIES=${CURL_LIBRARIES}")
    
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES arm64)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")
    file(COPY "${CMAKE_SOURCE_DIR}/filmulator-gui/resources/macos/filmulator-gui" DESTINATION "./Filmulator.app/Contents/MacOS" FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE WORLD_EXECUTE)
    file(COPY "${CMAKE_SOURCE_DIR}/LICENSE" DESTINATION "./Filmulator.app/Contents/Resources")
    file(COPY "${CMAKE_SOURCE_DIR}/filmulator-gui/resources/macos/icon.icns" DESTINATION "./Filmulator.app/Contents/Resources")
    file(COPY "${CMAKE_SOURCE_DIR}/filmulator-gui/resources/macos/qt.conf" DESTINATION "./Filmulator.app/Contents/Resources")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/build")
    file(COPY "${CMAKE_SOURCE_DIR}/filmulator-gui/resources/macos/Info.plist.in" DESTINATION ".")
    set_source_files_properties(${CMAKE_SOURCE_DIR}/filmulator-gui/resources/macos/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Filmulator.app/Contents/Resources")
    # Copy QML directory to Resources - REMOVED, using qt5_import_qml_plugins instead
    install(TARGETS filmulator BUNDLE DESTINATION "${CMAKE_SOURCE_DIR}/build")
    set_target_properties(filmulator PROPERTIES MACOSX_BUNDLE TRUE MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/filmulator-gui/resources/macos/Info.plist.in")
else()
    install(TARGETS filmulator
        RUNTIME
            DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

endif()

if(UNIX)
    install(FILES "${CMAKE_SOURCE_DIR}/filmulator-gui/resources/filmulator64icon.svg" DESTINATION share/icons/hicolor/scalable/apps)
    install(FILES "${CMAKE_SOURCE_DIR}/filmulator-gui/resources/linux/filmulator.desktop" DESTINATION share/applications)
    install(FILES "${CMAKE_SOURCE_DIR}/filmulator-gui/resources/linux/org.filmulator.Filmulator.metainfo.xml" DESTINATION share/metainfo)
endif()

qt5_import_qml_plugins(filmulator)

add_subdirectory(filmulator-gui/qml)

# Code formatting and linting
find_program(UV_EXE uv)
if(UV_EXE)
    set(CLANG_FORMAT "uv" "run" "clang-format")
    set(CLANG_TIDY "uv" "run" "clang-tidy")
    message(STATUS "Using clang tools via uv (version 18.*)")
else()
    find_program(CLANG_FORMAT clang-format)
    find_program(CLANG_TIDY clang-tidy)
endif()

if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_S_FILES 
        "filmulator-gui/*.cpp" 
        "filmulator-gui/*.hpp" 
        "filmulator-gui/*.h"
        "tests/*.cpp"
        "tests/*.hpp"
        "tests/*.h"
    )
    # Exclude third-party directories
    list(FILTER ALL_S_FILES EXCLUDE REGEX "filmulator-gui/database/cJSON.c")
    list(FILTER ALL_S_FILES EXCLUDE REGEX "filmulator-gui/core/nlmeans/eigen/.*")
    list(FILTER ALL_S_FILES EXCLUDE REGEX "filmulator-gui/Halide/.*")
    list(FILTER ALL_S_FILES EXCLUDE REGEX "filmulator-gui/core/rawtherapee/.*")
    list(FILTER ALL_S_FILES EXCLUDE REGEX "filmulator-gui/qtquick2applicationviewer/.*")
    # Exclude filmulator.cpp
    list(FILTER ALL_S_FILES EXCLUDE REGEX "filmulator-gui/core/filmulator.*")
    
    add_custom_target(
        format
        COMMAND ${CLANG_FORMAT} -i -style=file ${ALL_S_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-format on all source files..."
    )
endif()

if(CLANG_TIDY)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
    add_custom_target(
        tidy
        COMMAND ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR} ${filmulator_CORE_SRCS} filmulator-gui/main.cpp
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy on core source files..."
    )
endif()
