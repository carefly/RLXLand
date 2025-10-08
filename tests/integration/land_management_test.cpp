#include "data/land/LandCore.h"
#include "mocks/MockPlayer.h"
#include "utils/TestDataLoader.h"
#include "utils/TestEnvironment.h"
#include "utils/TestHelper.h"
#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <functional>
#include <memory>
#include <string>


namespace rlx_land::test {

TEST_CASE("Land Management Integration Tests", "[land][integration]") {
    rlx_land::test::TestEnvironment::getInstance().initialize();
    rlx_land::test::TestDataLoader& dataLoader = rlx_land::test::TestDataLoader::getInstance();

    SECTION("Basic Land Creation") {
        // 1. 准备测试玩家
        MockPlayer testPlayer("TestPlayer", "12345");
        testPlayer.setOperator(false);

        // 2. 准备土地数据
        auto landData = dataLoader.createTestLand("12345", 100, 200);
        REQUIRE(landData != nullptr);

        // 3. 验证土地创建
        SECTION("Land Data Validation") {
            REQUIRE(landData->ld.ownerXuid == "12345");
            REQUIRE(landData->data.x == 100);
            REQUIRE(landData->data.z == 200);
            REQUIRE(landData->data.dx == 50);
            REQUIRE(landData->data.dz == 50);
        }

        SECTION("Land Owner Validation") {
            REQUIRE(landData->isOwner("12345"));
            REQUIRE_FALSE(landData->isOwner("67890"));
        }
    }

    SECTION("Member Management") {
        // 1. 创建土地和所有者
        MockPlayer owner("LandOwner", "67890");
        auto       land = dataLoader.createTestLand("67890", 300, 400);

        // 2. 测试成员添加
        SECTION("Add Member Success") {
            MockPlayer member("LandMember", "11111");

            // 模拟添加成员操作
            land->data.memberXuids.push_back("11111");

            // 验证成员添加成功
            REQUIRE(land->hasBasicPermission("11111"));
            REQUIRE_FALSE(land->isOwner("11111"));
        }

        SECTION("Remove Member Success") {
            // 先添加成员
            land->data.memberXuids.push_back("22222");
            REQUIRE(land->hasBasicPermission("22222"));

            // 移除成员
            auto& members = land->data.memberXuids;
            members.erase(std::remove(members.begin(), members.end(), "22222"), members.end());

            REQUIRE_FALSE(land->hasBasicPermission("22222"));
        }
    }

    SECTION("Data Loading") {
        // 测试数据加载功能
        auto loadedLands = dataLoader.loadLandTestData();

        SECTION("Load Test Data") {
            REQUIRE(!loadedLands.empty());
            REQUIRE(loadedLands.size() >= 2); // 至少有2个测试土地

            // 验证第一个测试土地
            auto& firstLand = loadedLands[0];
            REQUIRE(firstLand->ld.ownerXuid == "test_owner_001");
            REQUIRE(firstLand->data.x == 1000);
            REQUIRE(firstLand->data.z == 1000);
        }

        SECTION("Data Consistency") {
            for (const auto& land : loadedLands) {
                REQUIRE(!land->ld.ownerXuid.empty());
                REQUIRE(land->data.id > 0);
                REQUIRE(land->data.dx > 0);
                REQUIRE(land->data.dz > 0);
            }
        }
    }

    SECTION("Performance Test") {
        // 简单的性能测试
        const int                                     NUM_LANDS = 100;
        std::vector<std::unique_ptr<LandInformation>> lands;

        TestHelper::measureExecutionTime("Create " + std::to_string(NUM_LANDS) + " lands", [&]() {
            for (int i = 0; i < NUM_LANDS; ++i) {
                auto land = dataLoader.createTestLand(std::to_string(10000 + i), 100 * i, 200 * i);
                lands.push_back(std::move(land));
            }
        });

        // 验证所有土地都创建成功
        REQUIRE(lands.size() == NUM_LANDS);

        // 验证数据完整性
        for (int i = 0; i < NUM_LANDS; ++i) {
            REQUIRE(lands[i]->ld.ownerXuid == std::to_string(10000 + i));
            REQUIRE(lands[i]->data.x == 100 * i);
            REQUIRE(lands[i]->data.z == 200 * i);
        }
    }

    rlx_land::test::TestEnvironment::getInstance().cleanup();
}

} // namespace rlx_land::test