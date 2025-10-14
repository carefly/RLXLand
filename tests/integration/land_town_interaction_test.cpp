#include "common/LeviLaminaAPI.h"
#include "common/exceptions/LandExceptions.h"
#include "data/service/DataService.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>


namespace rlx_land::test {

TEST_CASE("Land and Town Interaction Tests", "[land][town][interaction]") {

    auto dataService = rlx_land::DataService::getInstance();
    dataService->clearAllData();

    rlx_land::LeviLaminaAPI::clearMockPlayers();
    // 设置模拟玩家数据（包括腐竹）
    rlx_land::LeviLaminaAPI::addMockPlayer("100000001", "腐竹", true); // 腐竹
    rlx_land::LeviLaminaAPI::addMockPlayer("200000001", "小明");
    rlx_land::LeviLaminaAPI::addMockPlayer("200000002", "小红");
    rlx_land::LeviLaminaAPI::addMockPlayer("200000003", "张三");
    rlx_land::LeviLaminaAPI::addMockPlayer("200000004", "李四");
    rlx_land::LeviLaminaAPI::addMockPlayer("200000005", "王五");

    SECTION("Land Creation within Town") {
        // 首先创建一个城镇
        TownData townData;
        townData.name        = "测试城镇";
        townData.mayorXuid   = "200000001";
        townData.x           = 100;
        townData.z           = 100;
        townData.x_end       = 300;
        townData.z_end       = 300;
        townData.d           = 0;
        townData.perm        = 0;
        townData.description = "用于测试Land在Town内创建的城镇";
        townData.id          = dataService->getMaxId<TownData>() + 1;

        PlayerInfo operatorInfo("100000001", "腐竹", true);
        dataService->createItem<TownData>(townData, operatorInfo);

        // 添加城镇成员
        auto* town = dataService->findTownAt(200, 200, 0);
        REQUIRE(town != nullptr);
        dataService->addItemMember<TownData>(town, "小红");
        dataService->addItemMember<TownData>(town, "张三");

        SECTION("Town Member Can Create Land within Town") {
            // 城镇成员在城镇内创建领地
            LandData landData;
            landData.ownerXuid = "200000002"; // 小红
            landData.x         = 150;
            landData.z         = 150;
            landData.x_end     = 200;
            landData.z_end     = 200;
            landData.d         = 0;
            landData.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo memberInfo("200000002", "小红", false);
            dataService->createItem<LandData>(landData, memberInfo);

            // 验证领地创建成功
            auto* land = dataService->findLandAt(175, 175, 0);
            REQUIRE(land != nullptr);
            REQUIRE(land->getOwnerXuid() == "200000002");

            // 验证城镇仍然存在
            auto* existingTown = dataService->findTownAt(175, 175, 0);
            REQUIRE(existingTown != nullptr);
            REQUIRE(existingTown->getTownName() == "测试城镇");
        }

        SECTION("Non-Town Member Cannot Create Land within Town") {
            // 非城镇成员尝试在城镇内创建领地
            LandData landData;
            landData.ownerXuid = "200000004"; // 李四（非城镇成员）
            landData.x         = 250;
            landData.z         = 250;
            landData.x_end     = 280;
            landData.z_end     = 280;
            landData.d         = 0;
            landData.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo nonMemberInfo("200000004", "李四", false);

            // 验证非城镇成员无法在城镇内创建领地
            REQUIRE_THROWS_AS(dataService->createItem<LandData>(landData, nonMemberInfo), RealmPermissionException);

            // 验证领地没有被创建
            auto* land = dataService->findLandAt(265, 265, 0);
            REQUIRE(land == nullptr);

            // 验证城镇仍然存在
            auto* existingTown = dataService->findTownAt(265, 265, 0);
            REQUIRE(existingTown != nullptr);
            REQUIRE(existingTown->getTownName() == "测试城镇");
        }

        SECTION("Operator Can Create Land within Any Town") {
            // 腐竹可以在任何城镇内创建领地
            LandData landData;
            landData.ownerXuid = "100000001"; // 腐竹
            landData.x         = 120;
            landData.z         = 120;
            landData.x_end     = 140;
            landData.z_end     = 140;
            landData.d         = 0;
            landData.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo operatorInfo("100000001", "腐竹", true);
            dataService->createItem<LandData>(landData, operatorInfo);

            // 验证领地创建成功
            auto* land = dataService->findLandAt(130, 130, 0);
            REQUIRE(land != nullptr);
            REQUIRE(land->getOwnerXuid() == "100000001");
        }
    }

    SECTION("Land Creation in Wilderness") {
        SECTION("Any Player Can Create Land in Wilderness") {
            // 在野外（不在任何城镇内）创建领地
            LandData landData1;
            landData1.ownerXuid = "200000001"; // 小明
            landData1.x         = 1000;
            landData1.z         = 1000;
            landData1.x_end     = 1050;
            landData1.z_end     = 1050;
            landData1.d         = 0;
            landData1.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo1("200000001", "小明", false);
            dataService->createItem<LandData>(landData1, playerInfo1);

            // 验证领地创建成功
            auto* land1 = dataService->findLandAt(1025, 1025, 0);
            REQUIRE(land1 != nullptr);
            REQUIRE(land1->getOwnerXuid() == "200000001");

            // 另一个玩家也可以在野外创建领地
            LandData landData2;
            landData2.ownerXuid = "200000002"; // 小红
            landData2.x         = 1100;
            landData2.z         = 1100;
            landData2.x_end     = 1150;
            landData2.z_end     = 1150;
            landData2.d         = 0;
            landData2.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo2("200000002", "小红", false);
            dataService->createItem<LandData>(landData2, playerInfo2);

            // 验证第二个领地创建成功
            auto* land2 = dataService->findLandAt(1125, 1125, 0);
            REQUIRE(land2 != nullptr);
            REQUIRE(land2->getOwnerXuid() == "200000002");

            // 验证两个领地都存在
            REQUIRE(land1->getOwnerXuid() == "200000001");
            REQUIRE(land2->getOwnerXuid() == "200000002");
        }

        SECTION("Land Conflict Detection in Wilderness") {
            // 在野外创建第一个领地
            LandData firstLand;
            firstLand.ownerXuid = "200000001";
            firstLand.x         = 2000;
            firstLand.z         = 2000;
            firstLand.x_end     = 2050;
            firstLand.z_end     = 2050;
            firstLand.d         = 0;
            firstLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo1("200000001", "小明", false);
            dataService->createItem<LandData>(firstLand, playerInfo1);

            // 验证第一个领地创建成功
            auto* createdLand = dataService->findLandAt(2025, 2025, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getOwnerXuid() == "200000001");

            // 尝试创建重叠的领地
            LandData conflictingLand;
            conflictingLand.ownerXuid = "200000002";
            conflictingLand.x         = 2025;
            conflictingLand.z         = 2025;
            conflictingLand.x_end     = 2075;
            conflictingLand.z_end     = 2075;
            conflictingLand.d         = 0;
            conflictingLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo2("200000002", "小红", false);

            // 验证重叠的领地创建失败
            REQUIRE_THROWS_AS(dataService->createItem<LandData>(conflictingLand, playerInfo2), RealmConflictException);

            // 验证原领地仍然存在
            auto* originalLand = dataService->findLandAt(2025, 2025, 0);
            REQUIRE(originalLand != nullptr);
            REQUIRE(originalLand->getOwnerXuid() == "200000001");
        }
    }

    SECTION("Town Overlap Detection") {
        SECTION("Town Cannot Overlap Existing Town") {
            // 创建第一个城镇
            TownData firstTown;
            firstTown.name        = "第一个城镇";
            firstTown.mayorXuid   = "200000001";
            firstTown.x           = 500;
            firstTown.z           = 500;
            firstTown.x_end       = 600;
            firstTown.z_end       = 600;
            firstTown.d           = 0;
            firstTown.perm        = 0;
            firstTown.description = "第一个城镇";
            firstTown.id          = dataService->getMaxId<TownData>() + 1;

            PlayerInfo operatorInfo("100000001", "腐竹", true);
            dataService->createItem<TownData>(firstTown, operatorInfo);

            // 验证第一个城镇创建成功
            auto* createdTown1 = dataService->findTownAt(550, 550, 0);
            REQUIRE(createdTown1 != nullptr);
            REQUIRE(createdTown1->getTownName() == "第一个城镇");

            // 尝试创建重叠的城镇
            TownData overlappingTown;
            overlappingTown.name        = "重叠城镇";
            overlappingTown.mayorXuid   = "200000002";
            overlappingTown.x           = 550;
            overlappingTown.z           = 550;
            overlappingTown.x_end       = 650;
            overlappingTown.z_end       = 650;
            overlappingTown.d           = 0;
            overlappingTown.perm        = 0;
            overlappingTown.description = "重叠的城镇";
            overlappingTown.id          = dataService->getMaxId<TownData>() + 1;

            // 验证重叠的城镇创建失败
            REQUIRE_THROWS_AS(dataService->createItem<TownData>(overlappingTown, operatorInfo), RealmConflictException);

            // 验证第一个城镇仍然存在
            auto* existingTown = dataService->findTownAt(550, 550, 0);
            REQUIRE(existingTown != nullptr);
            REQUIRE(existingTown->getTownName() == "第一个城镇");
        }

        SECTION("Multiple Towns Can Coexist Without Overlap") {
            // 创建第一个城镇
            TownData town1;
            town1.name        = "城镇1";
            town1.mayorXuid   = "200000001";
            town1.x           = 100;
            town1.z           = 100;
            town1.x_end       = 200;
            town1.z_end       = 200;
            town1.d           = 0;
            town1.perm        = 0;
            town1.description = "城镇1";
            town1.id          = dataService->getMaxId<TownData>() + 1;

            PlayerInfo operatorInfo("100000001", "腐竹", true);
            dataService->createItem<TownData>(town1, operatorInfo);

            // 创建第二个不重叠的城镇
            TownData town2;
            town2.name        = "城镇2";
            town2.mayorXuid   = "200000002";
            town2.x           = 300;
            town2.z           = 300;
            town2.x_end       = 400;
            town2.z_end       = 400;
            town2.d           = 0;
            town2.perm        = 0;
            town2.description = "城镇2";
            town2.id          = dataService->getMaxId<TownData>() + 1;

            dataService->createItem<TownData>(town2, operatorInfo);

            // 验证两个城镇都存在
            auto* createdTown1 = dataService->findTownAt(150, 150, 0);
            auto* createdTown2 = dataService->findTownAt(350, 350, 0);
            REQUIRE(createdTown1 != nullptr);
            REQUIRE(createdTown2 != nullptr);
            REQUIRE(createdTown1->getTownName() == "城镇1");
            REQUIRE(createdTown2->getTownName() == "城镇2");
        }
    }

    SECTION("Land and Town Spatial Queries") {
        // 创建一个城镇
        TownData townData;
        townData.name        = "查询测试城镇";
        townData.mayorXuid   = "200000001";
        townData.x           = 100;
        townData.z           = 100;
        townData.x_end       = 300;
        townData.z_end       = 300;
        townData.d           = 0;
        townData.perm        = 0;
        townData.description = "用于空间查询测试的城镇";
        townData.id          = dataService->getMaxId<TownData>() + 1;

        PlayerInfo operatorInfo("100000001", "腐竹", true);
        dataService->createItem<TownData>(townData, operatorInfo);

        // 在城镇内创建领地
        LandData landInTown;
        landInTown.ownerXuid = "200000002";
        landInTown.x         = 150;
        landInTown.z         = 150;
        landInTown.x_end     = 200;
        landInTown.z_end     = 200;
        landInTown.d         = 0;
        landInTown.id        = dataService->getMaxId<LandData>() + 1;

        PlayerInfo memberInfo("200000002", "小红", false);
        dataService->addItemMember<TownData>(dataService->findTownAt(175, 175, 0), "小红");
        dataService->createItem<LandData>(landInTown, memberInfo);

        // 在城镇外创建领地
        LandData landOutsideTown;
        landOutsideTown.ownerXuid = "200000003";
        landOutsideTown.x         = 400;
        landOutsideTown.z         = 400;
        landOutsideTown.x_end     = 450;
        landOutsideTown.z_end     = 450;
        landOutsideTown.d         = 0;
        landOutsideTown.id        = dataService->getMaxId<LandData>() + 1;

        PlayerInfo playerInfo3("200000003", "张三", false);
        dataService->createItem<LandData>(landOutsideTown, playerInfo3);

        SECTION("Query Point Inside Land and Town") {
            // 查询同时在Land和Town内的点
            auto* land = dataService->findLandAt(175, 175, 0);
            auto* town = dataService->findTownAt(175, 175, 0);

            REQUIRE(land != nullptr);
            REQUIRE(town != nullptr);
            REQUIRE(land->getOwnerXuid() == "200000002");
            REQUIRE(town->getTownName() == "查询测试城镇");
        }

        SECTION("Query Point Inside Town but Outside Land") {
            // 查询在Town内但不在Land内的点
            auto* land = dataService->findLandAt(250, 250, 0);
            auto* town = dataService->findTownAt(250, 250, 0);

            REQUIRE(land == nullptr);
            REQUIRE(town != nullptr);
            REQUIRE(town->getTownName() == "查询测试城镇");
        }

        SECTION("Query Point Outside Both Land and Town") {
            // 查询既不在Land也不在Town内的点
            auto* land = dataService->findLandAt(500, 500, 0);
            auto* town = dataService->findTownAt(500, 500, 0);

            REQUIRE(land == nullptr);
            REQUIRE(town == nullptr);
        }

        SECTION("Query Point Inside Land but Outside Town") {
            // 查询在Land内但不在Town内的点
            auto* land = dataService->findLandAt(425, 425, 0);
            auto* town = dataService->findTownAt(425, 425, 0);

            REQUIRE(land != nullptr);
            REQUIRE(town == nullptr);
            REQUIRE(land->getOwnerXuid() == "200000003");
        }
    }

    SECTION("Land and Town Deletion Interactions") {
        // 创建一个城镇
        TownData townData;
        townData.name        = "删除交互测试城镇";
        townData.mayorXuid   = "200000001";
        townData.x           = 100;
        townData.z           = 100;
        townData.x_end       = 300;
        townData.z_end       = 300;
        townData.d           = 0;
        townData.perm        = 0;
        townData.description = "用于删除交互测试的城镇";
        townData.id          = dataService->getMaxId<TownData>() + 1;

        PlayerInfo operatorInfo("100000001", "腐竹", true);
        dataService->createItem<TownData>(townData, operatorInfo);

        // 在城镇内创建领地
        LandData landInTown;
        landInTown.ownerXuid = "200000002";
        landInTown.x         = 150;
        landInTown.z         = 150;
        landInTown.x_end     = 200;
        landInTown.z_end     = 200;
        landInTown.d         = 0;
        landInTown.id        = dataService->getMaxId<LandData>() + 1;

        PlayerInfo memberInfo("200000002", "小红", false);
        dataService->addItemMember<TownData>(dataService->findTownAt(175, 175, 0), "小红");
        dataService->createItem<LandData>(landInTown, memberInfo);

        SECTION("Delete Town Does Not Affect Land") {
            // 删除城镇
            dataService->deleteItem<TownData>(townData);

            // 验证城镇被删除
            auto* deletedTown = dataService->findTownAt(175, 175, 0);
            REQUIRE(deletedTown == nullptr);

            // 验证领地仍然存在
            auto* existingLand = dataService->findLandAt(175, 175, 0);
            REQUIRE(existingLand != nullptr);
            REQUIRE(existingLand->getOwnerXuid() == "200000002");
        }

        SECTION("Delete Land Does Not Affect Town") {
            // 删除领地
            dataService->deleteItem<LandData>(landInTown);

            // 验证领地被删除
            auto* deletedLand = dataService->findLandAt(175, 175, 0);
            REQUIRE(deletedLand == nullptr);

            // 验证城镇仍然存在
            auto* existingTown = dataService->findTownAt(175, 175, 0);
            REQUIRE(existingTown != nullptr);
            REQUIRE(existingTown->getTownName() == "删除交互测试城镇");
        }
    }

    SECTION("Multi-Dimension Land and Town Interaction") {
        // 在维度0创建城镇
        TownData townDim0;
        townDim0.name        = "维度0城镇";
        townDim0.mayorXuid   = "200000001";
        townDim0.x           = 100;
        townDim0.z           = 100;
        townDim0.x_end       = 200;
        townDim0.z_end       = 200;
        townDim0.d           = 0;
        townDim0.perm        = 0;
        townDim0.description = "维度0的城镇";
        townDim0.id          = dataService->getMaxId<TownData>() + 1;

        PlayerInfo operatorInfo("100000001", "腐竹", true);
        dataService->createItem<TownData>(townDim0, operatorInfo);

        // 在维度1创建相同坐标的城镇
        TownData townDim1;
        townDim1.name        = "维度1城镇";
        townDim1.mayorXuid   = "200000001";
        townDim1.x           = 100;
        townDim1.z           = 100;
        townDim1.x_end       = 200;
        townDim1.z_end       = 200;
        townDim1.d           = 1;
        townDim1.perm        = 0;
        townDim1.description = "维度1的城镇";
        townDim1.id          = dataService->getMaxId<TownData>() + 1;

        dataService->createItem<TownData>(townDim1, operatorInfo);

        // 在维度0的城镇内创建领地
        LandData landDim0;
        landDim0.ownerXuid = "200000002";
        landDim0.x         = 150;
        landDim0.z         = 150;
        landDim0.x_end     = 180;
        landDim0.z_end     = 180;
        landDim0.d         = 0;
        landDim0.id        = dataService->getMaxId<LandData>() + 1;

        PlayerInfo memberInfo("200000002", "小红", false);
        dataService->addItemMember<TownData>(dataService->findTownAt(150, 150, 0), "小红");
        dataService->createItem<LandData>(landDim0, memberInfo);

        SECTION("Different Dimensions Are Independent") {
            // 验证维度0的城镇和领地
            auto* townInDim0 = dataService->findTownAt(150, 150, 0);
            auto* landInDim0 = dataService->findLandAt(165, 165, 0);
            REQUIRE(townInDim0 != nullptr);
            REQUIRE(landInDim0 != nullptr);
            REQUIRE(townInDim0->getDimension() == 0);
            REQUIRE(landInDim0->getDimension() == 0);

            // 验证维度1的城镇存在但没有领地
            auto* townInDim1 = dataService->findTownAt(150, 150, 1);
            auto* landInDim1 = dataService->findLandAt(165, 165, 1);
            REQUIRE(townInDim1 != nullptr);
            REQUIRE(landInDim1 == nullptr);
            REQUIRE(townInDim1->getDimension() == 1);

            // 在维度1创建领地
            LandData landDim1;
            landDim1.ownerXuid = "200000003";
            landDim1.x         = 150;
            landDim1.z         = 150;
            landDim1.x_end     = 180;
            landDim1.z_end     = 180;
            landDim1.d         = 1;
            landDim1.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo3("200000003", "张三", false);
            dataService->addItemMember<TownData>(dataService->findTownAt(150, 150, 1), "张三");
            dataService->createItem<LandData>(landDim1, playerInfo3);

            // 验证维度1的领地创建成功
            auto* createdLandInDim1 = dataService->findLandAt(165, 165, 1);
            REQUIRE(createdLandInDim1 != nullptr);
            REQUIRE(createdLandInDim1->getDimension() == 1);

            // 验证维度0的领地不受影响
            auto* existingLandInDim0 = dataService->findLandAt(165, 165, 0);
            REQUIRE(existingLandInDim0 != nullptr);
            REQUIRE(existingLandInDim0->getOwnerXuid() == "200000002");
        }
    }

    SECTION("Complex Land and Town Scenarios") {
        SECTION("Multiple Towns with Different Members") {
            // 创建两个不同的城镇
            TownData town1;
            town1.name        = "城镇A";
            town1.mayorXuid   = "200000001";
            town1.x           = 100;
            town1.z           = 100;
            town1.x_end       = 200;
            town1.z_end       = 200;
            town1.d           = 0;
            town1.perm        = 0;
            town1.description = "城镇A";
            town1.id          = dataService->getMaxId<TownData>() + 1;

            PlayerInfo operatorInfo("100000001", "腐竹", true);
            dataService->createItem<TownData>(town1, operatorInfo);

            TownData town2;
            town2.name        = "城镇B";
            town2.mayorXuid   = "200000002";
            town2.x           = 300;
            town2.z           = 300;
            town2.x_end       = 400;
            town2.z_end       = 400;
            town2.d           = 0;
            town2.perm        = 0;
            town2.description = "城镇B";
            town2.id          = dataService->getMaxId<TownData>() + 1;
            dataService->createItem<TownData>(town2, operatorInfo);

            // 为城镇A添加成员
            auto* townA = dataService->findTownAt(150, 150, 0);
            dataService->addItemMember<TownData>(townA, "小红");

            // 为城镇B添加成员
            auto* townB = dataService->findTownAt(350, 350, 0);
            dataService->addItemMember<TownData>(townB, "张三");

            // 城镇A的成员在城镇A内创建领地
            LandData landInTownA;
            landInTownA.ownerXuid = "200000002";
            landInTownA.x         = 120;
            landInTownA.z         = 120;
            landInTownA.x_end     = 150;
            landInTownA.z_end     = 150;
            landInTownA.d         = 0;
            landInTownA.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo memberA("200000002", "小红", false);
            dataService->createItem<LandData>(landInTownA, memberA);

            // 城镇B的成员尝试在城镇A内创建领地（应该失败）
            LandData landInTownAByMemberB;
            landInTownAByMemberB.ownerXuid = "200000003";
            landInTownAByMemberB.x         = 160;
            landInTownAByMemberB.z         = 160;
            landInTownAByMemberB.x_end     = 180;
            landInTownAByMemberB.z_end     = 180;
            landInTownAByMemberB.d         = 0;
            landInTownAByMemberB.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo memberB("200000003", "张三", false);

            // 验证城镇B的成员无法在城镇A内创建领地
            REQUIRE_THROWS_AS(
                dataService->createItem<LandData>(landInTownAByMemberB, memberB),
                RealmPermissionException
            );

            // 验证城镇A的成员可以在城镇B内创建领地（如果B成员）
            dataService->addItemMember<TownData>(townB, "小红"); // 将小红也加入城镇B

            LandData landInTownBByMemberA;
            landInTownBByMemberA.ownerXuid = "200000002";
            landInTownBByMemberA.x         = 320;
            landInTownBByMemberA.z         = 320;
            landInTownBByMemberA.x_end     = 350;
            landInTownBByMemberA.z_end     = 350;
            landInTownBByMemberA.d         = 0;
            landInTownBByMemberA.id        = dataService->getMaxId<LandData>() + 1;

            // 验证城镇A的成员可以在城镇B内创建领地（因为也是B成员）
            REQUIRE_NOTHROW(dataService->createItem<LandData>(landInTownBByMemberA, memberA));

            auto* createdLandInTownB = dataService->findLandAt(335, 335, 0);
            REQUIRE(createdLandInTownB != nullptr);
            REQUIRE(createdLandInTownB->getOwnerXuid() == "200000002");
        }
    }
}

} // namespace rlx_land::test