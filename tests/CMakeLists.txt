enable_testing()

find_package(Catch2 CONFIG REQUIRED)

# Collect all unit test source files
set(UNIT_TEST_SOURCES
    main.cpp
    unit/test_curves.cpp
    unit/test_white_balance.cpp
    unit/test_pipeline_infrastructure.cpp
    unit/test_nan_break.cpp
)

# Collect golden/acceptance test source files
set(GOLDEN_TEST_SOURCES
    main.cpp
    golden/test_curves_golden.cpp
    golden/test_white_balance_golden.cpp
    golden/test_color_adjustments_golden.cpp
    golden/test_filmulate_golden.cpp
    golden/test_nlmeans_golden.cpp
)

# Unit tests executable
add_executable(unit_tests
    ${UNIT_TEST_SOURCES}
)

target_link_libraries(unit_tests
    PRIVATE
    Catch2::Catch2
    Catch2::Catch2WithMain
    filmulator-lib
)

# Golden tests executable
add_executable(golden_tests
    ${GOLDEN_TEST_SOURCES}
)

target_include_directories(golden_tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/filmulator-gui
)

target_compile_definitions(golden_tests
    PRIVATE
    GOLDEN_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/golden/data/"
)

target_link_libraries(golden_tests
    PRIVATE
    Catch2::Catch2
    Catch2::Catch2WithMain
    filmulator-lib
)

# Discover tests for CTest integration
include(Catch)
catch_discover_tests(unit_tests)
catch_discover_tests(golden_tests)

if(ENABLE_SPIX_TESTING)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_test(
        NAME e2e_raw_pipeline
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/e2e/test_raw_pipeline.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()
