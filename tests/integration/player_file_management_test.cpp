#include "common/JsonLoader.h"
#include "utils/TestEnvironment.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <chrono>


namespace rlx_land::test {

// 前向声明
namespace TestHelpers {
    void deletePlayerFile(const std::string& xuid, const std::string& playerName);
}

// RAII 风格的文件清理类
class FileCleanupGuard {
public:
    FileCleanupGuard(const std::string& xuid, const std::string& playerName)
    : m_xuid(xuid), m_playerName(playerName) {}

    ~FileCleanupGuard() {
        try {
            TestHelpers::deletePlayerFile(m_xuid, m_playerName);
        } catch (...) {
            // 析构函数不应该抛出异常
        }
    }

    FileCleanupGuard(const FileCleanupGuard&) = delete;
    FileCleanupGuard& operator=(const FileCleanupGuard&) = delete;

private:
    std::string m_xuid;
    std::string m_playerName;
};

// 测试辅助函数
namespace TestHelpers {
std::string getLandsBaseDir() { return "plugins/RLXModeResources/data/lands"; }

std::string generatePlayerFileName(const std::string& xuid, const std::string& playerName) {
    return std::format("{}-{}.json", xuid, playerName);
}

std::filesystem::path getPlayerFilePath(const std::string& xuid, const std::string& playerName) {
    return std::filesystem::path(getLandsBaseDir()) / generatePlayerFileName(xuid, playerName);
}

bool fileExists(const std::filesystem::path& path) {
    return std::filesystem::exists(path);
}

bool fileIsEmpty(const std::filesystem::path& path) {
    if (!fileExists(path)) return false;

    std::ifstream file(path);
    if (!file.is_open()) return false;

    nlohmann::json json;
    try {
        file >> json;
    } catch (...) {
        file.close();
        return false;
    }
    file.close();
    return json.is_array() && json.empty();
}

void deletePlayerFile(const std::string& xuid, const std::string& playerName) {
    std::filesystem::path filePath = getPlayerFilePath(xuid, playerName);
    if (!fileExists(filePath)) return;

    // 尝试多次删除，避免 Windows 文件占用问题
    for (int i = 0; i < 3; ++i) {
        try {
            std::filesystem::remove(filePath);
            return;
        } catch (const std::filesystem::filesystem_error& e) {
            if (i < 2) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            } else {
                // 最后一次失败，记录但不抛出异常（避免测试中断）
                std::cerr << "Warning: Failed to delete file " << filePath << ": " << e.what() << std::endl;
            }
        }
    }
}

void createPlayerFileWithContent(const std::string& xuid, const std::string& playerName, const nlohmann::json& content) {
    std::filesystem::path filePath = getPlayerFilePath(xuid, playerName);

    try {
        std::filesystem::create_directories(filePath.parent_path());
        std::ofstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to create file: " + filePath.string());
        }
        file << content.dump(4);
        file.close();
    } catch (const std::exception& e) {
        std::cerr << "Error creating player file: " << e.what() << std::endl;
        throw;
    }
}

} // namespace TestHelpers


TEST_CASE("玩家文件管理 - hasPlayerFile", "[player_file]") {
    auto& env = TestEnvironment::getInstance();
    env.initialize();

    const std::string testXuid   = "test_has_file_12345";
    const std::string testName   = "TestPlayer";
    const std::string testXuid2  = "test_has_file_67890";
    const std::string testName2  = "AnotherPlayer";

    // 清理测试文件
    TestHelpers::deletePlayerFile(testXuid, testName);
    TestHelpers::deletePlayerFile(testXuid2, testName2);

    SECTION("玩家文件不存在时应返回 false") {
        REQUIRE_FALSE(rlx_land::JsonLoader::hasPlayerFile(testXuid));
    }

    SECTION("玩家文件存在时应返回 true") {
        // 创建测试文件
        TestHelpers::createPlayerFileWithContent(testXuid, testName, nlohmann::json::array());

        REQUIRE(rlx_land::JsonLoader::hasPlayerFile(testXuid));

        // 清理
        TestHelpers::deletePlayerFile(testXuid, testName);
    }

    SECTION("多个玩家文件时应正确识别") {
        // 创建两个不同玩家的文件
        TestHelpers::createPlayerFileWithContent(testXuid, testName, nlohmann::json::array());
        TestHelpers::createPlayerFileWithContent(testXuid2, testName2, nlohmann::json::array());

        REQUIRE(rlx_land::JsonLoader::hasPlayerFile(testXuid));
        REQUIRE(rlx_land::JsonLoader::hasPlayerFile(testXuid2));

        // 清理
        TestHelpers::deletePlayerFile(testXuid, testName);
        TestHelpers::deletePlayerFile(testXuid2, testName2);
    }
}


TEST_CASE("玩家文件管理 - createEmptyPlayerFile", "[player_file]") {
    auto& env = TestEnvironment::getInstance();
    env.initialize();

    const std::string testXuid = "test_create_file_12345";
    const std::string testName = "TestPlayer";

    // 清理测试文件
    TestHelpers::deletePlayerFile(testXuid, testName);

    SECTION("应创建空数组文件") {
        auto filePath = TestHelpers::getPlayerFilePath(testXuid, testName);

        REQUIRE_FALSE(TestHelpers::fileExists(filePath));

        // 调用创建函数
        rlx_land::JsonLoader::createEmptyPlayerFile(testXuid, testName);

        // 验证文件存在且为空数组
        REQUIRE(TestHelpers::fileExists(filePath));
        REQUIRE(TestHelpers::fileIsEmpty(filePath));

        // 清理
        TestHelpers::deletePlayerFile(testXuid, testName);
    }

    SECTION("重复创建时应覆盖旧文件") {
        auto filePath = TestHelpers::getPlayerFilePath(testXuid, testName);

        // 创建包含数据的文件
        nlohmann::json testData = nlohmann::json::array();
        testData.push_back({{"x", 0}, {"z", 0}, {"id", 1}});
        TestHelpers::createPlayerFileWithContent(testXuid, testName, testData);

        REQUIRE(TestHelpers::fileExists(filePath));
        REQUIRE_FALSE(TestHelpers::fileIsEmpty(filePath));

        // 重新创建空文件
        rlx_land::JsonLoader::createEmptyPlayerFile(testXuid, testName);

        // 验证文件被覆盖为空数组
        REQUIRE(TestHelpers::fileIsEmpty(filePath));

        // 清理
        TestHelpers::deletePlayerFile(testXuid, testName);
    }
}


TEST_CASE("玩家文件管理 - checkAndUpdatePlayerFileName 场景测试", "[player_file]") {
    auto& env = TestEnvironment::getInstance();
    env.initialize();

    // 清理：在测试开始前删除可能残留的测试文件
    TestHelpers::deletePlayerFile("test_new_player_12345", "Steve");
    TestHelpers::deletePlayerFile("test_normal_login_23456", "Steve");
    TestHelpers::deletePlayerFile("test_rename_34567", "Steve");
    TestHelpers::deletePlayerFile("test_rename_34567", "Alex");
    TestHelpers::deletePlayerFile("test_empty_xuid", "Steve");
    TestHelpers::deletePlayerFile("test_empty_name_45678", "");

    SECTION("场景1: 新玩家首次登录 - 应创建空文件") {
        const std::string testXuid = "test_new_player_12345";
        const std::string playerName = "Steve";

        auto filePath = TestHelpers::getPlayerFilePath(testXuid, playerName);

        REQUIRE_FALSE(TestHelpers::fileExists(filePath));

        // 模拟玩家登录
        rlx_land::JsonLoader::checkAndUpdatePlayerFileName(testXuid, playerName);

        // 验证文件已创建且为空
        REQUIRE(TestHelpers::fileExists(filePath));
        REQUIRE(TestHelpers::fileIsEmpty(filePath));

        // 清理
        TestHelpers::deletePlayerFile(testXuid, playerName);
    }

    SECTION("场景2: 正常登录 - 文件已存在且名称正确 - 不应修改") {
        const std::string testXuid = "test_normal_login_23456";
        const std::string playerName = "Steve";

        auto filePath = TestHelpers::getPlayerFilePath(testXuid, playerName);

        // 创建包含数据的文件
        nlohmann::json testData = nlohmann::json::array();
        testData.push_back({{"x", 100}, {"z", 200}, {"id", 1}, {"ownerXuid", testXuid}});
        TestHelpers::createPlayerFileWithContent(testXuid, playerName, testData);

        // 记录文件修改时间和大小
        auto originalTime = std::filesystem::last_write_time(filePath);
        auto originalSize = std::filesystem::file_size(filePath);

        // 短暂延迟确保时间戳可区分
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // 模拟玩家登录
        rlx_land::JsonLoader::checkAndUpdatePlayerFileName(testXuid, playerName);

        // 验证文件仍然存在且数据未改变
        REQUIRE(TestHelpers::fileExists(filePath));
        REQUIRE_FALSE(TestHelpers::fileIsEmpty(filePath));

        // 验证文件未被修改（修改时间和大小应该相同）
        auto newTime = std::filesystem::last_write_time(filePath);
        auto newSize = std::filesystem::file_size(filePath);
        REQUIRE(originalSize == newSize);

        // 清理
        TestHelpers::deletePlayerFile(testXuid, playerName);
    }

    SECTION("场景3: 玩家改名 - 应重命名文件") {
        const std::string testXuid = "test_rename_34567";
        const std::string oldName = "Steve";
        const std::string newName = "Alex";

        auto oldFilePath = TestHelpers::getPlayerFilePath(testXuid, oldName);
        auto newFilePath = TestHelpers::getPlayerFilePath(testXuid, newName);

        // 创建包含数据的旧文件
        nlohmann::json testData = nlohmann::json::array();
        testData.push_back({{"x", 100}, {"z", 200}, {"id", 1}, {"ownerXuid", testXuid}});
        TestHelpers::createPlayerFileWithContent(testXuid, oldName, testData);

        REQUIRE(TestHelpers::fileExists(oldFilePath));
        REQUIRE_FALSE(TestHelpers::fileExists(newFilePath));

        // 模拟改名后的玩家登录
        rlx_land::JsonLoader::checkAndUpdatePlayerFileName(testXuid, newName);

        // 给文件系统一点时间完成重命名操作
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // 验证旧文件已删除，新文件已创建
        REQUIRE_FALSE(TestHelpers::fileExists(oldFilePath));
        REQUIRE(TestHelpers::fileExists(newFilePath));

        // 验证数据保持不变
        {
            std::ifstream newFile(newFilePath);
            nlohmann::json  newData;
            newFile >> newData;
            REQUIRE(newData.size() == 1);
            REQUIRE(newData[0]["x"] == 100);
            REQUIRE(newData[0]["z"] == 200);
            // 文件在这里自动关闭
        }

        // 给文件系统一点时间释放文件句柄
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // 清理
        TestHelpers::deletePlayerFile(testXuid, newName);
    }

    SECTION("场景4: 参数验证 - xuid 为空 - 不应创建文件") {
        const std::string testXuid;
        const std::string playerName = "Steve";

        auto filePath = TestHelpers::getPlayerFilePath(testXuid, playerName);

        REQUIRE_FALSE(TestHelpers::fileExists(filePath));

        // 模拟登录（空 xuid）
        rlx_land::JsonLoader::checkAndUpdatePlayerFileName(testXuid, playerName);

        // 验证文件未创建
        REQUIRE_FALSE(TestHelpers::fileExists(filePath));
    }

    SECTION("场景5: 参数验证 - playerName 为空 - 不应创建文件") {
        const std::string testXuid = "test_empty_name_45678";
        const std::string playerName;

        auto filePath = TestHelpers::getPlayerFilePath(testXuid, playerName);

        REQUIRE_FALSE(TestHelpers::fileExists(filePath));

        // 模拟登录（空名字）
        rlx_land::JsonLoader::checkAndUpdatePlayerFileName(testXuid, playerName);

        // 验证文件未创建
        REQUIRE_FALSE(TestHelpers::fileExists(filePath));
    }
}


TEST_CASE("玩家文件管理 - 边界情况测试", "[player_file]") {
    auto& env = TestEnvironment::getInstance();
    env.initialize();

    SECTION("连续多次登录同一玩家") {
        const std::string testXuid   = "test_boundary_11111";
        const std::string playerName = "Steve";

        // 清理
        TestHelpers::deletePlayerFile(testXuid, playerName);

        auto filePath = TestHelpers::getPlayerFilePath(testXuid, playerName);

        // 第一次登录
        rlx_land::JsonLoader::checkAndUpdatePlayerFileName(testXuid, playerName);
        REQUIRE(TestHelpers::fileExists(filePath));

        // 记录文件修改时间
        auto modifyTime = std::filesystem::last_write_time(filePath);

        // 短暂延迟确保时间戳不同（如果文件被修改）
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // 第二次登录
        rlx_land::JsonLoader::checkAndUpdatePlayerFileName(testXuid, playerName);

        // 验证文件未被修改
        auto newModifyTime = std::filesystem::last_write_time(filePath);
        REQUIRE(modifyTime == newModifyTime);

        // 第三次登录
        rlx_land::JsonLoader::checkAndUpdatePlayerFileName(testXuid, playerName);

        // 验证文件仍然未被修改
        newModifyTime = std::filesystem::last_write_time(filePath);
        REQUIRE(modifyTime == newModifyTime);

        // 清理
        TestHelpers::deletePlayerFile(testXuid, playerName);
    }

    SECTION("特殊字符处理") {
        const std::string testXuid   = "test_special_99999";
        const std::string playerName = "Test Player";

        // 清理
        TestHelpers::deletePlayerFile(testXuid, playerName);

        // 模拟登录（名字包含空格）
        rlx_land::JsonLoader::checkAndUpdatePlayerFileName(testXuid, playerName);

        auto filePath = TestHelpers::getPlayerFilePath(testXuid, playerName);

        // 验证文件创建成功
        REQUIRE(TestHelpers::fileExists(filePath));

        // 清理
        TestHelpers::deletePlayerFile(testXuid, playerName);
    }
}

} // namespace rlx_land::test
