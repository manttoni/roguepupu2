cmake_minimum_required(VERSION 3.25)
project(roguepupu2 LANGUAGES CXX)
file(MAKE_DIRECTORY logs)

# ========================
# C++ Standard
# ========================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========================
# Compiler warnings and optimization
# ========================
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -g -O3)
endif()

# ========================
# Include directories
# ========================
include_directories(headers)

# ========================
# Source files
# ========================
file(GLOB_RECURSE SRCS "sources/*.cpp")

# Separate main.cpp from the rest
list(FILTER SRCS EXCLUDE REGEX ".*/main\\.cpp$")  # Exclude main.cpp from library

# ========================
# Core game library
# ========================
add_library(roguepupu2_lib STATIC ${SRCS})
target_include_directories(roguepupu2_lib PUBLIC headers)
target_compile_features(roguepupu2_lib PUBLIC cxx_std_20)

# ========================
# Game executable
# ========================
add_executable(${PROJECT_NAME} sources/main.cpp)  # Only main.cpp goes here
target_link_libraries(${PROJECT_NAME} PRIVATE roguepupu2_lib ncursesw panelw m)

# Ensure logs directory exists after build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/logs
)

# ========================
# Google Test
# ========================
find_package(GTest REQUIRED)
enable_testing()

file(GLOB_RECURSE TEST_SRCS "tests/*.cpp")
add_executable(${PROJECT_NAME}_tests ${TEST_SRCS})

# Add include directories for tests
target_include_directories(${PROJECT_NAME}_tests PRIVATE headers tests tests/helpers)

target_link_libraries(${PROJECT_NAME}_tests
    PRIVATE roguepupu2_lib
    PRIVATE GTest::gtest
    PRIVATE GTest::gtest_main
    PRIVATE ncursesw panelw m       # <-- add these
)


include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_tests)

