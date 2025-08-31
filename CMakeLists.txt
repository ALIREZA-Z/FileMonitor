cmake_minimum_required(VERSION 3.10)
project(FileMonitorProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(file_monitor src/file_monitor.cpp)
add_executable(socket_server src/socket_server.cpp)

if(WIN32)
    target_link_libraries(file_monitor ws2_32)
    target_link_libraries(socket_server ws2_32)
endif()
