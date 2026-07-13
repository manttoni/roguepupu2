cmake_minimum_required(VERSION 3.25)
project(roguepupu2 LANGUAGES CXX)

# ========================
# C++ Standard
# ========================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========================
# Options
# ========================
option(ENABLE_TESTS "Build unit tests" ON)
option(ENABLE_ASAN "Enable AddressSanitizer" ON)

# Disable tests automatically in Release unless explicitly enabled
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(ENABLE_TESTS OFF CACHE BOOL "Build unit tests" FORCE)
endif()

# ========================
# Compiler warnings
# ========================
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra)
endif()

# ========================
# Sanitizers
# ========================
if(ENABLE_ASAN AND NOT MSVC)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

# ========================
# Includes
# ========================
include_directories(headers)

# ========================
# Sources
# ========================
file(GLOB_RECURSE SRCS "sources/*.cpp")
list(FILTER SRCS EXCLUDE REGEX ".*/main\\.cpp$")

# ========================
# Core library
# ========================
add_library(roguepupu2_lib STATIC ${SRCS})
target_include_directories(roguepupu2_lib PUBLIC headers)
target_compile_features(roguepupu2_lib PUBLIC cxx_std_20)

# ========================
# Game executable
# ========================
add_executable(${PROJECT_NAME} sources/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE roguepupu2_lib ncursesw panelw m)

# Ensure logs directory exists
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/logs
)

# ========================
# Tests (ONLY if enabled)
# ========================
if(ENABLE_TESTS)

    find_package(GTest REQUIRED)
    enable_testing()

    file(GLOB_RECURSE TEST_SRCS "tests/*.cpp")

    add_executable(${PROJECT_NAME}_tests ${TEST_SRCS})

    target_compile_definitions(${PROJECT_NAME}_tests PRIVATE RUNNING_TESTS)

    target_include_directories(${PROJECT_NAME}_tests PRIVATE
        headers
        tests
        tests/helpers
    )

    target_link_libraries(${PROJECT_NAME}_tests
        PRIVATE roguepupu2_lib
        PRIVATE GTest::gtest
        PRIVATE GTest::gtest_main
        PRIVATE ncursesw panelw m
    )

    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}_tests)

endif()
