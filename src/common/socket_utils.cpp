#include "socket_utils.h"
#include <windows.h>
#include <stdexcept>

std::string getTempSocketPath(const std::string& name) {
    return R"(\\.\pipe\)" + name;  // Use Windows named pipe path
}
