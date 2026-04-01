######################################################################
# @author      : ElGatoPanzon (contact@elgatopanzon.io)
# @file        : CMakeLists
# @created     : Wednesday Apr 01, 2026 13:08:15 CST
# @description : Build config for whisker-module-raylib module
######################################################################

cmake_minimum_required(VERSION 3.14)
project(whisker-module-raylib C)

# Check for local whisker first, then fall back to FetchContent
if(EXISTS "$ENV{HOME}/Projects/whisker")
    message(STATUS "Using local whisker from $ENV{HOME}/Projects/whisker")
    add_subdirectory($ENV{HOME}/Projects/whisker whisker)
else()
    message(STATUS "Fetching whisker from GitHub")
    include(FetchContent)
    FetchContent_Declare(
        whisker
        GIT_REPOSITORY https://github.com/ElGatoPanzon/whisker.git
        GIT_TAG main
    )
    FetchContent_MakeAvailable(whisker)
endif()

# Fetch raylib
include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
)
FetchContent_MakeAvailable(raylib)

file(GLOB_RECURSE WHISKER-MODULE-RAYLIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_library(whisker_module_raylib OBJECT
    ${WHISKER-MODULE-RAYLIB_SOURCES}
)

target_include_directories(whisker_module_raylib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(whisker_module_raylib
    PUBLIC
        whisker
        raylib
)

target_compile_options(whisker_module_raylib PRIVATE -std=c11)
