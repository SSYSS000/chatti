cmake_minimum_required(VERSION 3.0)

project(chatti LANGUAGES C)

add_executable(${PROJECT_NAME}-server
    network.c
    log.c
    chat.c
    server.c)

add_executable(${PROJECT_NAME}-client
    network.c
    log.c
    chat.c
    ui.c
    client.c)
