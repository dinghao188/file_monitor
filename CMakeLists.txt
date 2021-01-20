project(file_monitor)
cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)

include_directories(lib)
add_subdirectory(lib)

set(BUILD_EXAMPLE ON)
if(BUILD_EXAMPLE)
    add_subdirectory(examples)
endif()