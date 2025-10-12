#include "common/exceptions/LandExceptions.h"
#include "data/land/LandCore.h"
#include "data/service/DataService.h"
#include "mocks/MockLeviLaminaAPI.h"
#include "mocks/MockPlayer.h"
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>


namespace rlx_land::test {

TEST_CASE("Land Management Integration Tests", "[land][integration]") {

    auto dataService = rlx_land::DataService::getInstance();
    dataService->clearAllData();
    // 设置模拟玩家数据
    mock::MockLeviLaminaAPI::clearMockPlayers();
    mock::MockLeviLaminaAPI::addMockPlayer("12345", "TestPlayer");
    mock::MockLeviLaminaAPI::addMockPlayer("67890", "LandOwner");
    mock::MockLeviLaminaAPI::addMockPlayer("11111", "Member1");
    mock::MockLeviLaminaAPI::addMockPlayer("22222", "Member2");

    SECTION("Basic Land Creation via DataService") {
        // 1. 准备测试玩家
        MockPlayer testPlayer("TestPlayer", "12345");
        testPlayer.setOperator(false);

        // 2. 通过 DataService 创建土地
        LandData landData;
        landData.ownerXuid = "12345";
        landData.x         = 100;
        landData.z         = 200;
        landData.x_end     = 150;
        landData.z_end     = 300;
        landData.d         = 0; // 默认维度
        landData.id        = dataService->getMaxId<LandData>() + 1;

        // 3. 使用 DataService 创建土地
        PlayerInfo playerInfo("12345", "TestPlayer", false);
        dataService->createItem<LandData>(landData, playerInfo);


        // 4. 验证土地创建
        SECTION("Land Data Validation via DataService") {

            auto* land = dataService->findLandAt(100, 200, 0);
            REQUIRE(land != nullptr);
            REQUIRE(land->getOwnerXuid() == "12345");
            REQUIRE(land->getX() == 100);
            REQUIRE(land->getZ() == 200);
            REQUIRE(land->getXEnd() == 150);
            REQUIRE(land->getZEnd() == 300);
            REQUIRE(land->getOwnerName() == "TestPlayer");
        }

        SECTION("Land Owner Validation via DataService") {

            auto* land = dataService->findLandAt(100, 200, 0);
            REQUIRE(land != nullptr);
            REQUIRE(land->isOwner("12345"));
            REQUIRE_FALSE(land->isOwner("67890"));
        }
    }

    SECTION("Land Creation with Validation") {
        MockPlayer testPlayer("TestPlayer", "12345");
        testPlayer.setOperator(false);

        SECTION("Valid Land Creation") {
            LandData landData;
            landData.ownerXuid = "12345";
            landData.x         = 100;
            landData.z         = 200;
            landData.x_end     = 150;
            landData.z_end     = 300;
            landData.d         = 0;
            landData.id        = dataService->getMaxId<LandData>() + 1;

            // 使用PlayerInfo结构创建土地
            PlayerInfo playerInfo(testPlayer.getXuid(), testPlayer.getName(), testPlayer.isOperator());
            REQUIRE_NOTHROW(dataService->createItem<LandData>(landData, playerInfo));

            // 验证土地创建成功
            auto* land = dataService->findLandAt(100, 200, 0);
            REQUIRE(land != nullptr);
            REQUIRE(land->getOwnerXuid() == "12345");
        }

        SECTION("Land Creation with Out of Range Coordinates") {
            LandData landData;
            landData.ownerXuid = "12345";
            landData.x         = LAND_RANGE + 1; // 超出范围
            landData.z         = 200;
            landData.x_end     = LAND_RANGE + 50;
            landData.z_end     = 300;
            landData.d         = 0;
            landData.id        = dataService->getMaxId<LandData>() + 1;

            // 应该抛出坐标超出范围异常
            PlayerInfo playerInfo2(testPlayer.getXuid(), testPlayer.getName(), testPlayer.isOperator());
            REQUIRE_THROWS_AS(dataService->createItem<LandData>(landData, playerInfo2), LandOutOfRangeException);
        }

        SECTION("Land Creation with Conflict") {
            // 先创建一个土地
            LandData existingLand;
            existingLand.ownerXuid = "67890";
            existingLand.x         = 100;
            existingLand.z         = 200;
            existingLand.x_end     = 150;
            existingLand.z_end     = 300;
            existingLand.d         = 0;
            existingLand.id        = dataService->getMaxId<LandData>() + 1;
            PlayerInfo existingPlayerInfo("67890", "LandOwner", false);
            dataService->createItem<LandData>(existingLand, existingPlayerInfo);

            // 尝试创建冲突的土地
            LandData conflictLand;
            conflictLand.ownerXuid = "12345";
            conflictLand.x         = 125; // 与现有土地重叠
            conflictLand.z         = 250;
            conflictLand.x_end     = 175;
            conflictLand.z_end     = 350;
            conflictLand.d         = 0;
            conflictLand.id        = dataService->getMaxId<LandData>() + 1;

            // 应该抛出领地冲突异常
            PlayerInfo playerInfo3(testPlayer.getXuid(), testPlayer.getName(), testPlayer.isOperator());
            REQUIRE_THROWS_AS(dataService->createItem<LandData>(conflictLand, playerInfo3), LandConflictException);
        }
    }

    SECTION("Town Creation with Validation") {
        MockPlayer testPlayer("TestPlayer", "12345");
        testPlayer.setOperator(true); // Town创建需要腐竹权限

        SECTION("Valid Town Creation") {
            TownData townData;
            townData.name        = "TestTown";
            townData.mayorXuid   = "12345";
            townData.x           = 100;
            townData.z           = 200;
            townData.x_end       = 150;
            townData.z_end       = 300;
            townData.d           = 0;
            townData.id          = dataService->getMaxId<TownData>() + 1;
            townData.description = "Test Town Description";

            // 使用PlayerInfo结构创建城镇
            PlayerInfo playerInfo(testPlayer.getXuid(), testPlayer.getName(), testPlayer.isOperator());
            REQUIRE_NOTHROW(dataService->createItem<TownData>(townData, playerInfo));

            // 验证城镇创建成功
            auto* town = dataService->findTownAt(100, 200, 0);
            REQUIRE(town != nullptr);
            REQUIRE(town->getTownName() == "TestTown");
        }

        SECTION("Town Creation with Out of Range Coordinates") {
            TownData townData;
            townData.name        = "TestTown";
            townData.mayorXuid   = "12345";
            townData.x           = LAND_RANGE + 1; // 超出范围
            townData.z           = 200;
            townData.x_end       = LAND_RANGE + 50;
            townData.z_end       = 300;
            townData.d           = 0;
            townData.id          = dataService->getMaxId<TownData>() + 1;
            townData.description = "Test Town Description";

            // 应该抛出坐标超出范围异常
            PlayerInfo playerInfo(testPlayer.getXuid(), testPlayer.getName(), testPlayer.isOperator());
            REQUIRE_THROWS_AS(dataService->createItem<TownData>(townData, playerInfo), TownOutOfRangeException);
        }

        SECTION("Town Creation without Operator Permission") {
            MockPlayer nonOpPlayer("NonOpPlayer", "99999");
            nonOpPlayer.setOperator(false);

            TownData townData;
            townData.name        = "TestTown";
            townData.mayorXuid   = "99999";
            townData.x           = 100;
            townData.z           = 200;
            townData.x_end       = 150;
            townData.z_end       = 300;
            townData.d           = 0;
            townData.id          = dataService->getMaxId<TownData>() + 1;
            townData.description = "Test Town Description";

            // 应该抛出权限异常
            PlayerInfo playerInfo(nonOpPlayer.getXuid(), nonOpPlayer.getName(), nonOpPlayer.isOperator());
            REQUIRE_THROWS_AS(dataService->createItem<TownData>(townData, playerInfo), TownPermissionException);
        }

        SECTION("Town Creation with Conflict") {
            // 先创建一个城镇
            TownData existingTown;
            existingTown.name        = "ExistingTown";
            existingTown.mayorXuid   = "12345";
            existingTown.x           = 100;
            existingTown.z           = 200;
            existingTown.x_end       = 150;
            existingTown.z_end       = 300;
            existingTown.d           = 0;
            existingTown.id          = dataService->getMaxId<TownData>() + 1;
            existingTown.description = "Existing Town Description";

            PlayerInfo existingPlayerInfo(testPlayer.getXuid(), testPlayer.getName(), testPlayer.isOperator());
            dataService->createItem<TownData>(existingTown, existingPlayerInfo);

            // 尝试创建冲突的城镇
            TownData conflictTown;
            conflictTown.name        = "ConflictTown";
            conflictTown.mayorXuid   = "12345";
            conflictTown.x           = 125; // 与现有城镇重叠
            conflictTown.z           = 250;
            conflictTown.x_end       = 175;
            conflictTown.z_end       = 350;
            conflictTown.d           = 0;
            conflictTown.id          = dataService->getMaxId<TownData>() + 1;
            conflictTown.description = "Conflict Town Description";

            // 应该抛出城镇冲突异常
            PlayerInfo conflictPlayerInfo(testPlayer.getXuid(), testPlayer.getName(), testPlayer.isOperator());
            REQUIRE_THROWS_AS(
                dataService->createItem<TownData>(conflictTown, conflictPlayerInfo),
                TownConflictException
            );
        }
    }

    // SECTION("Member Management via DataService") {
    //     // 1. 通过 DataService 创建土地
    //     LandData landData;
    //     landData.ownerXuid = "67890";
    //     landData.x         = 300;
    //     landData.z         = 400;
    //     landData.dx        = 50;
    //     landData.dz        = 50;
    //     landData.d         = 0;
    //     landData.id        = dataService->getMaxId<LandData>() + 1;

    //     dataService->createItem<LandData>(landData);
    //     auto* land = dataService->findLandAt(300, 400, 0);
    //     REQUIRE(land != nullptr);

    //     // 2. 测试成员添加
    //     SECTION("Add Member via DataService") {
    //         // 使用 DataService 添加成员
    //         dataService->addItemMember<LandData>(land, "Member1");

    //         // 验证成员添加成功
    //         REQUIRE(land->hasBasicPermission("11111")); // Member1 的 XUID
    //         REQUIRE_FALSE(land->isOwner("11111"));
    //     }

    //     SECTION("Remove Member via DataService") {
    //         // 先添加成员
    //         dataService->addItemMember<LandData>(land, "Member2");
    //         REQUIRE(land->hasBasicPermission("22222"));

    //         // 使用 DataService 移除成员
    //         dataService->removeItemMember<LandData>(land, "Member2");

    //         REQUIRE_FALSE(land->hasBasicPermission("22222"));
    //     }
    // }

    // SECTION("Data Query via DataService") {
    //     // 创建多个测试土地
    //     std::vector<LandData> testLands;

    //     LandData land1;
    //     land1.ownerXuid   = "test_owner_001";
    //     land1.x           = 1000;
    //     land1.z           = 1000;
    //     land1.dx          = 50;
    //     land1.dz          = 50;
    //     land1.d           = 0;
    //     land1.perm        = 0;
    //     land1.id          = dataService->getMaxId<LandData>() + 1;
    //     land1.description = "";
    //     testLands.push_back(land1);

    //     LandData land2;
    //     land2.ownerXuid   = "test_owner_002";
    //     land2.x           = 2000;
    //     land2.z           = 2000;
    //     land2.dx          = 60;
    //     land2.dz          = 60;
    //     land2.d           = 0;
    //     land2.perm        = 0;
    //     land2.id          = dataService->getMaxId<LandData>() + 2;
    //     land2.description = "";
    //     testLands.push_back(land2);

    //     // 设置模拟玩家
    //     mock::MockLeviLaminaAPI::addMockPlayer("test_owner_001", "TestOwner1");
    //     mock::MockLeviLaminaAPI::addMockPlayer("test_owner_002", "TestOwner2");

    //     // 通过 DataService 创建土地
    //     for (auto& landData : testLands) {
    //         dataService->createItem<LandData>(landData);
    //     }

    //     SECTION("Query All Lands via DataService") {
    //         auto allLands = dataService->getAllItems<LandData>();
    //         REQUIRE(!allLands.empty());
    //         REQUIRE(allLands.size() >= 2); // 至少有刚创建的2个测试土地

    //         // 验证第一个测试土地
    //         auto* firstLand = allLands[0];
    //         REQUIRE(firstLand->ld.ownerXuid == "test_owner_001");
    //         REQUIRE(firstLand->data.x == 1000);
    //         REQUIRE(firstLand->data.z == 1000);
    //     }

    //     SECTION("Spatial Query via DataService") {
    //         auto* land1 = dataService->findLandAt(1000, 1000, 0);
    //         REQUIRE(land1 != nullptr);
    //         REQUIRE(land1->ld.ownerXuid == "test_owner_001");

    //         auto* land2 = dataService->findLandAt(2000, 2000, 0);
    //         REQUIRE(land2 != nullptr);
    //         REQUIRE(land2->ld.ownerXuid == "test_owner_002");

    //         // 测试不存在的位置
    //         auto* nonExistent = dataService->findLandAt(999, 999, 0);
    //         REQUIRE(nonExistent == nullptr);
    //     }
    // }

    // SECTION("Permission Management via DataService") {
    //     // 创建测试土地
    //     LandData landData;
    //     landData.ownerXuid = "12345";
    //     landData.x         = 500;
    //     landData.z         = 600;
    //     landData.dx        = 50;
    //     landData.dz        = 50;
    //     landData.d         = 0;
    //     landData.id        = dataService->getMaxId<LandData>() + 1;

    //     dataService->createItem<LandData>(landData);
    //     auto* land = dataService->findLandAt(500, 600, 0);
    //     REQUIRE(land != nullptr);

    //     SECTION("Modify Permission via DataService") {
    //         // 修改权限为只读
    //         dataService->modifyItemPermission<LandData>(land, 1); // 假设1表示只读权限

    //         // 验证权限修改（这里需要根据实际的权限验证逻辑来测试）
    //         REQUIRE(land->ld.perm == 1);
    //     }
    // }

    // SECTION("Performance Test via DataService") {
    //     // 简单的性能测试
    //     const int NUM_LANDS = 100;

    //     TestHelper::measureExecutionTime("Create " + std::to_string(NUM_LANDS) + " lands via DataService", [&]() {
    //         for (int i = 0; i < NUM_LANDS; ++i) {
    //             LandData landData;
    //             landData.ownerXuid = std::to_string(10000 + i);
    //             landData.x         = 100 * i;
    //             landData.z         = 200 * i;
    //             landData.dx        = 50;
    //             landData.dz        = 50;
    //             landData.d         = 0;
    //             landData.id        = dataService->getMaxId<LandData>() + 1;

    //             dataService->createItem<LandData>(landData);
    //         }
    //     });

    //     // 验证所有土地都创建成功
    //     auto allLands = dataService->getAllItems<LandData>();
    //     REQUIRE(allLands.size() >= NUM_LANDS);

    //     // 验证数据完整性和空间查询
    //     for (int i = 0; i < NUM_LANDS; ++i) {
    //         auto* land = dataService->findLandAt(100 * i, 200 * i, 0);
    //         REQUIRE(land != nullptr);
    //         REQUIRE(land->ld.ownerXuid == std::to_string(10000 + i));
    //     }
    // }

    // 测试环境将由全局固定装置自动清理
}

} // namespace rlx_land::test