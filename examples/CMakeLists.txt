include_directories(..)
add_executable(demo demo.cpp)
target_link_libraries(demo file_monitor)