#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <afunix.h>
#include <iostream>
#include <string>
#pragma comment(lib, "ws2_32.lib")

void sendMessage(const std::string& socketPath, const std::string& msg) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    SOCKET sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strcpy_s(addr.sun_path, socketPath.c_str());

    if (connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        // std::cerr << "Connect failed\n";   // keep quiet if server not ready
        closesocket(sock);
        WSACleanup();
        return;
    }

    send(sock, msg.c_str(), static_cast<int>(msg.length()), 0);
    closesocket(sock);
    WSACleanup();
}

void monitorDirectory(const std::wstring& dir, const std::string& socketPath) {
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
        std::cerr << "Failed to monitor directory\n";
        return;
    }

    char buffer[1024];
    DWORD bytesReturned;

    std::wcout << L"Monitoring directory: " << dir << std::endl;

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

                // convert wide string to UTF-8
                int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, fileName.c_str(), (int)fileName.size(), NULL, 0, NULL, NULL);
                std::string utf8File(sizeNeeded, 0);
                WideCharToMultiByte(CP_UTF8, 0, fileName.c_str(), (int)fileName.size(), &utf8File[0], sizeNeeded, NULL, NULL);

                std::string action;
                switch (fni->Action) {
                    case FILE_ACTION_ADDED: action = "Created: "; break;
                    case FILE_ACTION_MODIFIED: action = "Modified: "; break;
                    case FILE_ACTION_REMOVED: action = "Deleted: "; break;
                    case FILE_ACTION_RENAMED_OLD_NAME: action = "Renamed from: "; break;
                    case FILE_ACTION_RENAMED_NEW_NAME: action = "Renamed to: "; break;
                    default: action = "Other: "; break;
                }

                std::cout << action << utf8File << std::endl;
                sendMessage(socketPath, action + utf8File);

                if (fni->NextEntryOffset == 0) break;
                fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>((LPBYTE)fni + fni->NextEntryOffset);
            } while (true);
        }
    }

    CloseHandle(hDir);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: file_monitor.exe <folder_path>\n";
        return 1;
    }

    // Determine current folder for socket
    char cwd[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, cwd);
    std::string socketPath = std::string(cwd) + "\\filemonitor.sock";

    // Convert argv to wide string
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, NULL, 0);
    std::wstring folderPath(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, &folderPath[0], size_needed);

    monitorDirectory(folderPath, socketPath);
    return 0;
}
