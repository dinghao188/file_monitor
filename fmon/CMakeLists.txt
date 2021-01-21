add_library(fmon SHARED file_monitor.cpp)
target_include_directories(fmon PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)