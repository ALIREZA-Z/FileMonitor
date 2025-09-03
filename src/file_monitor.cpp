#include <windows.h>
#include <iostream>
#include <string>
#include <zmq.hpp>

#include "common/string_utils.h"
#include "common/id_utils.h"

// Global ZeroMQ context
zmq::context_t zmqContext(1);

// Send message to server via ZeroMQ
void sendMessage(zmq::socket_t& socket, const std::string& clientId, const std::string& msg) {
    try {
        std::string fullMsg = clientId + "|" + msg;
        zmq::message_t zmqMsg(fullMsg.begin(), fullMsg.end());
        socket.send(zmqMsg, zmq::send_flags::none);

#ifndef NDEBUG
        std::cout << "[Client] Message sent: " << fullMsg << std::endl;
#endif
    } catch (const zmq::error_t& ex) {
        std::cerr << "[Client] ZeroMQ error: " << ex.what() << std::endl;
    }
}

// Check if folder can be accessed and monitored
bool checkFolderAccessible(const std::wstring& folderPath) {
    HANDLE hDir = CreateFileW(
        folderPath.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        nullptr
    );

    if (hDir == INVALID_HANDLE_VALUE) {
        std::string errorMsg = "ERROR: Cannot open source folder: " + StringUtils::wstringToUtf8(folderPath);
        std::cerr << errorMsg << std::endl;
        return false;
    }

    CloseHandle(hDir);
    return true;
}

void monitorDirectory(const std::wstring& dir, zmq::socket_t& socket, const std::string& clientId) {
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
        sendMessage(socket, clientId, errorMsg);
        std::cerr << errorMsg << std::endl;
        return;
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
            FALSE,
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

                sendMessage(socket, clientId, action + utf8File);

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

    std::wstring folderPath = StringUtils::utf8ToWstring(argv[1]);

    if (!checkFolderAccessible(folderPath)) {
        std::cerr << StringUtils::wstringToUtf8(folderPath) << " is not accessible.";
        return 1;
    }

    // Unique client ID
    std::string clientId = IdUtils::generateShortId(8);
    std::cout << "[Client] Generated client ID: " << clientId << std::endl;

    // Connect to server (PUSH → tcp://127.0.0.1:5555)
    zmq::socket_t socket(zmqContext, zmq::socket_type::push);
    socket.connect("tcp://127.0.0.1:5555");

    // Send initial SRC/DST message
    std::string initMsg = "SRC:" + std::string(argv[1]) + "|DST:" + std::string(argv[2]);
    sendMessage(socket, clientId, initMsg);

    // Start monitoring
    monitorDirectory(folderPath, socket, clientId);

    return 0;
}
