#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <afunix.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#include "common/socket_utils.h"
#include "common/string_utils.h"
#include "common/id_utils.h"

// Send message to server
void sendMessage(const std::string& socketPath, const std::string& clientId, const std::string& msg) {
    try {
        WinsockRAII winsock;  // ensures WSACleanup() at scope exit

        SOCKET sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
#ifndef NDEBUG
            std::cerr << "[Client] Socket creation failed, error: " << WSAGetLastError() << std::endl;
#endif
            return;
        }

        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strcpy_s(addr.sun_path, socketPath.c_str());

#ifndef NDEBUG
        std::cout << "[Client] Attempting to connect to: " << socketPath << std::endl;
#endif

        if (connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
#ifndef NDEBUG
            std::cerr << "[Client] Connect failed, error: " << WSAGetLastError() << std::endl;
#endif
            closesocket(sock);
            return;
        }

#ifndef NDEBUG
        std::cout << "[Client] Connected successfully. Sending message..." << std::endl;
#endif

        // Prepend clientId to message: "clientId|message"
        std::string fullMsg = clientId + "|" + msg;
        send(sock, fullMsg.c_str(), static_cast<int>(fullMsg.size()), 0);

#ifndef NDEBUG
        std::cout << "[Client] Message sent: " << fullMsg << std::endl;
#endif
        closesocket(sock);

#ifndef NDEBUG
        std::cout << "[Client] Disconnected." << std::endl;
#endif
    } catch (const std::exception& ex) {
        std::cerr << "[Client] Exception: " << ex.what() << std::endl;
    }
}

void monitorDirectory(const std::wstring& dir, const std::string& socketPath, const std::string& clientId) {
    HANDLE hDir = CreateFileW(
        dir.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        nullptr
    );

    if (hDir == INVALID_HANDLE_VALUE) {
        std::string errorMsg = "ERROR: Cannot monitor folder: " + StringUtils::wstringToUtf8(dir);
        sendMessage(socketPath, clientId, errorMsg);
#ifndef NDEBUG
        std::cerr << errorMsg << std::endl;
#endif
        return; // Stop monitoring
    }

#ifndef NDEBUG
    std::wcout << L"Monitoring directory: " << dir << std::endl;
#endif

    char buffer[1024];
    DWORD bytesReturned;

    while (true) {
        if (ReadDirectoryChangesW(
            hDir,
            &buffer,
            sizeof(buffer),
            FALSE, // no subdirectories
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytesReturned,
            nullptr,
            nullptr
        )) {
            FILE_NOTIFY_INFORMATION* fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
            do {
                std::wstring fileName(fni->FileName, fni->FileNameLength / sizeof(WCHAR));
                std::string utf8File = StringUtils::wstringToUtf8(fileName);

                std::string action;
                switch (fni->Action) {
                    case FILE_ACTION_ADDED: action = "Created: "; break;
                    case FILE_ACTION_MODIFIED: action = "Modified: "; break;
                    case FILE_ACTION_REMOVED: action = "Deleted: "; break;
                    case FILE_ACTION_RENAMED_OLD_NAME: action = "Renamed from: "; break;
                    case FILE_ACTION_RENAMED_NEW_NAME: action = "Renamed to: "; break;
                    default: action = "Other: "; break;
                }

#ifndef NDEBUG
                std::cout << action << utf8File << std::endl;
#endif
                sendMessage(socketPath, clientId, action + utf8File);

                if (fni->NextEntryOffset == 0) break;
                fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>((LPBYTE)fni + fni->NextEntryOffset);
            } while (true);
        }
    }

    CloseHandle(hDir);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: file_monitor.exe <src_folder_path> <dst_folder_path>\n";
        return 1;
    }

    std::string socketPath = getTempSocketPath();

    // Convert source folder path to wstring
    std::wstring folderPath = StringUtils::utf8ToWstring(argv[1]);

    // Generate a short unique client ID (e.g., 8 chars)
    std::string clientId = IdUtils::generateShortId(8);
#ifndef NDEBUG
    std::cout << "[Client] Generated client ID: " << clientId << std::endl;
#endif

    // Send initial message with source and destination paths
    std::string initMsg = "SRC:" + std::string(argv[1]) + "|DST:" + std::string(argv[2]);
    sendMessage(socketPath, clientId, initMsg);

    // Start monitoring
    monitorDirectory(folderPath, socketPath, clientId);

    return 0;
}
