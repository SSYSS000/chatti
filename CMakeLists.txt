cmake_minimum_required(VERSION 3.0)

project(chatti LANGUAGES C)

set(SERVER_TARGET ${PROJECT_NAME}-server)
set(CLIENT_TARGET ${PROJECT_NAME}-client)

set(CMAKE_BUILD_TYPE Debug)

add_executable(${SERVER_TARGET}
    network.c
    log.c
    chat.c
    server.c)

add_executable(${CLIENT_TARGET}
    network.c
    log.c
    chat.c
    ui.c
    client.c)

target_link_libraries(${CLIENT_TARGET} PRIVATE ncurses)
