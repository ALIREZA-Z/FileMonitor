#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <afunix.h>
#include <iostream>
#include <string>
#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    // Get current folder
    char cwd[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, cwd);
    std::string socketPath = std::string(cwd) + "\\filemonitor.sock";

    SOCKET serverSock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strcpy_s(addr.sun_path, socketPath.c_str());

    // Delete old socket file if exists
    DeleteFile(socketPath.c_str());

    if (bind(serverSock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    if (listen(serverSock, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on: " << socketPath << "\n";

    while (true) {
        SOCKET clientSock = accept(serverSock, nullptr, nullptr);
        if (clientSock == INVALID_SOCKET) continue;

        char buffer[256]{0};
        int received = recv(clientSock, buffer, sizeof(buffer)-1, 0);
        if (received > 0) {
            buffer[received] = '\0';
            std::cout << "Received: " << buffer << "\n";
        }

        closesocket(clientSock);
    }

    closesocket(serverSock);
    WSACleanup();
    return 0;
}
