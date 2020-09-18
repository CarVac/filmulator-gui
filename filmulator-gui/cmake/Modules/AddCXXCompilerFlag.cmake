#
# add_c_compiler_flag("-Werror" SUPPORTED_CFLAGS)
#
# Copyright (c) 2019      Andreas Schneider <asn@cryptomilk.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(CheckCXXCompilerFlag)

macro(add_cxx_compiler_flag _COMPILER_FLAG _OUTPUT_VARIABLE)
    string(TOUPPER ${_COMPILER_FLAG} _COMPILER_FLAG_NAME)
    string(REGEX REPLACE "^-" "" _COMPILER_FLAG_NAME "${_COMPILER_FLAG_NAME}")
    string(REGEX REPLACE "(-|=|\ )" "_" _COMPILER_FLAG_NAME "${_COMPILER_FLAG_NAME}")

    check_cxx_compiler_flag("${_COMPILER_FLAG}" WITH_${_COMPILER_FLAG_NAME}_FLAG)
    if (WITH_${_COMPILER_FLAG_NAME}_FLAG)
        #string(APPEND ${_OUTPUT_VARIABLE} "${_COMPILER_FLAG} ")
        list(APPEND ${_OUTPUT_VARIABLE} ${_COMPILER_FLAG})
    endif()
endmacro()
