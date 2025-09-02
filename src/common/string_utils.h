#pragma once
#include <string>
#include <windows.h>

namespace StringUtils {

    // Convert std::wstring → UTF-8 encoded std::string
    inline std::string wstringToUtf8(const std::wstring& wstr) {
        if (wstr.empty()) return std::string();
        int sizeNeeded = WideCharToMultiByte(
            CP_UTF8, 0,
            wstr.c_str(), (int)wstr.size(),
            NULL, 0, NULL, NULL
        );
        std::string result(sizeNeeded, 0);
        WideCharToMultiByte(
            CP_UTF8, 0,
            wstr.c_str(), (int)wstr.size(),
            &result[0], sizeNeeded, NULL, NULL
        );
        return result;
    }

    // Convert UTF-8 std::string → std::wstring
    inline std::wstring utf8ToWstring(const std::string& str) {
        if (str.empty()) return std::wstring();
        int sizeNeeded = MultiByteToWideChar(
            CP_UTF8, 0,
            str.c_str(), (int)str.size(),
            NULL, 0
        );
        std::wstring result(sizeNeeded, 0);
        MultiByteToWideChar(
            CP_UTF8, 0,
            str.c_str(), (int)str.size(),
            &result[0], sizeNeeded
        );
        return result;
    }

}
