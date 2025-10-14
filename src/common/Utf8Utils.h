#pragma once

#include <fstream>
#include <locale>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace rlx_land {

class Utf8Utils {
public:
    // 设置UTF-8环境（在程序启动时调用）
    static void setUtf8Environment() {
#ifdef _WIN32
        // Windows系统设置UTF-8
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
#endif
        // 设置locale为UTF-8
        try {
            std::locale::global(std::locale("en_US.UTF-8"));
        } catch (const std::exception&) {
            // 如果设置失败，使用系统默认locale
        }
    }

    // 创建UTF-8编码的输入文件流
    static std::ifstream createUtf8InputStream(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (file.is_open()) {
            // 设置UTF-8 locale
            try {
                file.imbue(std::locale("en_US.UTF-8"));
            } catch (const std::exception&) {
                // 如果设置失败，使用默认locale
            }
        }
        return file;
    }

    // 创建UTF-8编码的输出文件流
    static std::ofstream createUtf8OutputStream(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (file.is_open()) {
            // 设置UTF-8 locale
            try {
                file.imbue(std::locale("en_US.UTF-8"));
            } catch (const std::exception&) {
                // 如果设置失败，使用默认locale
            }
        }
        return file;
    }
};

} // namespace rlx_land