#pragma once
#include <string>
#include <random>

namespace IdUtils {
    inline std::string generateShortId(size_t length = 8) {
        static const char chars[] =
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        std::string id;
        id.reserve(length);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, sizeof(chars) - 2);

        for (size_t i = 0; i < length; ++i) {
            id += chars[dis(gen)];
        }
        return id;
    }
}
