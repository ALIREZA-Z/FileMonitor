cmake_minimum_required(VERSION 3.10)
project(FileMonitorProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable vcpkg toolchain
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Find cppzmq (ZeroMQ C++ bindings + C lib)
find_package(cppzmq CONFIG REQUIRED)

# Common library
add_library(common STATIC
    src/common/socket_utils.cpp
)

target_include_directories(common PUBLIC src/common)

# Executables
add_executable(file_monitor src/file_monitor.cpp)
add_executable(socket_server src/socket_server.cpp)

# Link common + ws2_32 + cppzmq
target_link_libraries(file_monitor PRIVATE common ws2_32 cppzmq)
target_link_libraries(socket_server PRIVATE common ws2_32 cppzmq)
