cmake_minimum_required(VERSION 3.16)

project(roguepupu2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -g -O3")

# Include headers
include_directories(${CMAKE_SOURCE_DIR}/headers)

# Recursive source files
file(GLOB_RECURSE SRCS ${CMAKE_SOURCE_DIR}/sources/*.cpp)

# Add executable
add_executable(${PROJECT_NAME} ${SRCS})

# Find ncurses (wide)
find_package(Curses REQUIRED)
if(CURSES_FOUND)
	target_include_directories(${PROJECT_NAME} PRIVATE ${CURSES_INCLUDE_DIR})
	# Link ncursesw, panelw, and math
	target_link_libraries(${PROJECT_NAME} PRIVATE panelw ncursesw m)
endif()

# Ensure logs directory exists after build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/logs
)

