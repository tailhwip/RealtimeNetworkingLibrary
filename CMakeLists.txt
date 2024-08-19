cmake_minimum_required(VERSION 3.21)

project("Real-time Networking Library")
set(CMAKE_C_STANDARD 99)

add_library(rnlib)

target_sources(rnlib PUBLIC
    include/rnlib/address.h
    include/rnlib/result.h
    include/rnlib/socket.h)

target_sources(rnlib PRIVATE
    src/address.h src/address.c
    src/socket.h src/socket.c)

if(WIN32)
    target_link_libraries(rnlib PRIVATE wsock32)
endif()