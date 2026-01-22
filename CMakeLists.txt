cmake_minimum_required(VERSION 3.16)

project(roguepupu2 LANGUAGES CXX)

enable_testing()  # fixed typo

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -g -O3")

# Include headers
include_directories(${CMAKE_SOURCE_DIR}/headers)

# Recursive source files
file(GLOB_RECURSE SRCS ${CMAKE_SOURCE_DIR}/sources/*.cpp)

# Add main executable
add_executable(${PROJECT_NAME} ${SRCS})

# Find ncurses (wide)
find_package(Curses REQUIRED)
if(CURSES_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CURSES_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE panelw ncursesw m)
endif()

# Ensure logs directory exists after build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/logs
)

# Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Add test executable
#add_executable(sanity_test tests/sanity_test.cpp)
#target_link_libraries(sanity_test PRIVATE gtest gtest_main pthread)

# Register test
#add_test(NAME SanityTest COMMAND sanity_test)

# Find all test source files
file(GLOB TEST_SRCS ${CMAKE_SOURCE_DIR}/tests/*_test.cpp)

foreach(test_src ${TEST_SRCS})
    # Get filename without directory and extension
    get_filename_component(test_name ${test_src} NAME_WE)

    # Add executable
    add_executable(${test_name} ${test_src})

    # Link with GTest and pthread
    target_link_libraries(${test_name} PRIVATE gtest gtest_main pthread)

    # Register test with CTest
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
