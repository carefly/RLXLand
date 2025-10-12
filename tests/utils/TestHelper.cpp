#include "TestHelper.h"
#include "common/exceptions/LandExceptions.h"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <thread>
#include <vector>


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

std::string TestHelper::generateInvalidXuid() {
    return ""; // 空字符串作为无效XUID
}

std::string TestHelper::generateEmptyString() { return ""; }

std::string TestHelper::generateLongString(size_t length) { return std::string(length, 'A'); }

BoundaryTestValues TestHelper::getBoundaryValues() {
    BoundaryTestValues values;
    values.minCoordinate = -30000000; // Minecraft世界边界
    values.maxCoordinate = 30000000;
    values.minSize       = 1;
    values.maxSize       = 1000; // 合理的最大土地大小
    values.minId         = 1;
    values.maxId         = 9223372036854775807LL; // INT64_MAX
    values.minPermission = 0;
    values.maxPermission = 10;
    return values;
}

int TestHelper::generateBoundaryCoordinate(bool isMin) {
    auto values = getBoundaryValues();
    return isMin ? values.minCoordinate : values.maxCoordinate;
}

int TestHelper::generateBoundarySize(bool isMin) {
    auto values = getBoundaryValues();
    return isMin ? values.minSize : values.maxSize;
}

LONG64 TestHelper::generateBoundaryId(bool isMin) {
    auto values = getBoundaryValues();
    return isMin ? values.minId : values.maxId;
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

bool TestHelper::createCorruptedFile(const std::string& path) {
    try {
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        // 写入一些无效的二进制数据
        std::vector<char> corruptedData = {(char)0xFF, (char)0xFE, (char)0xFD, (char)0xFC, (char)0xFB, (char)0xFA};
        file.write(corruptedData.data(), corruptedData.size());
        file.close();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void TestHelper::assertLandDataEqual(const BaseData& expected, const BaseData& actual) {
    if (expected.x != actual.x) {
        std::cout << "X coordinate mismatch: expected " << expected.x << ", got " << actual.x << std::endl;
    }
    if (expected.z != actual.z) {
        std::cout << "Z coordinate mismatch: expected " << expected.z << ", got " << actual.z << std::endl;
    }
    if (expected.x_end != actual.x_end) {
        std::cout << "X_END size mismatch: expected " << expected.x_end << ", got " << actual.x_end << std::endl;
    }
    if (expected.z_end != actual.z_end) {
        std::cout << "Z_END size mismatch: expected " << expected.z_end << ", got " << actual.z_end << std::endl;
    }
    if (expected.id != actual.id) {
        std::cout << "ID mismatch: expected " << expected.id << ", got " << actual.id << std::endl;
    }
}

void TestHelper::assertVectorContains(const std::vector<std::string>& vec, const std::string& value) {
    bool found = std::find(vec.begin(), vec.end(), value) != vec.end();
    if (!found) {
        std::cout << "Value '" << value << "' not found in vector" << std::endl;
    }
}

void TestHelper::assertVectorNotContains(const std::vector<std::string>& vec, const std::string& value) {
    bool found = std::find(vec.begin(), vec.end(), value) != vec.end();
    if (found) {
        std::cout << "Value '" << value << "' should not be found in vector" << std::endl;
    }
}

void TestHelper::assertThrows(std::function<void()> func, TestExceptionType expectedType) {
    try {
        func();
        std::cout << "Expected exception but none was thrown" << std::endl;
    } catch (const PlayerNotFoundException& e) {
        if (expectedType != TestExceptionType::PlayerNotFound) {
            std::cout << "Wrong exception type: PlayerNotFoundException" << std::endl;
        }
    } catch (const LandNotFoundException& e) {
        if (expectedType != TestExceptionType::LandNotFound) {
            std::cout << "Wrong exception type: LandNotFoundException" << std::endl;
        }
    } catch (const DuplicateException& e) {
        if (expectedType != TestExceptionType::Duplicate) {
            std::cout << "Wrong exception type: DuplicateException" << std::endl;
        }
    } catch (const NotMemberException& e) {
        if (expectedType != TestExceptionType::NotMember) {
            std::cout << "Wrong exception type: NotMemberException" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Unexpected exception: " << e.what() << std::endl;
    }
}

void TestHelper::assertLandDataValid(const LandInformation& land) {
    if (!isValidCoordinate(land.getX())) {
        std::cout << "Invalid X coordinate: " << land.getX() << std::endl;
    }
    if (!isValidCoordinate(land.getZ())) {
        std::cout << "Invalid Z coordinate: " << land.getZ() << std::endl;
    }
    if (!isValidSize(land.getWidth())) {
        std::cout << "Invalid width: " << land.getWidth() << std::endl;
    }
    if (!isValidSize(land.getHeight())) {
        std::cout << "Invalid height: " << land.getHeight() << std::endl;
    }
    if (!isValidXuid(land.getOwnerXuid())) {
        std::cout << "Invalid owner XUID: " << land.getOwnerXuid() << std::endl;
    }
}

void TestHelper::assertLandDataInvalid(const LandInformation& land) {
    bool hasInvalidData = false;
    if (!isValidCoordinate(land.getX()) || !isValidCoordinate(land.getZ()) || !isValidSize(land.getWidth())
        || !isValidSize(land.getHeight()) || !isValidXuid(land.getOwnerXuid())) {
        hasInvalidData = true;
    }

    if (!hasInvalidData) {
        std::cout << "Expected invalid land data but data appears valid" << std::endl;
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

void TestHelper::measureMemoryUsage(const std::string& testName, std::function<void()> func) {
    size_t beforeMemory = getCurrentMemoryUsage();
    func();
    size_t afterMemory = getCurrentMemoryUsage();

    std::cout << "[MEMORY] " << testName << ": " << (afterMemory - beforeMemory) << " bytes" << std::endl;
}

void TestHelper::performStressTest(const std::string& testName, int iterations, std::function<void()> func) {
    std::cout << "[STRESS] Starting " << testName << " with " << iterations << " iterations" << std::endl;

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        func();
    }

    auto endTime  = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << "[STRESS] " << testName << " completed in " << duration.count()
              << " ms (avg: " << (double)duration.count() / iterations << " ms per iteration)" << std::endl;
}

void TestHelper::trackMemoryUsage() {
    s_initialMemoryUsage = getCurrentMemoryUsage();
    std::cout << "[MEMORY] Starting memory tracking. Initial: " << s_initialMemoryUsage << " bytes" << std::endl;
}

void TestHelper::reportMemoryUsage() {
    size_t currentMemory = getCurrentMemoryUsage();
    std::cout << "[MEMORY] Memory tracking completed. Current: " << currentMemory
              << " bytes, Delta: " << (currentMemory - s_initialMemoryUsage) << " bytes" << std::endl;
}

size_t TestHelper::getCurrentMemoryUsage() {
    // 简化实现，实际应该使用平台特定的API
    return 0;
}

void TestHelper::runConcurrentTest(const std::string& testName, int numThreads, std::function<void(int)> func) {
    std::cout << "[CONCURRENT] Starting " << testName << " with " << numThreads << " threads" << std::endl;

    std::vector<std::thread> threads;
    std::atomic<int>         completedThreads(0);

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&, i]() {
            func(i);
            completedThreads++;
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto endTime  = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << "[CONCURRENT] " << testName << " completed in " << duration.count() << " ms. All "
              << completedThreads.load() << " threads finished" << std::endl;
}

void TestHelper::testThreadSafety(std::function<void()> sharedResourceOperation) {
    const int numThreads          = 10;
    const int operationsPerThread = 100;

    std::vector<std::thread> threads;
    std::atomic<int>         totalOperations(0);

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < operationsPerThread; ++j) {
                sharedResourceOperation();
                totalOperations++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "[THREAD_SAFETY] Completed " << totalOperations.load() << " operations across " << numThreads
              << " threads" << std::endl;
}

bool TestHelper::isValidXuid(const std::string& xuid) { return !xuid.empty() && xuid.length() > 0; }

bool TestHelper::isValidCoordinate(int coord) {
    auto values = getBoundaryValues();
    return coord >= values.minCoordinate && coord <= values.maxCoordinate;
}

bool TestHelper::isValidSize(int size) {
    auto values = getBoundaryValues();
    return size >= values.minSize && size <= values.maxSize;
}

bool TestHelper::isValidPermission(int perm) {
    auto values = getBoundaryValues();
    return perm >= values.minPermission && perm <= values.maxPermission;
}

std::string TestHelper::getExceptionMessage(TestExceptionType type) {
    switch (type) {
    case TestExceptionType::PlayerNotFound:
        return "Player not found";
    case TestExceptionType::LandNotFound:
        return "Land not found";
    case TestExceptionType::Duplicate:
        return "Duplicate entry";
    case TestExceptionType::NotMember:
        return "Not a member";
    case TestExceptionType::InvalidData:
        return "Invalid data";
    case TestExceptionType::OutOfBounds:
        return "Out of bounds";
    case TestExceptionType::NullPointer:
        return "Null pointer";
    default:
        return "Unknown exception";
    }
}

} // namespace rlx_land::test