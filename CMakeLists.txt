cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("MAPF")

file(GLOB_RECURSE SRC_ALL src/*.cpp)
set(MAIN_SRC src/Main.cpp)

add_executable(MAPF ${MAIN_SRC} ${SRC_ALL})
set(CMAKE_CXX_EXTENSIONS OFF)
set_target_properties(MAPF PROPERTIES CXX_STANDARD 23)