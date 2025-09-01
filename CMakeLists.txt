cmake_minimum_required(VERSION 3.10)
project(FileMonitorProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Fetch standalone Asio
include(FetchContent)
FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG        asio-1-30-2
)
FetchContent_MakeAvailable(asio)

# Common library
add_library(common STATIC src/common/socket_utils.cpp)
target_include_directories(common PUBLIC src/common)

# --------------------------
# Windows version check
# --------------------------
if(WIN32)
    # Set minimum Windows version to 10
    add_compile_definitions(_WIN32_WINNT=0x0A00)

    # Optional: warn if building on older Windows SDK
    message(STATUS "Targeting Windows 10 or higher (_WIN32_WINNT=0x0A00)")
endif()

# --------------------------
# Executables
# --------------------------
add_executable(file_monitor src/file_monitor.cpp)
target_link_libraries(file_monitor PRIVATE common)
target_include_directories(file_monitor PRIVATE ${asio_SOURCE_DIR}/asio/include)

# Only build socket_server on Windows 10 or higher
if(WIN32)
    add_executable(socket_server src/socket_server.cpp)
    target_link_libraries(socket_server PRIVATE common ws2_32)
    target_include_directories(socket_server PRIVATE ${asio_SOURCE_DIR}/asio/include)
endif()
