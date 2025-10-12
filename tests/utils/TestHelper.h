#pragma once
#include "data/core/BaseInformation.h"
#include "data/land/LandCore.h"
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>



namespace rlx_land::test {

// 边界值测试结构
struct BoundaryTestValues {
    int    minCoordinate;
    int    maxCoordinate;
    int    minSize;
    int    maxSize;
    LONG64 minId;
    LONG64 maxId;
    int    minPermission;
    int    maxPermission;
};

// 测试异常类型
enum class TestExceptionType {
    PlayerNotFound,
    LandNotFound,
    Duplicate,
    NotMember,
    InvalidData,
    OutOfBounds,
    NullPointer
};

class TestHelper {
public:
    // 测试数据生成
    static std::string generateTestXuid();
    static std::string generateTestName();
    static std::string generateInvalidXuid();             // 生成无效XUID
    static std::string generateEmptyString();             // 生成空字符串
    static std::string generateLongString(size_t length); // 生成长字符串

    // 边界值生成
    static BoundaryTestValues getBoundaryValues();
    static int                generateBoundaryCoordinate(bool isMin = false);
    static int                generateBoundarySize(bool isMin = false);
    static LONG64             generateBoundaryId(bool isMin = false);

    // 文件操作辅助
    static bool        createTestFile(const std::string& path, const std::string& content);
    static std::string readTestFile(const std::string& path);
    static void        cleanupTestFile(const std::string& path);
    static bool        createCorruptedFile(const std::string& path); // 创建损坏文件

    // 测试断言辅助
    static void assertLandDataEqual(const BaseData& expected, const BaseData& actual);
    static void assertVectorContains(const std::vector<std::string>& vec, const std::string& value);
    static void assertVectorNotContains(const std::vector<std::string>& vec, const std::string& value);
    static void assertThrows(std::function<void()> func, TestExceptionType expectedType);
    static void assertLandDataValid(const LandInformation& land);
    static void assertLandDataInvalid(const LandInformation& land);

    // 性能测试辅助
    static void measureExecutionTime(const std::string& testName, std::function<void()> func);
    static void measureMemoryUsage(const std::string& testName, std::function<void()> func);
    static void performStressTest(const std::string& testName, int iterations, std::function<void()> func);

    // 内存泄漏检测辅助
    static void   trackMemoryUsage();
    static void   reportMemoryUsage();
    static size_t getCurrentMemoryUsage();

    // 并发测试辅助
    static void runConcurrentTest(const std::string& testName, int numThreads, std::function<void(int)> func);
    static void testThreadSafety(std::function<void()> sharedResourceOperation);

    // 数据验证辅助
    static bool isValidXuid(const std::string& xuid);
    static bool isValidCoordinate(int coord);
    static bool isValidSize(int size);
    static bool isValidPermission(int perm);

private:
    static size_t      s_initialMemoryUsage;
    static std::string getExceptionMessage(TestExceptionType type);
};

} // namespace rlx_land::test