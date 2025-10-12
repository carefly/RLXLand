#include "TestDataLoader.h"
#include "TestHelper.h"
#include "data/town/TownCore.h"
#include "mocks/MockLeviLaminaAPI.h"
#include <random>
#include <stdexcept>
#include <string>


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
        landData.x_end       = 1050;
        landData.z_end       = 1050;
        landData.d           = 0;
        landData.perm        = 3;
        landData.description = "Test land for basic functionality";

        // 为测试用户添加模拟名称
        mock::MockLeviLaminaAPI::addMockPlayer("test_owner_001", "TestOwnerOne");
        mock::MockLeviLaminaAPI::addMockPlayer("test_member_001", "TestMemberOne");

        auto landInfo = std::make_unique<LandInformation>(landData);
        // ownerName 会在构造函数中自动设置
        landInfo->addMember("test_member_001");

        lands.push_back(std::move(landInfo));
    }

    // 创建测试土地2
    {
        LandData landData;
        landData.id          = 1002;
        landData.ownerXuid   = "test_owner_002";
        landData.x           = 2000;
        landData.z           = 2000;
        landData.x_end       = 2100;
        landData.z_end       = 2100;
        landData.d           = 0;
        landData.perm        = 5;
        landData.description = "Large test land for performance testing";

        // 为测试用户添加模拟名称
        mock::MockLeviLaminaAPI::addMockPlayer("test_owner_002", "TestOwnerTwo");

        auto landInfo = std::make_unique<LandInformation>(landData);
        // ownerName 会在构造函数中自动设置

        lands.push_back(std::move(landInfo));
    }

    return lands;
}

std::vector<std::unique_ptr<BaseInformation>> TestDataLoader::loadTownTestData() {
    std::vector<std::unique_ptr<BaseInformation>> towns;


    return towns;
}

std::vector<std::unique_ptr<LandInformation>> TestDataLoader::loadBoundaryTestData() {
    std::vector<std::unique_ptr<LandInformation>> lands;

    addBoundaryLands(lands);

    return lands;
}

std::vector<std::unique_ptr<LandInformation>> TestDataLoader::loadInvalidTestData() {
    std::vector<std::unique_ptr<LandInformation>> lands;

    addInvalidLands(lands);

    return lands;
}

std::vector<std::unique_ptr<LandInformation>> TestDataLoader::loadPerformanceTestData(int count) {
    std::vector<std::unique_ptr<LandInformation>> lands;

    addPerformanceLands(lands, count);

    return lands;
}

std::unique_ptr<LandInformation> TestDataLoader::createTestLand(const std::string& ownerXuid, int x, int z) {
    LandData landData;
    landData.id          = getMaxId() + 1;
    landData.ownerXuid   = ownerXuid;
    landData.x           = x;
    landData.z           = z;
    landData.x_end       = x + 50; // 默认大小
    landData.z_end       = z + 50;
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

std::unique_ptr<LandInformation>
TestDataLoader::createBoundaryTestLand(const std::string& ownerXuid, int x, int z, int x_end, int z_end) {
    LandData landData;
    landData.id          = getMaxId() + 1;
    landData.ownerXuid   = ownerXuid;
    landData.x           = x;
    landData.z           = z;
    landData.x_end       = x_end;
    landData.z_end       = z_end;
    landData.d           = 0;
    landData.perm        = 3;
    landData.description = "Boundary test land";

    mock::MockLeviLaminaAPI::addMockPlayer(ownerXuid, "BoundaryTest_" + ownerXuid);

    auto landInfo   = std::make_unique<LandInformation>(landData);
    auto returnLand = std::make_unique<LandInformation>(landData);
    m_testData.push_back(std::move(landInfo));
    return returnLand;
}

std::unique_ptr<LandInformation>
TestDataLoader::createInvalidTestLand(const std::string& ownerXuid, int x, int z, int x_end, int z_end) {
    LandData landData;
    landData.id          = getMaxId() + 1;
    landData.ownerXuid   = ownerXuid; // 可能无效
    landData.x           = x;         // 可能无效
    landData.z           = z;         // 可能无效
    landData.x_end       = x_end;     // 可能无效
    landData.z_end       = z_end;     // 可能无效
    landData.d           = 0;
    landData.perm        = 3;
    landData.description = "Invalid test land";

    if (!ownerXuid.empty()) {
        mock::MockLeviLaminaAPI::addMockPlayer(ownerXuid, "InvalidTest_" + ownerXuid);
    }

    auto landInfo   = std::make_unique<LandInformation>(landData);
    auto returnLand = std::make_unique<LandInformation>(landData);
    m_testData.push_back(std::move(landInfo));
    return returnLand;
}

std::unique_ptr<BaseInformation> TestDataLoader::createTestTown(const std::string& ownerXuid, int x, int z) {
    // 输入参数验证
    if (ownerXuid.empty()) {
        throw std::invalid_argument("Owner XUID cannot be empty for town creation");
    }

    if (!TestHelper::isValidCoordinate(x) || !TestHelper::isValidCoordinate(z)) {
        throw std::invalid_argument(
            "Invalid coordinates for town creation: x=" + std::to_string(x) + ", z=" + std::to_string(z)
        );
    }

    try {
        // 创建城镇数据
        TownData townData;
        townData.id          = getMaxId() + 1;
        townData.mayorXuid   = ownerXuid;
        townData.x           = x;
        townData.z           = z;
        townData.x_end       = x + 200; // 城镇默认大小比土地大
        townData.z_end       = z + 200;
        townData.d           = 0; // 默认维度
        townData.perm        = 5; // 城镇默认权限级别
        townData.description = "Auto-generated test town";
        townData.name        = "TestTown_" + std::to_string(townData.id);

        // 为测试用户添加模拟名称
        const std::string mayorName = "TestMayor_" + ownerXuid;
        mock::MockLeviLaminaAPI::addMockPlayer(ownerXuid, mayorName);

        // 创建城镇信息对象
        auto townInfo = std::make_unique<TownInformation>(townData);

        // 添加一些默认成员（用于测试）
        const std::string memberXuid = "town_member_" + std::to_string(townData.id);
        const std::string memberName = "TownMember_" + std::to_string(townData.id);
        mock::MockLeviLaminaAPI::addMockPlayer(memberXuid, memberName);
        townInfo->addMember(memberXuid);

        // 存储测试数据副本以供后续使用
        auto storedTown = std::make_unique<TownInformation>(townData);
        m_testData.push_back(std::move(storedTown));

        return townInfo;

    } catch (const std::exception& e) {
        // 记录错误并重新抛出
        throw std::runtime_error("Failed to create test town: " + std::string(e.what()));
    }
}

void TestDataLoader::cleanupTestData() {
    m_testData.clear();
    mock::MockLeviLaminaAPI::clearMockPlayers();
}

LONG64 TestDataLoader::getMaxId() const {
    LONG64 maxId = 0;
    for (const auto& data : m_testData) {
        if (data->getId() > maxId) {
            maxId = data->getId();
        }
    }
    return maxId;
}

bool TestDataLoader::validateTestData(const std::vector<std::unique_ptr<LandInformation>>& lands) {
    for (const auto& land : lands) {
        if (!TestHelper::isValidXuid(land->getOwnerXuid()) || !TestHelper::isValidCoordinate(land->getX())
            || !TestHelper::isValidCoordinate(land->getZ()) || !TestHelper::isValidSize(land->getWidth())
            || !TestHelper::isValidSize(land->getHeight())) {
            return false;
        }
    }
    return true;
}

bool TestDataLoader::validateBoundaryData(const std::vector<std::unique_ptr<LandInformation>>& lands) {
    auto boundaryValues = TestHelper::getBoundaryValues();

    for (const auto& land : lands) {
        int width  = land->getWidth();
        int height = land->getHeight();

        // 检查是否在边界值范围内
        if ((land->getX() == boundaryValues.minCoordinate || land->getX() == boundaryValues.maxCoordinate)
            || (land->getZ() == boundaryValues.minCoordinate || land->getZ() == boundaryValues.maxCoordinate)
            || (width == boundaryValues.minSize || width == boundaryValues.maxSize)
            || (height == boundaryValues.minSize || height == boundaryValues.maxSize)) {
            continue; // 边界值，有效
        }

        // 检查是否超出边界
        if (land->getX() < boundaryValues.minCoordinate || land->getX() > boundaryValues.maxCoordinate
            || land->getZ() < boundaryValues.minCoordinate || land->getZ() > boundaryValues.maxCoordinate
            || width < boundaryValues.minSize || width > boundaryValues.maxSize || height < boundaryValues.minSize
            || height > boundaryValues.maxSize) {
            return false; // 超出边界
        }
    }
    return true;
}

void TestDataLoader::addBoundaryLands(std::vector<std::unique_ptr<LandInformation>>& lands) {
    auto boundaryValues = TestHelper::getBoundaryValues();

    // 最小坐标边界
    auto land1 = createBoundaryTestLand(
        "boundary_min_coord",
        boundaryValues.minCoordinate,
        boundaryValues.minCoordinate,
        50,
        50
    );
    lands.push_back(std::move(land1));

    // 最大坐标边界
    auto land2 = createBoundaryTestLand(
        "boundary_max_coord",
        boundaryValues.maxCoordinate,
        boundaryValues.maxCoordinate,
        50,
        50
    );
    lands.push_back(std::move(land2));

    // 最小尺寸边界
    auto land3 = createBoundaryTestLand("boundary_min_size", 0, 0, boundaryValues.minSize, boundaryValues.minSize);
    lands.push_back(std::move(land3));

    // 最大尺寸边界
    auto land4 =
        createBoundaryTestLand("boundary_max_size", 10000, 10000, boundaryValues.maxSize, boundaryValues.maxSize);
    lands.push_back(std::move(land4));

    // 最小ID边界
    auto land5 = createBoundaryTestLand("boundary_min_id", 5000, 5000, 100, 100);
    land5->setId(boundaryValues.minId);
    lands.push_back(std::move(land5));

    // 最大ID边界
    auto land6 = createBoundaryTestLand("boundary_max_id", 6000, 6000, 100, 100);
    land6->setId(boundaryValues.maxId);
    lands.push_back(std::move(land6));
}

void TestDataLoader::addInvalidLands(std::vector<std::unique_ptr<LandInformation>>& lands) {
    // 空XUID
    auto land1 = createInvalidTestLand("", 1000, 1000, 1050, 1050);
    lands.push_back(std::move(land1));

    // 超出边界的坐标
    auto land2 = createInvalidTestLand("invalid_coord_1", 99999999, 1000, 100000049, 1050);
    lands.push_back(std::move(land2));

    auto land3 = createInvalidTestLand("invalid_coord_2", -99999999, 1000, -99999949, 1050);
    lands.push_back(std::move(land3));

    // 无效尺寸（终点坐标小于等于起点坐标）
    auto land4 = createInvalidTestLand("invalid_size_1", 2000, 2000, 2000, 2050); // 宽度为0
    lands.push_back(std::move(land4));

    auto land5 = createInvalidTestLand("invalid_size_2", 2000, 2000, 1999, 2050); // 宽度为负
    lands.push_back(std::move(land5));

    auto land6 = createInvalidTestLand("invalid_size_3", 2000, 2000, 2000, 2000); // 高度为0
    lands.push_back(std::move(land6));

    // 无效权限
    auto land7 = createInvalidTestLand("invalid_perm", 3000, 3000, 3050, 3050);
    land7->setPermission(-1);
    lands.push_back(std::move(land7));

    auto land8 = createInvalidTestLand("invalid_perm_2", 3000, 3000, 3050, 3050);
    land8->setPermission(999);
    lands.push_back(std::move(land8));
}

void TestDataLoader::addPerformanceLands(std::vector<std::unique_ptr<LandInformation>>& lands, int count) {
    std::random_device              rd;
    std::mt19937                    gen(rd());
    std::uniform_int_distribution<> coordDist(-10000, 10000);
    std::uniform_int_distribution<> sizeDist(10, 200);

    for (int i = 0; i < count; ++i) {
        std::string ownerXuid = "perf_test_" + std::to_string(i);
        int         x         = coordDist(gen);
        int         z         = coordDist(gen);
        int         width     = sizeDist(gen);
        int         height    = sizeDist(gen);

        auto land = createTestLand(ownerXuid, x, z);
        land->setCoordinates(land->getX(), land->getZ(), x + width, z + height);
        land->setDescription("Performance test land #" + std::to_string(i));

        lands.push_back(std::move(land));
    }
}

} // namespace rlx_land::test