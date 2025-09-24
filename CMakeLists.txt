cmake_minimum_required(VERSION 4.0)
project(kazl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(.)

add_executable(kazl
        main.cpp
        staticlib.h)
