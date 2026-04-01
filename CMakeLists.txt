cmake_minimum_required(VERSION 3.21)

# Custom cmake modules (always needed for DefineCMakeDefaults, CompilerChecks, etc.)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

include(DefineCMakeDefaults)

project(Filmulator VERSION 0.11.2)

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
find_package(spdlog CONFIG REQUIRED)

# Spix E2E testing support (optional)
option(ENABLE_SPIX_TESTING "Enable Spix-based E2E testing" OFF)

if(NOT DEFINED ENABLE_NAN_TRAPPING)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(ENABLE_NAN_TRAPPING ON)
    else()
        set(ENABLE_NAN_TRAPPING OFF)
    endif()
endif()
option(ENABLE_NAN_TRAPPING "Enable NaN trapping and debug breaks" ${ENABLE_NAN_TRAPPING})

if(ENABLE_SPIX_TESTING)
    # AnyRPC from vcpkg (no CMake config, so set paths manually)
    set(AnyRPC_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include")
    set(AnyRPC_LIBRARIES "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/lib/libanyrpc.a")
    
    include(FetchContent)
    
    # Spix - UI test automation library (not in vcpkg, so fetch it)
    FetchContent_Declare(
        spix
        GIT_REPOSITORY https://github.com/faaxm/spix.git
        GIT_TAG v0.9
    )
    set(SPIX_QT_MAJOR 5 CACHE STRING "" FORCE)
    set(SPIX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(spix)
endif()

# SET( EIGEN3_INCLUDE_DIR "$ENV{EIGEN3_INCLUDE_DIR}" )
# IF( NOT EIGEN3_INCLUDE_DIR )
#     MESSAGE( FATAL_ERROR "Please point the environment variable EIGEN3_INCLUDE_DIR to the include directory of your Eigen3 installation.")
# ENDIF()
# INCLUDE_DIRECTORIES ( "${EIGEN3_INCLUDE_DIR}" )

# Checks for config.h
include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(filmulator_CORE_SRCS
    filmulator-gui/core/agitate.cpp
    filmulator-gui/core/colorCurves.cpp
    filmulator-gui/core/colorSpaces.cpp
    filmulator-gui/core/curves.cpp
    filmulator-gui/core/develop.cpp
    filmulator-gui/core/diffuse.cpp
    filmulator-gui/core/exposure.cpp
    filmulator-gui/core/filmulate.cpp
    filmulator-gui/core/imagePipeline.cpp
    filmulator-gui/core/pipeline/LoadStage.cpp
    filmulator-gui/core/pipeline/DemosaicStage.cpp
    filmulator-gui/core/pipeline/PostDemosaicStage.cpp
    filmulator-gui/core/pipeline/NlmeansNRStage.cpp
    filmulator-gui/core/pipeline/ImpulseNRStage.cpp
    filmulator-gui/core/pipeline/ChromaNRStage.cpp
    filmulator-gui/core/pipeline/PrefilmulationStage.cpp
    filmulator-gui/core/imload.cpp
    filmulator-gui/core/imread.cpp
    filmulator-gui/core/imreadJpeg.cpp
    filmulator-gui/core/imreadTiff.cpp
    filmulator-gui/core/imwriteJpeg.cpp
    filmulator-gui/core/imwriteTiff.cpp
    filmulator-gui/core/layerMix.cpp
    filmulator-gui/core/logging.cpp
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

# Create the core library
add_library(filmulator-lib STATIC
    ${filmulator_CORE_SRCS}
    ${filmulator_RSCS}
)

target_compile_definitions(filmulator-lib
    PUBLIC
        $<$<BOOL:${ENABLE_NAN_TRAPPING}>:ENABLE_NAN_TRAPPING>
)

# Compile options for the library
target_compile_options(filmulator-lib
    PRIVATE
        ${DEFAULT_CXX_COMPILER_FLAGS}
        -DHAVE_CONFIG_H
        $<$<CXX_COMPILER_ID:MSVC>:/openmp:experimental>
)

# Include directories for the library
target_include_directories(filmulator-lib
    PUBLIC
        filmulator-gui/core
        filmulator-gui/database
        filmulator-gui/ui
        filmulator-gui/qtquick2applicationviewer
        ${CMAKE_CURRENT_BINARY_DIR}
)

if(TARGET PkgConfig::LENSFUN)
    set(LENSFUN_TARGET PkgConfig::LENSFUN)
elseif(DEFINED LENSFUN_LIBRARY)
    set(LENSFUN_TARGET ${LENSFUN_LIBRARY})
else()
    message(FATAL_ERROR "lensfun target not found")
endif()

# Link libraries for the library
target_link_libraries(filmulator-lib
    PUBLIC
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
        CURL::libcurl
        LibArchive::LibArchive
        ZLIB::ZLIB
        OpenMP::OpenMP_CXX
        rtprocess::rtprocess
        spdlog::spdlog
)

# Define the executable
if(APPLE)
    add_executable(filmulator MACOSX_BUNDLE
        filmulator-gui/main.cpp
    )

elseif(WIN32)
    add_executable(filmulator
        filmulator-gui/main.cpp
    )
    target_link_directories(filmulator PRIVATE ${GLIB2_LIBRARY_DIRS})
else()
    add_executable(filmulator
        filmulator-gui/main.cpp
    )
endif()
target_compile_definitions(filmulator PRIVATE)
set_target_properties(filmulator PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Link the executable to the library
target_link_libraries(filmulator
    PRIVATE
        filmulator-lib
)

# Conditionally link Spix for E2E testing
if(ENABLE_SPIX_TESTING)
    target_link_libraries(filmulator PRIVATE Spix ${AnyRPC_LIBRARIES})
    target_include_directories(filmulator PRIVATE 
        ${spix_SOURCE_DIR}/lib/include
    )
    target_compile_definitions(filmulator PRIVATE ENABLE_SPIX_TESTING)
endif()

enable_testing()
add_subdirectory(tests)

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

qt5_import_qml_plugins(filmulator)

add_subdirectory(filmulator-gui/qml)

# Code formatting and linting
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_S_FILES 
        "filmulator-gui/*.cpp" 
        "filmulator-gui/*.hpp" 
        "filmulator-gui/*.h"
        "tests/*.cpp"
        "tests/*.hpp"
        "tests/*.h"
    )
    # Exclude third-party cJSON
    list(REMOVE_ITEM ALL_S_FILES "${CMAKE_SOURCE_DIR}/filmulator-gui/database/cJSON.c")
    
    add_custom_target(
        format
        COMMAND ${CLANG_FORMAT} -i -style=file ${ALL_S_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-format on all source files..."
    )
endif()

find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
    add_custom_target(
        tidy
        COMMAND ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR} ${filmulator_CORE_SRCS} filmulator-gui/main.cpp
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy on core source files..."
    )
endif()
