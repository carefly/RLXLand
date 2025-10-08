#include "TestDataLoader.h"
#include "TestEnvironment.h"
#include "mocks/MockLeviLaminaAPI.h"
#include <fstream>
#include <iostream>

namespace rlx_land::test {

TestDataLoader& TestDataLoader::getInstance() {
    static TestDataLoader instance;
    return instance;
}

std::vector<std::unique_ptr<LandInformation>> TestDataLoader::loadLandTestData() {
    std::vector<std::unique_ptr<LandInformation>> lands;

    // 由于我们在架构设计阶段，先创建一些基本的测试数据
    // 实际实现中应该从JSON文件加载

    // 创建测试土地1
    {
        LandData landData;
        landData.id          = 1001;
        landData.ownerXuid   = "test_owner_001";
        landData.x           = 1000;
        landData.z           = 1000;
        landData.dx          = 50;
        landData.dz          = 50;
        landData.d           = 0;
        landData.perm        = 3;
        landData.description = "Test land for basic functionality";

        // 为测试用户添加模拟名称
        mock::MockLeviLaminaAPI::addMockPlayer("test_owner_001", "TestOwnerOne");
        mock::MockLeviLaminaAPI::addMockPlayer("test_member_001", "TestMemberOne");

        auto landInfo       = std::make_unique<LandInformation>(landData);
        landInfo->ownerName = "TestOwnerOne";
        landInfo->data.memberXuids.push_back("test_member_001");

        lands.push_back(std::move(landInfo));
    }

    // 创建测试土地2
    {
        LandData landData;
        landData.id          = 1002;
        landData.ownerXuid   = "test_owner_002";
        landData.x           = 2000;
        landData.z           = 2000;
        landData.dx          = 100;
        landData.dz          = 100;
        landData.d           = 0;
        landData.perm        = 5;
        landData.description = "Large test land for performance testing";

        // 为测试用户添加模拟名称
        mock::MockLeviLaminaAPI::addMockPlayer("test_owner_002", "TestOwnerTwo");

        auto landInfo       = std::make_unique<LandInformation>(landData);
        landInfo->ownerName = "TestOwnerTwo";

        lands.push_back(std::move(landInfo));
    }

    return lands;
}

std::vector<std::unique_ptr<BaseInformation>> TestDataLoader::loadTownTestData() {
    std::vector<std::unique_ptr<BaseInformation>> towns;

    // 城镇测试数据的实现
    // 由于架构设计阶段，先返回空列表

    return towns;
}

std::unique_ptr<LandInformation> TestDataLoader::createTestLand(const std::string& ownerXuid, int x, int z) {
    LandData landData;
    landData.id          = getMaxId() + 1;
    landData.ownerXuid   = ownerXuid;
    landData.x           = x;
    landData.z           = z;
    landData.dx          = 50; // 默认大小
    landData.dz          = 50;
    landData.d           = 0; // 默认维度
    landData.perm        = 3; // 默认权限级别
    landData.description = "Auto-generated test land";

    // 为测试用户添加模拟名称
    mock::MockLeviLaminaAPI::addMockPlayer(ownerXuid, "TestUser_" + ownerXuid);

    auto landInfo = std::make_unique<LandInformation>(landData);
    // ownerName 会在LandInformation构造函数中通过模拟API设置

    // 返回存储在m_testData中的对象的副本
    auto returnLand = std::make_unique<LandInformation>(landData);
    m_testData.push_back(std::move(landInfo));
    return returnLand;
}

std::unique_ptr<BaseInformation> TestDataLoader::createTestTown(const std::string& ownerXuid, int x, int z) {
    // 城镇创建的实现
    // 由于架构设计阶段，先返回nullptr
    return nullptr;
}

void TestDataLoader::cleanupTestData() { m_testData.clear(); }

LONG64 TestDataLoader::getMaxId() const {
    LONG64 maxId = 0;
    for (const auto& data : m_testData) {
        if (data->data.id > maxId) {
            maxId = data->data.id;
        }
    }
    return maxId;
}

} // namespace rlx_land::test