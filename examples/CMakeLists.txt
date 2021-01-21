function(example example)
    set(EXE_NAME "examples.${example}")
    string(REPLACE "/" "_" EXE_NAME "${EXE_NAME}")
    add_executable(${EXE_NAME} ${example}.cpp)
    target_link_libraries(${EXE_NAME} fmon)
endfunction()

example(demo)