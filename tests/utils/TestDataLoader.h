#pragma once
#include "data/core/BaseInformation.h"
#include "data/land/LandCore.h"
#include <memory>
#include <vector>


namespace rlx_land::test {

class TestDataLoader {
public:
    static TestDataLoader& getInstance();

    // 加载测试数据
    std::vector<std::unique_ptr<LandInformation>> loadLandTestData();
    std::vector<std::unique_ptr<BaseInformation>> loadTownTestData();

    // 创建测试数据
    std::unique_ptr<LandInformation> createTestLand(const std::string& ownerXuid, int x, int z);
    std::unique_ptr<BaseInformation> createTestTown(const std::string& ownerXuid, int x, int z);

    // 清理测试数据
    void cleanupTestData();

    // 获取最大ID
    LONG64 getMaxId() const;

private:
    TestDataLoader() = default;
    std::vector<std::unique_ptr<BaseInformation>> m_testData;
};

} // namespace rlx_land::test