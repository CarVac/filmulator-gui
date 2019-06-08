# - Try to find Exiv2
# Once done this will define
#
#  EXIV2_FOUND - system has Exiv2
#  EXIV2_INCLUDE_DIR - the Exiv2 include directory
#  EXIV2_LIBRARIES - Link these to use Exiv2
#  EXIV2_DEFINITIONS - Compiler switches required for using Exiv2
#
#=============================================================================
#  Copyright (c) 2019 Andres Schneider <asn@cryptomilk.org>
#
#  Distributed under the OSI-approved BSD License (the "License");
#  see accompanying file Copyright.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even the
#  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the License for more information.
#=============================================================================
#

if (UNIX)
  find_package(PkgConfig)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(_EXIV2 exiv2)
  endif (PKG_CONFIG_FOUND)
endif (UNIX)

find_path(EXIV2_INCLUDE_DIR
    NAMES
        exiv2/exif.hpp
    PATHS
        ${_EXIV2_INCLUDEDIR}
)

find_library(EXIV2_LIBRARY
    NAMES
        exiv2
    PATHS
        ${_EXIV2_LIBDIR}
)

if (EXIV2_LIBRARY)
    set(EXIV2_LIBRARIES
        ${EXIV2_LIBRARIES}
        ${EXIV2_LIBRARY}
    )
endif (EXIV2_LIBRARY)

# Get the version number from exiv2/version.hpp and store it in the cache:
if (EXIV2_INCLUDE_DIR AND NOT EXIV2_VERSION)
    set(EXIV2_VERSION_STRING_FOUND FALSE)

    if (EXISTS ${EXIV2_INCLUDE_DIR}/exiv2/version.hpp)
        file(READ ${EXIV2_INCLUDE_DIR}/exiv2/version.hpp EXIV2_VERSION_CONTENT)

        string(FIND "${EXIV2_VERSION_CONTENT}" "#define EXIV2_MAJOR_VERSION" EXIV2_MAJOR_FOUND)
        if (${EXIV2_MAJOR_FOUND} GREATER 0)
            set(EXIV2_VERSION_STRING_FOUND TRUE)
        endif()
    endif()

    if (NOT EXIV2_MAJOR_FOUND AND EXISTS ${EXIV2_INCLUDE_DIR}/exiv2/exv_conf.h)
        file(READ ${EXIV2_INCLUDE_DIR}/exiv2/exv_conf.h EXIV2_VERSION_CONTENT)
    endif()

    string(REGEX MATCH "#define EXIV2_MAJOR_VERSION +\\( *([0-9]+) *\\)"  _dummy "${EXIV2_VERSION_CONTENT}")
    set(EXIV2_VERSION_MAJOR "${CMAKE_MATCH_1}")

    string(REGEX MATCH "#define EXIV2_MINOR_VERSION +\\( *([0-9]+) *\\)"  _dummy "${EXIV2_VERSION_CONTENT}")
    set(EXIV2_VERSION_MINOR "${CMAKE_MATCH_1}")

    string(REGEX MATCH "#define EXIV2_PATCH_VERSION +\\( *([0-9]+) *\\)"  _dummy "${EXIV2_VERSION_CONTENT}")
    set(EXIV2_VERSION_PATCH "${CMAKE_MATCH_1}")

    set(EXIV2_VERSION "${EXIV2_VERSION_MAJOR}.${EXIV2_VERSION_MINOR}.${EXIV2_VERSION_PATCH}")
endif (EXIV2_INCLUDE_DIR AND NOT EXIV2_VERSION)

if (EXIV2_VERSION)
    set(EXIV2_FOUND TRUE)
endif (EXIV2_VERSION)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Exiv2
                                  FOUND_VAR EXIV2_FOUND
                                  REQUIRED_VARS EXIV2_INCLUDE_DIR EXIV2_LIBRARY EXIV2_LIBRARIES
                                  VERSION_VAR EXIV2_VERSION)

# show the EXIV2_INCLUDE_DIR and EXIV2_LIBRARIES variables only in the advanced view
mark_as_advanced(EXIV2_INCLUDE_DIR EXIV2_LIBRARY EXIV2_LIBRARIES)
