cmake_minimum_required(VERSION 3.25)

project(
    roguepupu2
    VERSION 0.1.0
    DESCRIPTION "A data-driven C++ roguelike"
    LANGUAGES CXX
)

# Generate compile_commands.json for clangd and other tooling.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ROGUEPUPU_BUILD_TESTS "Build unit tests" ON)
option(ROGUEPUPU_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ROGUEPUPU_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

# ---------------------------------------------------------------------------
# Project settings
# ---------------------------------------------------------------------------

add_library(roguepupu_project_options INTERFACE)
target_compile_features(roguepupu_project_options INTERFACE cxx_std_20)

add_library(roguepupu_project_warnings INTERFACE)

if(MSVC)
    target_compile_options(roguepupu_project_warnings INTERFACE
        /W4
        $<$<BOOL:${ROGUEPUPU_WARNINGS_AS_ERRORS}>:/WX>
    )
else()
    target_compile_options(roguepupu_project_warnings INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -fdiagnostics-color=always
        -ftemplate-backtrace-limit=2

        $<$<CXX_COMPILER_ID:Clang>:-ferror-limit=1>
        $<$<CXX_COMPILER_ID:GNU>:-fmax-errors=3>

        $<$<BOOL:${ROGUEPUPU_WARNINGS_AS_ERRORS}>:-Werror>
    )
endif()

if(ROGUEPUPU_ENABLE_ASAN)
    if(MSVC)
        message(FATAL_ERROR
            "ROGUEPUPU_ENABLE_ASAN is not configured for MSVC"
        )
    endif()

    target_compile_options(roguepupu_project_options INTERFACE
        -fsanitize=address
        -fno-omit-frame-pointer
    )

    target_link_options(roguepupu_project_options INTERFACE
        -fsanitize=address
    )
endif()
# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_library(NCURSESW_LIBRARY NAMES ncursesw REQUIRED)
find_library(PANELW_LIBRARY NAMES panelw REQUIRED)

set(ROGUEPUPU_PLATFORM_LIBRARIES
    ${NCURSESW_LIBRARY}
    ${PANELW_LIBRARY}
)

if(UNIX AND NOT APPLE)
    list(APPEND ROGUEPUPU_PLATFORM_LIBRARIES m)
endif()

# ---------------------------------------------------------------------------
# Core library
# ---------------------------------------------------------------------------

# CONFIGURE_DEPENDS is convenient for this project while it is being heavily
# rewritten. An explicit source list would be stricter for a larger project.
file(GLOB_RECURSE ROGUEPUPU_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/sources/*.cpp"
)
list(FILTER ROGUEPUPU_CORE_SOURCES EXCLUDE REGEX "/main\\.cpp$")

add_library(roguepupu2_lib STATIC)
target_sources(roguepupu2_lib PRIVATE ${ROGUEPUPU_CORE_SOURCES})

target_include_directories(roguepupu2_lib
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/headers"
)

target_link_libraries(roguepupu2_lib
    PUBLIC
        roguepupu_project_options
    PRIVATE
        roguepupu_project_warnings
        ${ROGUEPUPU_PLATFORM_LIBRARIES}
)

# ---------------------------------------------------------------------------
# Game executable
# ---------------------------------------------------------------------------

add_executable(roguepupu2 sources/main.cpp)

target_link_libraries(roguepupu2
    PRIVATE
        roguepupu2_lib
        roguepupu_project_warnings
)

set_target_properties(roguepupu2 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_command(
    TARGET roguepupu2
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_BINARY_DIR}/logs"
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

if(ROGUEPUPU_BUILD_TESTS)
    include(CTest)
    enable_testing()

    find_package(GTest REQUIRED)
    include(GoogleTest)

    file(GLOB_RECURSE ROGUEPUPU_TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    )

    add_executable(roguepupu2_tests)
    target_sources(roguepupu2_tests PRIVATE ${ROGUEPUPU_TEST_SOURCES})

    target_compile_definitions(roguepupu2_tests PRIVATE RUNNING_TESTS)

    target_include_directories(roguepupu2_tests
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/tests"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/helpers"
    )

    target_link_libraries(roguepupu2_tests
        PRIVATE
            roguepupu2_lib
            roguepupu_project_warnings
            GTest::gtest
            GTest::gtest_main
    )

    set_target_properties(roguepupu2_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    gtest_discover_tests(
        roguepupu2_tests
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DISCOVERY_TIMEOUT 30
    )
endif()

