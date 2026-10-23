cmake_minimum_required(VERSION 3.20)

project(MAPF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
include(CTest)

option(MAPF_BUILD_VISUALIZER "Build visualization support" ON)

# Cmake is combined from the original MAPF-visualizer and openFrameworks repo Makes and Cmakes
# and is enriched with few libraries used in this project

# TODO: check support for Windows, Mac and different Linux distros and DE


# ============================================================
# argparse
# ============================================================

FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)

FetchContent_MakeAvailable(argparse)


# ============================================================
# csv-parser
# ============================================================

FetchContent_Declare(
        csv_parser
        GIT_REPOSITORY https://github.com/vincentlaucsb/csv-parser.git
        GIT_TAG 2.3.0
)

FetchContent_MakeAvailable(csv_parser)

# csv-parser instruments its Debug build with --coverage, but does not expose
# the corresponding runtime requirement to targets that link its static library.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(
            csv
            INTERFACE
            $<$<CONFIG:Debug>:--coverage>
    )
endif()


# ============================================================
# GoogleTest
# ============================================================

if(BUILD_TESTING)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.15.2
            GIT_SHALLOW TRUE
    )

    FetchContent_MakeAvailable(googletest)
endif()


# ============================================================
# mapf-visualizer / openFrameworks
# ============================================================

if(MAPF_BUILD_VISUALIZER)
    find_package(PkgConfig REQUIRED)

    set(MAPF_VISUALIZER_DIR
            ${CMAKE_SOURCE_DIR}/external/mapf-visualizer
    )

    set(OF_ROOT
            ${MAPF_VISUALIZER_DIR}/third_party/openFrameworks
    )


    # ------------------------------------------------------------
    # openFrameworks system dependencies
    # ------------------------------------------------------------

    pkg_check_modules(
            OF_SYSTEM_DEPS
            REQUIRED
            IMPORTED_TARGET

            cairo
            zlib

            gstreamer-app-1.0
            gstreamer-1.0
            gstreamer-video-1.0
            gstreamer-base-1.0

            libudev

            freetype2
            fontconfig

            sndfile
            openal

            libcurl

            glfw3
            rtaudio

            libpulse-simple
            alsa

            gl
            glu
            glew

            gtk+-3.0

            libmpg123
    )


    # ------------------------------------------------------------
    # ofxGui
    # ------------------------------------------------------------

    file(GLOB OFXGUI_SOURCES
            CONFIGURE_DEPENDS
            ${OF_ROOT}/addons/ofxGui/src/*.cpp
    )


    # ------------------------------------------------------------
    # mapf-visualizer library
    # ------------------------------------------------------------

    add_library(
            mapf_visualizer
            STATIC

            ${MAPF_VISUALIZER_DIR}/src/Visualizer.cpp
            ${MAPF_VISUALIZER_DIR}/src/ofApp.cpp
            ${MAPF_VISUALIZER_DIR}/src/graph.cpp

            ${OFXGUI_SOURCES}
    )
endif()


# ------------------------------------------------------------
# Google Or Tools
# ------------------------------------------------------------

# OR-Tools dependencies
set(BUILD_DEPS ON CACHE BOOL "" FORCE)

# Disable unnecessary components
set(BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(MAPF_BUILD_TESTING ${BUILD_TESTING})
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_FLATZINC OFF CACHE BOOL "" FORCE)

# Keep enabled (required by current build)
set(BUILD_MATH_OPT ON CACHE BOOL "" FORCE)

# Disable optional solvers
set(USE_COINOR OFF CACHE BOOL "" FORCE)
set(USE_SCIP OFF CACHE BOOL "" FORCE)
set(USE_HIGHS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        or-tools
        GIT_REPOSITORY https://github.com/google/or-tools.git
        GIT_TAG stable
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(or-tools)

set(BUILD_TESTING ${MAPF_BUILD_TESTING} CACHE BOOL "Build project tests" FORCE)
unset(MAPF_BUILD_TESTING)

# ------------------------------------------------------------
# Include directories
# ------------------------------------------------------------

if(MAPF_BUILD_VISUALIZER)
    target_include_directories(
            mapf_visualizer
            PUBLIC

            # mapf-visualizer
            ${MAPF_VISUALIZER_DIR}/src
            ${MAPF_VISUALIZER_DIR}/include

            # openFrameworks core
            ${OF_ROOT}/libs/openFrameworks
            ${OF_ROOT}/libs/openFrameworks/3d
            ${OF_ROOT}/libs/openFrameworks/app
            ${OF_ROOT}/libs/openFrameworks/communication
            ${OF_ROOT}/libs/openFrameworks/events
            ${OF_ROOT}/libs/openFrameworks/gl
            ${OF_ROOT}/libs/openFrameworks/graphics
            ${OF_ROOT}/libs/openFrameworks/math
            ${OF_ROOT}/libs/openFrameworks/sound
            ${OF_ROOT}/libs/openFrameworks/types
            ${OF_ROOT}/libs/openFrameworks/utils
            ${OF_ROOT}/libs/openFrameworks/video

            # bundled openFrameworks dependencies
            ${OF_ROOT}/libs/fmod/include
            ${OF_ROOT}/libs/glm/include
            ${OF_ROOT}/libs/json/include
            ${OF_ROOT}/libs/kiss/include
            ${OF_ROOT}/libs/tess2/include
            ${OF_ROOT}/libs/utf8/include

            # addons
            ${OF_ROOT}/addons/ofxGui/src
    )


    # ------------------------------------------------------------
    # Compile definitions used by openFrameworks on Linux
    # ------------------------------------------------------------

    target_compile_definitions(
            mapf_visualizer
            PUBLIC

            GCC_HAS_REGEX
            OF_USING_GTK
            OF_USING_MPG123
    )


    # ------------------------------------------------------------
    # openFrameworks + bundled + Linux libraries
    # ------------------------------------------------------------

    target_link_libraries(
            mapf_visualizer
            PUBLIC

            # openFrameworks core
            ${OF_ROOT}/libs/openFrameworksCompiled/lib/linux64/libopenFrameworks.a

            # bundled static libraries used by openFrameworks
            ${OF_ROOT}/libs/kiss/lib/linux64/libkiss.a
            ${OF_ROOT}/libs/tess2/lib/linux64/libtess2.a

            # pkg-config dependencies
            PkgConfig::OF_SYSTEM_DEPS

            # Linux platform libraries
            glut

            X11
            Xrandr
            Xxf86vm
            Xi
            Xcursor
            Xinerama

            dl
            pthread

            freeimage
            pugixml
            uriparser
    )
endif()

# ============================================================
# MAPF
# ============================================================

file(
        GLOB_RECURSE
        MAPF_CORE_SOURCES
        CONFIGURE_DEPENDS
        src/*.cpp
)

list(
        REMOVE_ITEM
        MAPF_CORE_SOURCES
        ${CMAKE_SOURCE_DIR}/src/Main.cpp
)

add_library(
        mapf_core
        STATIC
        ${MAPF_CORE_SOURCES}
)

target_include_directories(
        mapf_core
        PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(
        mapf_core
        PRIVATE
        csv
        ortools::ortools
)

add_executable(
        MAPF
        src/Main.cpp
)

target_link_libraries(
        MAPF
        PRIVATE

        argparse
        mapf_core
)

target_compile_definitions(
        MAPF
        PRIVATE
        MAPF_WITH_VISUALIZER=$<BOOL:${MAPF_BUILD_VISUALIZER}>
)

if(MAPF_BUILD_VISUALIZER)
    target_link_libraries(MAPF PRIVATE mapf_visualizer)
endif()


# ============================================================
# Tests
# ============================================================

if(BUILD_TESTING)
    file(
            GLOB_RECURSE
            TEST_SOURCES
            CONFIGURE_DEPENDS
            tests/*.cpp
    )

    add_executable(
            mapf_tests
            ${TEST_SOURCES}
    )

    target_link_libraries(
            mapf_tests
            PRIVATE
            mapf_core
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(mapf_tests)
endif()
