#pragma once
#include <string>

// Utility for generating socket path
std::string getTempSocketPath(const std::string& sockName = "filemonitor.sock");

// RAII cleanup class for Winsock
class WinsockRAII {
public:
    WinsockRAII();
    ~WinsockRAII();
};
