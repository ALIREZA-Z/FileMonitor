#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <afunix.h>
#include <iostream>
#include <string>
#pragma comment(lib, "ws2_32.lib")

#include "common/socket_utils.h"

int main() {
    try {
        WinsockRAII winsock;
        std::string socketPath = getTempSocketPath();

        SOCKET serverSock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (serverSock == INVALID_SOCKET) {
            throw std::runtime_error("Socket creation failed");
        }

        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strcpy_s(addr.sun_path, socketPath.c_str());

        DeleteFileA(socketPath.c_str());

        if (bind(serverSock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
            throw std::runtime_error("Bind failed");
        }

        if (listen(serverSock, 5) == SOCKET_ERROR) {
            throw std::runtime_error("Listen failed");
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
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}

