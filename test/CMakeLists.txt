cmake_minimum_required(VERSION 3.0)

file(GLOB files "test_*.c")

foreach(file ${files})
    get_filename_component(target ${file} NAME_WLE)
    add_executable(${target} ${file} ../log.c)
    target_compile_definitions(${target} PUBLIC BUILD_TARGET_SERVER=1)
    add_test(NAME ${target} COMMAND ${target})
endforeach()

