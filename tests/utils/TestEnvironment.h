#pragma once
#include "data/core/DataLoaderTraits.h"
#include <basetsd.h>
#include <string>
#include <utility>



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
    void resetAllTestData(); // 完整重置所有测试数据

    // 测试辅助方法 - 获取Land或Town的中心坐标
    template <typename T>
    std::pair<LONG64, LONG64> getItemCenter(typename DataLoaderTraits<T>::InfoType* item);

private:
    TestEnvironment() = default;
    std::string m_testDataPath;
    std::string m_tempDataPath;
    bool        m_initialized = false;
};

} // namespace rlx_land::test