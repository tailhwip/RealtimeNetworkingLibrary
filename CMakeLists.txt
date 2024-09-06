cmake_minimum_required(VERSION 3.21)
Include(FetchContent)

project(rnlib)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(rnlib)

target_sources(rnlib PUBLIC
    include/rnlib/address.h
    include/rnlib/cryptography.h
    include/rnlib/socket.h)

target_sources(rnlib PRIVATE
    src/address.cpp
    src/cryptography.cpp
    src/socket.cpp)

if(WIN32)
    target_link_libraries(rnlib PRIVATE wsock32)
endif()

# Dependencies
set(PROJECT_VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor")

if(WIN32)
    if(MSVC_VERSION GREATER_EQUAL 1930)
        set(VENDOR_LIBRARY_PLATFORM "win32/v143")
    else()
        set(VENDOR_LIBRARY_PLATFORM "win32/v142")
    endif()
    set(VENDOR_LIBRARY_EXT "lib")
elseif(UNIX)
    set(VENDOR_LIBRARY_PLATFORM "unix")
    set(VENDOR_LIBRARY_EXT "a")
else()
    message(FATAL_ERROR "Unsupported Platform")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(VENDOR_LIBRARY_BUILD "debug")
else()
    set(VENDOR_LIBRARY_BUILD "release")
endif()

set(VENDOR_LIBRARY_PATH "lib/${VENDOR_LIBRARY_PLATFORM}/${VENDOR_LIBRARY_BUILD}")

# libsodium
target_include_directories(rnlib PRIVATE "${PROJECT_VENDOR_DIR}/libsodium/include")
target_link_libraries(rnlib PRIVATE "${PROJECT_VENDOR_DIR}/libsodium/${VENDOR_LIBRARY_PATH}/libsodium.${VENDOR_LIBRARY_EXT}")

# libcrc
target_include_directories(rnlib PRIVATE "${PROJECT_VENDOR_DIR}/libcrc/include")
target_link_libraries(rnlib PRIVATE "${PROJECT_VENDOR_DIR}/libcrc/${VENDOR_LIBRARY_PATH}/libcrc.${VENDOR_LIBRARY_EXT}")
