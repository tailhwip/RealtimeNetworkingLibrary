cmake_minimum_required(VERSION 3.21)

project("Real-time Networking Library")
set(CMAKE_C_STANDARD 99)

add_library(rnlib)

if(WIN32)
    target_link_libraries(rnlib PRIVATE wsock32)
endif()
