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

    // 边界值测试数据
    std::vector<std::unique_ptr<LandInformation>> loadBoundaryTestData();
    std::vector<std::unique_ptr<LandInformation>> loadInvalidTestData();
    std::vector<std::unique_ptr<LandInformation>> loadPerformanceTestData(int count = 1000);

    // 创建测试数据
    std::unique_ptr<LandInformation> createTestLand(const std::string& ownerXuid, int x, int z);
    std::unique_ptr<LandInformation>
    createBoundaryTestLand(const std::string& ownerXuid, int x, int z, int x_end, int z_end);
    std::unique_ptr<LandInformation>
    createInvalidTestLand(const std::string& ownerXuid, int x, int z, int x_end, int z_end);
    std::unique_ptr<BaseInformation> createTestTown(const std::string& ownerXuid, int x, int z);

    // 清理测试数据
    void cleanupTestData();

    // 获取最大ID
    LONG64 getMaxId() const;

    // 测试数据验证
    bool validateTestData(const std::vector<std::unique_ptr<LandInformation>>& lands);
    bool validateBoundaryData(const std::vector<std::unique_ptr<LandInformation>>& lands);

private:
    TestDataLoader() = default;
    std::vector<std::unique_ptr<BaseInformation>> m_testData;

    // 辅助方法
    void addBoundaryLands(std::vector<std::unique_ptr<LandInformation>>& lands);
    void addInvalidLands(std::vector<std::unique_ptr<LandInformation>>& lands);
    void addPerformanceLands(std::vector<std::unique_ptr<LandInformation>>& lands, int count);
};

} // namespace rlx_land::test