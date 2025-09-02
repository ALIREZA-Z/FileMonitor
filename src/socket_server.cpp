#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <afunix.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>


#pragma comment(lib, "ws2_32.lib")
#include "common/socket_utils.h"

// Simple parser: splits a string by delimiter
std::vector<std::string> split(const std::string& str, char delim) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

int main() {
    try {
        WinsockRAII winsock;
        std::string socketPath = getTempSocketPath();

        SOCKET serverSock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (serverSock == INVALID_SOCKET)
            throw std::runtime_error("Socket creation failed");

        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strcpy_s(addr.sun_path, socketPath.c_str());

        DeleteFileA(socketPath.c_str());

        if (bind(serverSock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
            throw std::runtime_error("Bind failed");

        if (listen(serverSock, 5) == SOCKET_ERROR)
            throw std::runtime_error("Listen failed");

        std::cout << "Server listening on: " << socketPath << "\n";

        // Map clientID -> pair(src, dest)
        std::map<std::string, std::pair<std::string, std::string>> clients;

        while (true) {
            SOCKET clientSock = accept(serverSock, nullptr, nullptr);
            if (clientSock == INVALID_SOCKET) continue;

            char buffer[1024]{0};
            int received = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
            if (received > 0) {
                buffer[received] = '\0';
                std::string msg(buffer);

                auto parts = split(msg, '|');
                if (parts.size() == 3) {
                    // Registration: clientID|src|dest
                    std::string clientID = parts[0];
                    std::string src = parts[1];
                    std::string dest = parts[2];
                    clients[clientID] = {src, dest};
                    std::cout << "[Server] Registered client: " << clientID 
                              << " src: " << src << " dest: " << dest << "\n";
                } else if (parts.size() == 2) {
                    // Normal message: clientID|message
                    std::string clientID = parts[0];
                    std::string actionMsg = parts[1];
                    auto it = clients.find(clientID);
                    if (it != clients.end()) {
                        std::cout << "[Server][" << clientID << "] " 
                                  << actionMsg << " (src: " << it->second.first 
                                  << " dest: " << it->second.second << ")\n";
                        // TODO: trigger robocopy using src/dest if needed
                    } else {
                        std::cout << "[Server] Unknown client: " << clientID 
                                  << " message: " << actionMsg << "\n";
                    }
                } else {
                    std::cout << "[Server] Invalid message: " << msg << "\n";
                }
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
