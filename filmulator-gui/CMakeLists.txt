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
find_package(exiv2 REQUIRED)
find_package(libraw REQUIRED)
find_package(JPEG REQUIRED)
find_package(TIFF REQUIRED)
find_package(OpenMP REQUIRED)
find_package(lensfun REQUIRED)
find_package(curl REQUIRED)
find_package(archive REQUIRED)

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

if(APPLE)
    add_executable(filmulator MACOSX_BUNDLE
        ${filmulator_SRCS}
        ${filmulator_RSCS}
    )
elseif(WIN32)
    add_executable(filmulator WIN32
        ${filmulator_SRCS}
        ${filmulator_RSCS}
    )
else()
    add_executable(filmulator
        ${filmulator_SRCS}
        ${filmulator_RSCS}
    )
endif()

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
        ${LENSFUN_INCLUDE_DIR}
        ${CURL_INCLUDE_DIR}
        ${ARCHIVE_INCLUDE_DIR}
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
    ${LENSFUN_LIBRARY}
    ${OpenMP_CXX_LIBRARIES}
    rtprocess::rtprocess
)

if(APPLE)
    file(COPY "${CMAKE_SOURCE_DIR}/resources/macos/filmulator-gui" DESTINATION "./Filmulator.app/Contents/MacOS" FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE WORLD_EXECUTE)
    file(COPY "${CMAKE_SOURCE_DIR}/../LICENSE" DESTINATION "./Filmulator.app/Contents/Resources")
    file(COPY "${CMAKE_SOURCE_DIR}/resources/macos/icon.icns" DESTINATION "./Filmulator.app/Contents/Resources")
    file(COPY "${CMAKE_SOURCE_DIR}/resources/macos/qt.conf" DESTINATION "./Filmulator.app/Contents/Resources")
    set(CMAKE_OSX_ARCHITECTURES x86_64)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/build")
    file(COPY "${CMAKE_SOURCE_DIR}/resources/macos/Info.plist.in" DESTINATION ".")
    set_source_files_properties(${CMAKE_SOURCE_DIR}/resources/macos/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Filmulator.app/Contents/Resources")
    set(filmulator ${filmulator} "${CMAKE_SOURCE_DIR}/resources/macos/Icon.icns")
    install(TARGETS filmulator BUNDLE DESTINATION "${CMAKE_SOURCE_DIR}/build")
    execute_process(COMMAND plutil -replace CFBundleGetInfoString -string "${PROJECT_NAME} ${PROJECT_VERSION}" ./Info.plist.in)
    execute_process(COMMAND plutil -replace CFBundleVersion -string "${PROJECT_VERSION}" ./Info.plist.in)
    execute_process(COMMAND plutil -replace LSMinimumSystemVersion -string "${CMAKE_OSX_DEPLOYMENT_TARGET}" ./Info.plist.in)
    set_target_properties(filmulator PROPERTIES MACOSX_BUNDLE TRUE MACOSX_BUNDLE_INFO_PLIST ./Info.plist.in)

    file(READ_SYMLINK "${EXIV2_LIBRARY}" result)
    file(COPY "${EXIV2_INCLUDE_DIR}/../lib/${result}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")
    file(READ_SYMLINK "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${result}" result2)
    file(COPY "${EXIV2_INCLUDE_DIR}/../lib/${result2}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")
    file(RENAME "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${result2}" "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${result}")

    #NEEDS LENSFUN, CURL, and ARCHIVE ADDED TO MAC BUILD

    if(IS_SYMLINK "${JPEG_LIBRARY_RELEASE}")
        file(READ_SYMLINK "${JPEG_LIBRARY_RELEASE}" result)
        file(COPY "${JPEG_INCLUDE_DIR}/../lib/${result}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")
        if(IS_SYMLINK "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${result}")
            file(READ_SYMLINK "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${result}" result2)
            file(COPY "${JPEG_INCLUDE_DIR}/../lib/${result2}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")
            file(RENAME "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${result2}" "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${result}")
        endif()
    endif()

    if(IS_SYMLINK "${TIFF_LIBRARY_RELEASE}")
        file(READ_SYMLINK "${TIFF_LIBRARY_RELEASE}" result)
        file(COPY "${TIFF_INCLUDE_DIR}/../lib/${result}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")
        if(IS_SYMLINK "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${result}")
            file(READ_SYMLINK "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${result}" result2)
            file(COPY "${TIFF_INCLUDE_DIR}/../lib/${result2}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")
            file(RENAME "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${result2}" "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${result}")
        endif()
    endif()

    file(COPY "${RTPROCESS_LIBRARIES}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")
    file(COPY "${OpenMP_libomp_LIBRARY}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")

else()
    install(TARGETS filmulator
        RUNTIME
            DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

endif()

add_subdirectory(qml)
