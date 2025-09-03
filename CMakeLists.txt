cmake_minimum_required(VERSION 3.10)
project(FileMonitorProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Organize build outputs
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable vcpkg toolchain if not already set
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

# Find ZeroMQ (via vcpkg or system)
find_package(ZeroMQ REQUIRED)

# Common library
add_library(common STATIC
    src/common/socket_utils.cpp
)
target_include_directories(common PUBLIC ${CMAKE_SOURCE_DIR}/src/common)

# Executables
add_executable(file_monitor src/file_monitor.cpp)
add_executable(socket_server src/socket_server.cpp)

# Link against common + ZeroMQ
target_link_libraries(file_monitor PRIVATE common libzmq)
target_link_libraries(socket_server PRIVATE common libzmq)

# Windows-specific networking lib
if(WIN32)
    target_link_libraries(file_monitor PRIVATE ws2_32)
    target_link_libraries(socket_server PRIVATE ws2_32)
endif()
