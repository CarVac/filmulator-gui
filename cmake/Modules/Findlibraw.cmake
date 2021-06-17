# - Try to find libraw
# Once done this will define
#
#  LIBRAW_FOUND - system has libraw
#  LIBRAW_INCLUDE_DIR - the libraw include directory
#  LIBRAW_LIBRARIES - Link these to use libraw
#  LIBRAW_DEFINITIONS - Compiler switches required for using libraw
#
#=============================================================================
#  Copyright (c) 2019 Carlo Vaccari <c.lo.to.da.down.lo@gmail.com>
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
    pkg_check_modules(_LIBRAW libraw)
  endif (PKG_CONFIG_FOUND)
endif (UNIX)

find_path(LIBRAW_INCLUDE_DIR
    NAMES
        libraw/libraw.h
    PATHS
        ${_LIBRAW_INCLUDEDIR}
)

find_library(RAW_R_LIBRARY
    NAMES
        raw_r
    PATHS
        ${_LIBRAW_LIBDIR}
)

if (RAW_R_LIBRARY)
    set(LIBRAW_LIBRARIES
        ${LIBRAW_LIBRARIES}
        ${RAW_R_LIBRARY}
    )
endif (RAW_R_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libraw DEFAULT_MSG LIBRAW_LIBRARIES LIBRAW_INCLUDE_DIR)

# show the LIBRAW_INCLUDE_DIR and LIBRAW_LIBRARIES variables only in the advanced view
mark_as_advanced(LIBRAW_INCLUDE_DIR LIBRAW_LIBRARIES)

