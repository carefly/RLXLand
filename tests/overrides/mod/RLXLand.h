#pragma once

#include <iostream>
#include <string>

namespace rlx_land {

// 模拟的 Logger 类，用于测试环境
class MockLogger {
public:
    void debug(const std::string& msg) { std::cout << "[DEBUG] " << msg << std::endl; }

    void info(const std::string& msg) { std::cout << "[INFO] " << msg << std::endl; }

    void warn(const std::string& msg) { std::cout << "[WARN] " << msg << std::endl; }

    void error(const std::string& msg) { std::cerr << "[ERROR] " << msg << std::endl; }

    // 支持格式化参数的版本
    template <typename... Args>
    void error(const std::string& format, Args... args) {
        (void)(sizeof...(args)); // 避免未使用参数警告
        std::cerr << "[ERROR] " << format << std::endl;
    }

    template <typename... Args>
    void info(const std::string& format, Args... args) {
        (void)(sizeof...(args)); // 避免未使用参数警告
        std::cout << "[INFO] " << format << std::endl;
    }

    template <typename... Args>
    void debug(const std::string& format, Args... args) {
        (void)(sizeof...(args)); // 避免未使用参数警告
        std::cout << "[DEBUG] " << format << std::endl;
    }
};

// 模拟的 NativeMod 类
class MockNativeMod {
private:
    MockLogger mLogger;

public:
    MockLogger& getLogger() { return mLogger; }
};

class RLXLand {
private:
    MockNativeMod mSelf;

public:
    static RLXLand& getInstance();

    RLXLand() = default;

    [[nodiscard]] MockNativeMod& getSelf() const { return const_cast<MockNativeMod&>(mSelf); }

    bool load() { return true; }
    bool enable() { return true; }
    bool disable() { return true; }
};

} // namespace rlx_land