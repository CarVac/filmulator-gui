cmake_minimum_required(VERSION 3.5)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

include(DefineCMakeDefaults)

project(Filmulator VERSION 0.9.0 LANGUAGES CXX)

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

# find a bunch of packages
find_package(rtprocess CONFIG REQUIRED)
find_package(Exiv2 REQUIRED)
find_package(libraw REQUIRED)
find_package(JPEG REQUIRED)
find_package(TIFF REQUIRED)
find_package(OpenMP REQUIRED)
find_package(LENSFUN REQUIRED)
find_package(CURL REQUIRED)
find_package(LibArchive REQUIRED)


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
    database/rawproc_lensfun/lensfun_dbupdate.cpp
    ui/filmImageProvider.cpp
    ui/lensSelectModel.cpp
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
        ${CURL_INCLUDE_DIRS}
        ${LibArchive_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}
)
    message(STATUS "EXIV2_VERSION=${EXIV2_VERSION}")

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
    ${CURL_LIBRARIES}
    ${LibArchive_LIBRARIES}
    ${OpenMP_CXX_LIBRARIES}
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

  
    if (IS_SYMLINK "${EXIV2_LIBRARY}")
        file(READ_SYMLINK "${EXIV2_LIBRARY}" result)
	get_filename_component(exiv2_dir "${EXIV2_LIBRARY}" DIRECTORY)
        message(STATUS "exiv2_dir=${exiv2_dir}")
        message(STATUS "result=${result}")

        if (IS_SYMLINK "${exiv2_dir}/${result}") # this is the most likely situation,
						 # assuming its from homebrew
            file(READ_SYMLINK "${exiv2_dir}/${result}" result2)
	    get_filename_component(exiv2_dir2 "${result2}" DIRECTORY)
	    message(STATUS "exiv2_dir2=${exiv2_dir2}")
	    message(STATUS "result2=${result2}")

	    # at this point we need a dirty, greasy hack... read_symlink appears
	    # to return the wrong thing. so because we fixed the FindExiv2.cmake
	    # file to actually return a real version number, we can make it ourselves...

	    get_filename_component(exiv2_dir2 "${exiv2_dir}/${result}" DIRECTORY)
	    message(STATUS "exiv2_dir2/libexiv2.EXIV2_VERSION.dylib=${exiv2_dir2}/libexiv2.${EXIV2_VERSION}.dylib")
	    file(COPY "${exiv2_dir2}/libexiv2.${EXIV2_VERSION}.dylib" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/")
	    file(RENAME "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/libexiv2.${EXIV2_VERSION}.dylib" "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/libexiv2.dylib")

	else()
            file(COPY "${exiv2_dir}/${result}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/")
        endif()

    else()
	# yeah right its always gonna be a symlink, but we'll put this here just in case...
	file(COPY "${EXIV2_LIBRARY}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/")
    endif()

    #NEEDS LENSFUN, CURL, and ARCHIVE ADDED TO MAC BUILD
    
    #LibArchive is a symlink
    file(READ_SYMLINK "${LibArchive_INCLUDE_DIR}/../lib/libarchive.dylib" libarchiveresult)
    message(STATUS "libarchiveresult=${libarchiveresult}")
    file(COPY "${LibArchive_INCLUDE_DIR}/../lib/${libarchiveresult}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")
    file(RENAME "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${libarchiveresult}" "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/libarchive.dylib")

    #LIBCURL is a symlink
    file(READ_SYMLINK "${CURL_LIBRARIES}" libcurlresult)
    file(COPY "${CURL_INCLUDE_DIR}/../lib/${libcurlresult}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")
    file(RENAME "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${libcurlresult}" "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/libcurl.dylib")

    #LENSFUN
    #message(STATUS "LENSFUN_LIBRARY=${LENSFUN_LIBRARY}")
    file(READ_SYMLINK "${LENSFUN_LIBRARY}" liblensfunresult)
    get_filename_component(liblensfun_dir "${LENSFUN_LIBRARY}" DIRECTORY)
    #message(STATUS "liblensfunresult=${liblensfunresult}")
    #message(STATUS "liblensfun_dir=${liblensfun_dir}")
    file(READ_SYMLINK "${liblensfun_dir}/${liblensfunresult}" liblensfunresult2)
    #message(STATUS "liblensfunresult=${liblensfunresult2}")
    file(COPY "${liblensfun_dir}/${liblensfunresult2}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")
    file(RENAME "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${liblensfunresult2}" "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/liblensfun.dylib")
    

    if(IS_SYMLINK "${JPEG_LIBRARY_RELEASE}")
        file(READ_SYMLINK "${JPEG_LIBRARY_RELEASE}" result)
	get_filename_component(libjpeg_dir "${JPEG_LIBRARY_RELEASE}" DIRECTORY)
	message(STATUS "JPEG_LIBRARY_RELEASE=${JPEG_LIBRARY_RELEASE}")
	message(STATUS "result=${result}")
	#file(COPY "${libjpeg_dir}/${result}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")
	#if(IS_SYMLINK "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${result}")
        if(IS_SYMLINK "${libjpeg_dir}/${result}")
            file(READ_SYMLINK "${libjpeg_dir}/${result}" result2)
	    message(STATUS "result=${result2}")
	    get_filename_component(libjpeg_dir2 "${libjpeg_dir}/${result}" DIRECTORY)
	    message(STATUS "libjpeg_dir2=${libjpeg_dir2}")
            file(COPY "${libjpeg_dir2}/${result2}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")
            file(RENAME "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${result2}" "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/libjpeg.dylib")
        endif()
    endif()

    if(IS_SYMLINK "${TIFF_LIBRARY_RELEASE}")
        file(READ_SYMLINK "${TIFF_LIBRARY_RELEASE}" result)
	message(STATUS "result=${result}")
	get_filename_component(libtiff_dir "${TIFF_LIBRARY_RELEASE}" DIRECTORY)
	#file(COPY "${TIFF_INCLUDE_DIR}/../lib/${result}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")
        if(IS_SYMLINK "${libtiff_dir}/${result}")
	    file(READ_SYMLINK "${libtiff_dir}/${result}" result2)
	    get_filename_component(libtiff_dir2 "${libtiff_dir}/${result}" DIRECTORY)
            file(COPY "${libtiff_dir2}/${result2}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")
            file(RENAME "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${result2}" "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/libtiff.dylib")
        endif()
    endif()

    file(COPY "${librtprocess_dylib}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")
    get_filename_component(librtprocess_name "${librtprocess_dylib}" NAME)
    #message(STATUS "librtprocess_name=${librtprocess_name}")
    file(RENAME "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/${librtprocess_name}" "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks/librtprocess.0.dylib")
    file(COPY "${OpenMP_libomp_LIBRARY}" DESTINATION "${CMAKE_SOURCE_DIR}/build/Filmulator.app/Contents/Frameworks")

else()
    install(TARGETS filmulator
        RUNTIME
            DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

endif()

add_subdirectory(qml)
