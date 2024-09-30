cmake_minimum_required(VERSION 3.0)

set(MODULES ${PROJECT_NAME}-test-modules)

add_library(${MODULES} SHARED
    ../log.c
    ../ui.c
    ../network.c
    ../chat.c)
target_compile_definitions(${MODULES} PUBLIC BUILD_TARGET_SERVER=1)

file(GLOB files "test_*.c")

foreach(file ${files})
    get_filename_component(target ${file} NAME_WLE)
    
    add_executable(${target} ${file})
    
    target_link_libraries(${target} PRIVATE ${MODULES} ${CURSES_LIBRARY})
    
    target_compile_definitions(${target} PUBLIC BUILD_TARGET_SERVER=1)
    
    add_test(NAME ${target} COMMAND ${target})
endforeach()

