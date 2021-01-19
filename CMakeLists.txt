project(file_monitor)
cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 14)

add_library(file_monitor SHARED file_monitor.cpp)

add_subdirectory(examples)

set(CMAKE_BUILD_TYPE Debug)