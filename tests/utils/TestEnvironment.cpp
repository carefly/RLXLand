#include "TestEnvironment.h"
#include "mocks/MockLeviLaminaAPI.h"
#include <filesystem>
#include <iostream>

namespace rlx_land::test {

TestEnvironment& TestEnvironment::getInstance() {
    static TestEnvironment instance;
    return instance;
}

void TestEnvironment::initialize() {
    if (m_initialized) {
        return;
    }

    // 设置测试数据路径
    m_testDataPath = std::filesystem::current_path().string() + "/tests/fixtures";
    m_tempDataPath = std::filesystem::current_path().string() + "/tests/temp";

    // 创建必要的目录
    std::filesystem::create_directories(m_testDataPath);
    std::filesystem::create_directories(m_tempDataPath);

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
    mock::MockLeviLaminaAPI::clearMockPlayers();

    m_initialized = false;
    std::cout << "Test environment cleaned up." << std::endl;
}

std::string TestEnvironment::getTestDataPath() const { return m_testDataPath; }

std::string TestEnvironment::getTempDataPath() const { return m_tempDataPath; }

void TestEnvironment::setupMockLeviLamina() {
    // 清理之前的模拟数据
    mock::MockLeviLaminaAPI::clearMockPlayers();

    // 添加一些基本的模拟用户
    mock::MockLeviLaminaAPI::addMockPlayer("system", "System");
    mock::MockLeviLaminaAPI::addMockPlayer("admin", "Admin");
}

void TestEnvironment::setupMockServer() {
    // 模拟服务器环境设置
    // 这里可以设置必要的模拟服务器配置
}

} // namespace rlx_land::test