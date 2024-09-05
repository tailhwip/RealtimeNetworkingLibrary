cmake_minimum_required(VERSION 3.21)
Include(FetchContent)

project("Real-time Networking Library")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(rnlib)

target_sources(rnlib PUBLIC
    include/rnlib/address.h
    include/rnlib/packet.h
    include/rnlib/socket.h
    include/rnlib/sockets.h)

target_sources(rnlib PRIVATE
    src/address.cpp
    src/socket.cpp
    src/sockets.cpp)

if(WIN32)
    target_link_libraries(rnlib PRIVATE wsock32)
endif()