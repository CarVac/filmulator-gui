include(AddCXXCompilerFlag)
include(CheckCXXCompilerFlagSSP)

if (UNIX)
    ##################### CXX ########################

    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    #
    # Check for -Werror turned on if possible
    #
    # This will prevent that compiler flags are detected incorrectly.
    #
    check_cxx_compiler_flag("-Werror" REQUIRED_FLAGS_WERROR)
    if (REQUIRED_FLAGS_WERROR)
        set(CMAKE_REQUIRED_FLAGS "-Werror")

        if (PICKY_DEVELOPER)
            list(APPEND SUPPORTED_CXX_COMPILER_FLAGS "-Werror")
        endif()
    endif()

    add_cxx_compiler_flag("-Wall" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Wshadow" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Wwrite-strings" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Werror=write-strings" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Wpointer-arith" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Werror=pointer-arith" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Wreturn-type" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Werror=return-type" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Wuninitialized" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Werror=uninitialized" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Wimplicit-fallthrough" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Werror=strict-overflow" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Wstrict-overflow=2" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Wno-format-zero-length" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Wmissing-field-initializers" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Wunused-label" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Werror=unused-label" SUPPORTED_CXX_COMPILER_FLAGS)

    check_cxx_compiler_flag("-Wformat" REQUIRED_FLAGS_WFORMAT)
    if (REQUIRED_FLAGS_WFORMAT)
        list(APPEND SUPPORTED_CXX_COMPILER_FLAGS "-Wformat")
        set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -Wformat")
    endif()
    add_cxx_compiler_flag("-Wformat-security" SUPPORTED_CXX_COMPILER_FLAGS)
    add_cxx_compiler_flag("-Werror=format-security" SUPPORTED_CXX_COMPILER_FLAGS)

    check_cxx_compiler_flag_ssp("-fstack-protector-strong" WITH_STACK_PROTECTOR_STRONG)
    if (WITH_STACK_PROTECTOR_STRONG)
        list(APPEND SUPPORTED_CXX_COMPILER_FLAGS "-fstack-protector-strong")
    else (WITH_STACK_PROTECTOR_STRONG)
        check_cxx_compiler_flag_ssp("-fstack-protector" WITH_STACK_PROTECTOR)
        if (WITH_STACK_PROTECTOR)
            list(APPEND SUPPORTED_CXX_COMPILER_FLAGS "-fstack-protector")
            # This is needed as Solaris has a seperate libssp
            if (SOLARIS)
                list(APPEND SUPPORTED_LINKER_FLAGS "-fstack-protector")
            endif()
        endif()
    endif (WITH_STACK_PROTECTOR_STRONG)

    check_cxx_compiler_flag_ssp("-fstack-clash-protection" WITH_STACK_CLASH_PROTECTION)
    if (WITH_STACK_CLASH_PROTECTION)
        list(APPEND SUPPORTED_CXX_COMPILER_FLAGS "-fstack-clash-protection")
    endif()

    if (PICKY_DEVELOPER)
        add_cxx_compiler_flag("-Wno-error=deprecated-declarations" SUPPORTED_CXX_COMPILER_FLAGS)
        add_cxx_compiler_flag("-Wno-error=tautological-compare" SUPPORTED_CXX_COMPILER_FLAGS)
    endif()

    # Unset CMAKE_REQUIRED_FLAGS
    unset(CMAKE_REQUIRED_FLAGS)
endif()

set(DEFAULT_CXX_COMPILE_FLAGS ${SUPPORTED_CXX_COMPILER_FLAGS} CACHE INTERNAL "Default CXX Compiler Flags" FORCE)
set(DEFAULT_LINK_FLAGS ${SUPPORTED_LINKER_FLAGS} CACHE INTERNAL "Default Linker Flags" FORCE)
