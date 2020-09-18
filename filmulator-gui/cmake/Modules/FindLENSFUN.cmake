# - Try to find lensfun
#
# Once found this will define
#  LENSFUN_FOUND - system has lensfun
#  LENSFUN_INCLUDE_DIR - where to find headers
#  LENSFUN_LIBRARIES - what to link to
# Also defined, but not for general use are
#  LENSFUN_LIBRARY


# =======================
# taken from darktable's FindLensFun.cmake
# =======================

include(LibFindMacros)

SET(LENSFUN_FIND_REQUIRED ${LensFun_FIND_REQUIRED})

# use pkg-config to get hints about paths
libfind_pkg_check_modules(Lensfun_PKGCONF lensfun)

find_path(LENSFUN_INCLUDE_DIR NAMES lensfun.h
    HINTS ${Lensfun_PKGCONF_INCLUDE_DIRS}
    /usr/include/lensfun
    /include/lensfun
    ENV LENSFUN_INCLUDE_DIR)
mark_as_advanced(LENSFUN_INCLUDE_DIR)

set(LENSFUN_NAMES ${LENSFUN_NAMES} lensfun liblensfun)
find_library(LENSFUN_LIBRARY NAMES ${LENSFUN_NAMES}
    HINTS ENV LENSFUN_LIB_DIR)
mark_as_advanced(LENSFUN_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set LENSFUN_FOUND to TRUE
#  if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LENSFUN DEFAULT_MSG LENSFUN_LIBRARY LENSFUN_INCLUDE_DIR)

IF(LENSFUN_FOUND)
    SET(LensFun_LIBRARIES ${LENSFUN_LIBRARY})
    SET(LensFun_INCLUDE_DIRS ${LENSFUN_INCLUDE_DIRS})
ENDIF(LENSFUN_FOUND)
