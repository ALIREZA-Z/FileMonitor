#include <asio.hpp>
#include <asio/local/stream_protocol.hpp>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

#include "common/socket_utils.h"

using asio::local::stream_protocol;

void sendMessage(asio::io_context& io, const std::string& socketPath, const std::string& msg) {
    try {
        stream_protocol::socket socket(io);
        socket.connect(stream_protocol::endpoint(socketPath));
        asio::write(socket, asio::buffer(msg));
    } catch (...) {
        // silently ignore if server not ready
    }
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

    if (hDir == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Failed to monitor directory");

    std::vector<char> buffer(1024);
    DWORD bytesReturned = 0;
    asio::io_context io;

    std::wcout << L"Monitoring directory: " << dir << "\n";

    while (true) {
        if (ReadDirectoryChangesW(
            hDir,
            buffer.data(),
            static_cast<DWORD>(buffer.size()),
            FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytesReturned,
            nullptr,
            nullptr
        )) {
            auto* fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer.data());
            do {
                std::wstring fileName(fni->FileName, fni->FileNameLength / sizeof(WCHAR));
                int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, fileName.c_str(),
                                                     static_cast<int>(fileName.size()),
                                                     nullptr, 0, nullptr, nullptr);
                std::string utf8File(sizeNeeded, '\0');
                WideCharToMultiByte(CP_UTF8, 0, fileName.c_str(),
                                    static_cast<int>(fileName.size()),
                                    utf8File.data(), sizeNeeded, nullptr, nullptr);

                std::string action;
                switch (fni->Action) {
                    case FILE_ACTION_ADDED: action = "Created: "; break;
                    case FILE_ACTION_MODIFIED: action = "Modified: "; break;
                    case FILE_ACTION_REMOVED: action = "Deleted: "; break;
                    case FILE_ACTION_RENAMED_OLD_NAME: action = "Renamed from: "; break;
                    case FILE_ACTION_RENAMED_NEW_NAME: action = "Renamed to: "; break;
                    default: action = "Other: "; break;
                }

                std::string msg = action + utf8File;
                std::cout << msg << "\n";
                sendMessage(io, socketPath, msg);

                if (fni->NextEntryOffset == 0) break;
                fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                    reinterpret_cast<BYTE*>(fni) + fni->NextEntryOffset
                );
            } while (true);
        }
    }

    CloseHandle(hDir);
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: file_monitor.exe <folder_path>\n";
            return 1;
        }

        std::string socketPath = getTempSocketPath();
        int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, nullptr, 0);
        std::wstring folderPath(sizeNeeded, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, folderPath.data(), sizeNeeded);

        monitorDirectory(folderPath, socketPath);

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}
