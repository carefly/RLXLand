#pragma once
#include <string>


namespace rlx_land::test {

class TestEnvironment {
public:
    static TestEnvironment& getInstance();

    void initialize();
    void cleanup();

    // 测试数据路径管理
    std::string getTestDataPath() const;
    std::string getTempDataPath() const;

    // 模拟环境设置
    void setupMockLeviLamina();
    void setupMockServer();

    // 清理方法
    void cleanupDefaultLandsFolder();

private:
    TestEnvironment() = default;
    std::string m_testDataPath;
    std::string m_tempDataPath;
    bool        m_initialized = false;
};

} // namespace rlx_land::test