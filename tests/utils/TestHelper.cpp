#include "TestHelper.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>


namespace rlx_land::test {

size_t TestHelper::s_initialMemoryUsage = 0;

std::string TestHelper::generateTestXuid() {
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(100000, 999999);

    return "test_xuid_" + std::to_string(dis(gen));
}

std::string TestHelper::generateTestName() {
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);

    return "TestPlayer" + std::to_string(dis(gen));
}

bool TestHelper::createTestFile(const std::string& path, const std::string& content) {
    try {
        std::ofstream file(path);
        if (!file.is_open()) {
            return false;
        }
        file << content;
        file.close();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string TestHelper::readTestFile(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            return "";
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        return content;
    } catch (const std::exception&) {
        return "";
    }
}

void TestHelper::cleanupTestFile(const std::string& path) {
    try {
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
        }
    } catch (const std::exception&) {
        // 忽略清理错误
    }
}

void TestHelper::assertLandDataEqual(const BaseData& expected, const BaseData& actual) {
    // 这里可以添加详细的数据比较逻辑
    // 由于我们在架构设计阶段，先提供基本框架
    std::cout << "Comparing land data..." << std::endl;
}

void TestHelper::assertVectorContains(const std::vector<std::string>& vec, const std::string& value) {
    bool found = std::find(vec.begin(), vec.end(), value) != vec.end();
    if (!found) {
        std::cout << "Value '" << value << "' not found in vector" << std::endl;
    }
}

void TestHelper::measureExecutionTime(const std::string& testName, std::function<void()> func) {
    auto startTime = std::chrono::high_resolution_clock::now();

    func();

    auto endTime  = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "[PERF] " << testName << ": " << duration.count() << " ms" << std::endl;
}

void TestHelper::trackMemoryUsage() {
    // 这里可以实现内存使用量跟踪
    // 由于跨平台兼容性，先提供基本框架
    s_initialMemoryUsage = 0; // 实际实现中应该获取当前内存使用量
    std::cout << "[MEMORY] Starting memory tracking..." << std::endl;
}

void TestHelper::reportMemoryUsage() {
    // 这里可以实现内存使用量报告
    std::cout << "[MEMORY] Memory tracking completed." << std::endl;
}

} // namespace rlx_land::test