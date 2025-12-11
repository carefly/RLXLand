#include "TestEnvironment.h"
#include "common/Utf8Utils.h"
#include "overrides/common/LeviLaminaAPI.h"
#include <filesystem>
#include <iostream>


namespace rlx_land::test {

using rlx_town::TownData;
using rlx_town::TownInformation;

TestEnvironment& TestEnvironment::getInstance() {
    static TestEnvironment instance;
    return instance;
}

void TestEnvironment::initialize() {
    if (m_initialized) {
        return;
    }
    // 设置UTF-8环境以解决中文乱码问题
    Utf8Utils::setUtf8Environment();

    std::cout << "Start initialize test environment." << std::endl;

    // 设置测试数据路径
    m_testDataPath = std::filesystem::current_path().string() + "/tests/fixtures";
    m_tempDataPath = std::filesystem::current_path().string() + "/tests/temp";

    // 创建必要的目录
    std::filesystem::create_directories(m_testDataPath);
    std::filesystem::create_directories(m_tempDataPath);

    // 清理默认lands文件夹数据
    cleanupDefaultLandsFolder();

    // 设置模拟环境
    setupMockLeviLamina();
    setupMockServer();

    m_initialized = true;
    std::cout << "Test environment initialized." << std::endl;
}

void TestEnvironment::cleanup() {
    if (!m_initialized) {
        return;
    }

    // 清理临时文件
    try {
        if (std::filesystem::exists(m_tempDataPath)) {
            std::filesystem::remove_all(m_tempDataPath);
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to cleanup temp files: " << e.what() << std::endl;
    }

    // 清理模拟数据
    LeviLaminaAPI::clearMockPlayers();

    m_initialized = false;
    std::cout << "Test environment cleaned up." << std::endl;
}

std::string TestEnvironment::getTestDataPath() const { return m_testDataPath; }

std::string TestEnvironment::getTempDataPath() const { return m_tempDataPath; }

void TestEnvironment::setupMockLeviLamina() {
    // 清理之前的模拟数据
    LeviLaminaAPI::clearMockPlayers();

    // 添加一些基本的模拟用户
    LeviLaminaAPI::addMockPlayer("system", "System");
    LeviLaminaAPI::addMockPlayer("admin", "Admin");
}

void TestEnvironment::setupMockServer() {
    // 模拟服务器环境设置
    // 这里可以设置必要的模拟服务器配置
}

void TestEnvironment::cleanupDefaultLandsFolder() {
    // 清理默认的lands文件夹数据
    std::string landsDir = "../RLXModeResources/data/lands";

    try {
        if (std::filesystem::exists(landsDir)) {
            std::filesystem::remove_all(landsDir);
            std::cout << "Cleaned up default lands folder: " << landsDir << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to cleanup default lands folder: " << e.what() << std::endl;
    }
}

template <typename T>
std::pair<LONG64, LONG64> TestEnvironment::getItemCenter(typename DataLoaderTraits<T>::InfoType* item) {
    if (item == nullptr) {
        return {0, 0};
    }
    // 返回中心坐标
    LONG64 centerX = (item->getX() + item->getXEnd()) / 2;
    LONG64 centerZ = (item->getZ() + item->getZEnd()) / 2;
    return {centerX, centerZ};
}

// 显式实例化
template std::pair<LONG64, LONG64> TestEnvironment::getItemCenter<LandData>(LandInformation* item);
template std::pair<LONG64, LONG64> TestEnvironment::getItemCenter<TownData>(TownInformation* item);

} // namespace rlx_land::test