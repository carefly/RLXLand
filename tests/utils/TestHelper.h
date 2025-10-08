#pragma once
#include "data/core/BaseInformation.h"
#include <functional>
#include <string>
#include <vector>


namespace rlx_land::test {

class TestHelper {
public:
    // 测试数据生成
    static std::string generateTestXuid();
    static std::string generateTestName();

    // 文件操作辅助
    static bool        createTestFile(const std::string& path, const std::string& content);
    static std::string readTestFile(const std::string& path);
    static void        cleanupTestFile(const std::string& path);

    // 测试断言辅助
    static void assertLandDataEqual(const BaseData& expected, const BaseData& actual);
    static void assertVectorContains(const std::vector<std::string>& vec, const std::string& value);

    // 性能测试辅助
    static void measureExecutionTime(const std::string& testName, std::function<void()> func);

    // 内存泄漏检测辅助
    static void trackMemoryUsage();
    static void reportMemoryUsage();

private:
    static size_t s_initialMemoryUsage;
};

} // namespace rlx_land::test