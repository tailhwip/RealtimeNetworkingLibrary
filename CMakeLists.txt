cmake_minimum_required(VERSION 3.21)
include(FetchContent)

project(rnlib)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(rnlib)

target_sources(
    rnlib
    PUBLIC include/rnlib/address.h 
           include/rnlib/connection.h
           include/rnlib/cryptography.h
           include/rnlib/handshake.h
           include/rnlib/packet.h
           include/rnlib/socket.h)

target_sources(
    rnlib
    PRIVATE src/address.cpp
            src/connection.cpp
            src/cryptography.cpp
            src/handshake.cpp
            src/packet.cpp
            src/socket.cpp)

if(WIN32)
    target_link_libraries(rnlib PRIVATE wsock32)
endif()

# Dependencies
set(PROJECT_VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor")

if(WIN32)
    if(MSVC_VERSION GREATER_EQUAL 1930)
        set(VENDOR_LIB_PLATFORM "win32/v143")
    else()
        set(VENDOR_LIB_PLATFORM "win32/v142")
    endif()
    set(VENDOR_LIB_EXT "lib")
elseif(UNIX)
    set(VENDOR_LIB_PLATFORM "unix")
    set(VENDOR_LIB_EXT "a")
else()
    message(FATAL_ERROR "Unsupported Platform")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(VENDOR_LIB_BUILD "debug")
else()
    set(VENDOR_LIB_BUILD "release")
endif()

set(VENDOR_LIB_PATH "lib/${VENDOR_LIB_PLATFORM}/${VENDOR_LIB_BUILD}")

# libsodium
target_include_directories(rnlib PRIVATE "${PROJECT_VENDOR_DIR}/libsodium/include")
target_link_libraries(rnlib PRIVATE "${PROJECT_VENDOR_DIR}/libsodium/${VENDOR_LIB_PATH}/libsodium.${VENDOR_LIB_EXT}")

# libcrc
target_include_directories(rnlib PRIVATE "${PROJECT_VENDOR_DIR}/libcrc/include")
target_link_libraries(rnlib PRIVATE "${PROJECT_VENDOR_DIR}/libcrc/${VENDOR_LIB_PATH}/libcrc.${VENDOR_LIB_EXT}")
