cmake_minimum_required(VERSION 3.0)

project(chatti LANGUAGES C)


set(SERVER_TARGET ${PROJECT_NAME}-server)
set(CLIENT_TARGET ${PROJECT_NAME}-client)

set(CMAKE_BUILD_TYPE Debug)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG=1)
endif()

add_executable(${SERVER_TARGET}
    network.c
    log.c
    chat.c
    server.c)

target_compile_definitions(${SERVER_TARGET} PUBLIC BUILD_TARGET_SERVER=1)

add_executable(${CLIENT_TARGET}
    network.c
    log.c
    chat.c
    ui.c
    client.c)

target_link_libraries(${CLIENT_TARGET} PRIVATE ncurses)

target_compile_definitions(${CLIENT_TARGET} PUBLIC BUILD_TARGET_CLIENT=1)

include(CTest)
add_subdirectory(test)

