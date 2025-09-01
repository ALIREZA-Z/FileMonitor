#include <winsock2.h>   // must come first
#include <ws2tcpip.h>
#include <windows.h>
#include <stdexcept>
#include "socket_utils.h"

#pragma comment(lib, "ws2_32.lib")


std::string getTempSocketPath(const std::string& sockName) {
    char tmpPath[MAX_PATH];
    if (GetTempPathA(MAX_PATH, tmpPath) == 0) {
        throw std::runtime_error("Failed to get temp path");
    }
    std::string path(tmpPath);
    if (!path.empty() && path.back() != '\\') {
        path.push_back('\\');
    }
    path += sockName;
    return path;
}

WinsockRAII::WinsockRAII() {
    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
}

WinsockRAII::~WinsockRAII() {
    WSACleanup();
}
