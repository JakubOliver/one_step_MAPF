cmake_minimum_required(VERSION 3.20)

project(MAPF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)

FetchContent_MakeAvailable(argparse)

FetchContent_Declare(
        csv_parser
        GIT_REPOSITORY https://github.com/vincentlaucsb/csv-parser.git
        GIT_TAG 2.3.0
)

FetchContent_MakeAvailable(csv_parser)

file(GLOB_RECURSE SRC_ALL CONFIGURE_DEPENDS src/*.cpp)

add_executable(MAPF ${SRC_ALL}
        src/InputLoader.cpp
        src/InputLoader.h
        src/Scenario.cpp
        src/Scenario.h
        src/Map.cpp
        src/Map.h
        src/algorithms/PriorityPlaning.cpp
        src/algorithms/PriorityPlaning.h
        src/Agent.cpp
        src/Agent.h)

target_link_libraries(
        MAPF
        PRIVATE
        argparse
        csv
)