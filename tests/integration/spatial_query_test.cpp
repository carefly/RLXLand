#include "common/LeviLaminaAPI.h"
#include "data/service/DataService.h"
#include "utils/TestEnvironment.h"
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>


namespace rlx_land::test {

TEST_CASE("Spatial Query and Boundary Check Tests", "[spatial][query][boundary]") {

    auto dataService = rlx_land::DataService::getInstance();
    dataService->clearAllData();

    rlx_land::LeviLaminaAPI::clearMockPlayers();
    // 设置模拟玩家数据
    rlx_land::LeviLaminaAPI::addMockPlayer("100000001", "腐竹", true); // 腐竹
    rlx_land::LeviLaminaAPI::addMockPlayer("200000001", "小明");
    rlx_land::LeviLaminaAPI::addMockPlayer("200000002", "小红");
    rlx_land::LeviLaminaAPI::addMockPlayer("200000003", "张三");
    rlx_land::LeviLaminaAPI::addMockPlayer("200000004", "李四");
    rlx_land::LeviLaminaAPI::addMockPlayer("200000005", "王五");


    SECTION("Basic Spatial Query Tests") {
        // 创建测试领地
        LandData testLand;
        testLand.ownerXuid = "200000001";
        testLand.x         = 100;
        testLand.z         = 100;
        testLand.x_end     = 200;
        testLand.z_end     = 200;
        testLand.d         = 0;
        testLand.id        = dataService->getMaxId<LandData>() + 1;

        PlayerInfo playerInfo("200000001", "小明", false);
        dataService->createItem<LandData>(testLand, playerInfo);

        SECTION("Query Point Inside Land") {
            // 测试领地内的点查询
            auto* land1 = dataService->findLandAt(150, 150, 0);
            REQUIRE(land1 != nullptr);
            REQUIRE(land1->getOwnerXuid() == "200000001");

            auto* land2 = dataService->findLandAt(100, 100, 0); // 边界点
            REQUIRE(land2 != nullptr);
            REQUIRE(land2->getOwnerXuid() == "200000001");

            auto* land3 = dataService->findLandAt(200, 200, 0); // 边界点
            REQUIRE(land3 != nullptr);
            REQUIRE(land3->getOwnerXuid() == "200000001");

            auto* land4 = dataService->findLandAt(125, 175, 0); // 领地内任意点
            REQUIRE(land4 != nullptr);
            REQUIRE(land4->getOwnerXuid() == "200000001");
        }

        SECTION("Query Point Outside Land") {
            // 测试领地外的点查询
            auto* land1 = dataService->findLandAt(99, 150, 0); // 左边界外
            REQUIRE(land1 == nullptr);

            auto* land2 = dataService->findLandAt(201, 150, 0); // 右边界外
            REQUIRE(land2 == nullptr);

            auto* land3 = dataService->findLandAt(150, 99, 0); // 上边界外
            REQUIRE(land3 == nullptr);

            auto* land4 = dataService->findLandAt(150, 201, 0); // 下边界外
            REQUIRE(land4 == nullptr);

            auto* land5 = dataService->findLandAt(50, 50, 0); // 远离领地
            REQUIRE(land5 == nullptr);
        }

        SECTION("Query Point in Different Dimensions") {
            // 测试不同维度的查询
            auto* landInDim0 = dataService->findLandAt(150, 150, 0);
            REQUIRE(landInDim0 != nullptr);

            auto* landInDim1 = dataService->findLandAt(150, 150, 1);
            REQUIRE(landInDim1 == nullptr);

            auto* landInDim2 = dataService->findLandAt(150, 150, 2);
            REQUIRE(landInDim2 == nullptr);
        }
    }

    SECTION("Complex Spatial Query Tests") {
        // 创建多个重叠或相邻的领地
        LandData land1;
        land1.ownerXuid = "200000001";
        land1.x         = 100;
        land1.z         = 100;
        land1.x_end     = 200;
        land1.z_end     = 200;
        land1.d         = 0;
        land1.id        = dataService->getMaxId<LandData>() + 1;

        PlayerInfo playerInfo1("200000001", "小明", false);
        dataService->createItem<LandData>(land1, playerInfo1);

        LandData land2;
        land2.ownerXuid = "200000002";
        land2.x         = 300;
        land2.z         = 100;
        land2.x_end     = 400;
        land2.z_end     = 200;
        land2.d         = 0;
        land2.id        = dataService->getMaxId<LandData>() + 1;

        PlayerInfo playerInfo2("200000002", "小红", false);
        dataService->createItem<LandData>(land2, playerInfo2);

        LandData land3;
        land3.ownerXuid = "200000003";
        land3.x         = 200;
        land3.z         = 300;
        land3.x_end     = 300;
        land3.z_end     = 400;
        land3.d         = 0;
        land3.id        = dataService->getMaxId<LandData>() + 1;

        PlayerInfo playerInfo3("200000003", "张三", false);
        dataService->createItem<LandData>(land3, playerInfo3);

        SECTION("Query Points in Multiple Lands") {
            // 验证不同领地的点查询
            auto* foundLand1 = dataService->findLandAt(150, 150, 0);
            REQUIRE(foundLand1 != nullptr);
            REQUIRE(foundLand1->getOwnerXuid() == "200000001");

            auto* foundLand2 = dataService->findLandAt(350, 150, 0);
            REQUIRE(foundLand2 != nullptr);
            REQUIRE(foundLand2->getOwnerXuid() == "200000002");

            auto* foundLand3 = dataService->findLandAt(250, 350, 0);
            REQUIRE(foundLand3 != nullptr);
            REQUIRE(foundLand3->getOwnerXuid() == "200000003");
        }

        SECTION("Query Points Between Lands") {
            // 测试领地之间的空地区域
            auto* landBetween1and2 = dataService->findLandAt(250, 150, 0);
            REQUIRE(landBetween1and2 == nullptr);

            auto* landBetween1and3 = dataService->findLandAt(200, 250, 0);
            REQUIRE(landBetween1and3 == nullptr);

            auto* landBetween2and3 = dataService->findLandAt(350, 250, 0);
            REQUIRE(landBetween2and3 == nullptr);
        }

        SECTION("Query Boundary Adjacent Points") {
            // 测试边界相邻的点
            auto* landAtBoundary1 = dataService->findLandAt(200, 150, 0); // land1右边界
            REQUIRE(landAtBoundary1 != nullptr);
            REQUIRE(landAtBoundary1->getOwnerXuid() == "200000001");

            auto* landAtBoundary2 = dataService->findLandAt(300, 150, 0); // land2左边界
            REQUIRE(landAtBoundary2 != nullptr);
            REQUIRE(landAtBoundary2->getOwnerXuid() == "200000002");

            auto* landAtBoundary3 = dataService->findLandAt(200, 300, 0); // land3上边界
            REQUIRE(landAtBoundary3 != nullptr);
            REQUIRE(landAtBoundary3->getOwnerXuid() == "200000003");
        }
    }

    SECTION("Town Spatial Query Tests") {
        // 创建测试城镇
        TownData testTown;
        testTown.name        = "测试城镇";
        testTown.mayorXuid   = "200000001";
        testTown.x           = 100;
        testTown.z           = 100;
        testTown.x_end       = 300;
        testTown.z_end       = 300;
        testTown.d           = 0;
        testTown.perm        = 0;
        testTown.description = "测试城镇";
        testTown.id          = dataService->getMaxId<TownData>() + 1;

        PlayerInfo operatorInfo("100000001", "腐竹", true);
        dataService->createItem<TownData>(testTown, operatorInfo);

        SECTION("Query Point Inside Town") {
            // 测试城镇内的点查询
            auto* town1 = dataService->findTownAt(200, 200, 0);
            REQUIRE(town1 != nullptr);
            REQUIRE(town1->getTownName() == "测试城镇");

            auto* town2 = dataService->findTownAt(100, 100, 0); // 边界点
            REQUIRE(town2 != nullptr);
            REQUIRE(town2->getTownName() == "测试城镇");

            auto* town3 = dataService->findTownAt(300, 300, 0); // 边界点
            REQUIRE(town3 != nullptr);
            REQUIRE(town3->getTownName() == "测试城镇");
        }

        SECTION("Query Point Outside Town") {
            // 测试城镇外的点查询
            auto* town1 = dataService->findTownAt(99, 200, 0); // 左边界外
            REQUIRE(town1 == nullptr);

            auto* town2 = dataService->findTownAt(301, 200, 0); // 右边界外
            REQUIRE(town2 == nullptr);

            auto* town3 = dataService->findTownAt(200, 99, 0); // 上边界外
            REQUIRE(town3 == nullptr);

            auto* town4 = dataService->findTownAt(200, 301, 0); // 下边界外
            REQUIRE(town4 == nullptr);

            auto* town5 = dataService->findTownAt(50, 50, 0); // 远离城镇
            REQUIRE(town5 == nullptr);
        }

        SECTION("Query Town in Different Dimensions") {
            // 测试不同维度的城镇查询
            auto* townInDim0 = dataService->findTownAt(200, 200, 0);
            REQUIRE(townInDim0 != nullptr);

            auto* townInDim1 = dataService->findTownAt(200, 200, 1);
            REQUIRE(townInDim1 == nullptr);

            auto* townInDim2 = dataService->findTownAt(200, 200, 2);
            REQUIRE(townInDim2 == nullptr);
        }
    }

    SECTION("Land and Town Combined Spatial Queries") {
        // 创建城镇
        TownData town;
        town.name        = "组合测试城镇";
        town.mayorXuid   = "200000001";
        town.x           = 100;
        town.z           = 100;
        town.x_end       = 400;
        town.z_end       = 400;
        town.d           = 0;
        town.perm        = 0;
        town.description = "组合测试城镇";
        town.id          = dataService->getMaxId<TownData>() + 1;

        PlayerInfo operatorInfo("100000001", "腐竹", true);
        dataService->createItem<TownData>(town, operatorInfo);

        // 添加城镇成员
        auto* createdTown = dataService->findTownAt(250, 250, 0);
        auto  center      = TestEnvironment::getInstance().getItemCenter<TownData>(createdTown);
        dataService->addItemMember<TownData>(
            center.first,
            center.second,
            createdTown->getDimension(),
            PlayerInfo("100000001", "腐竹", true),
            "小红"
        );

        // 在城镇内创建领地
        LandData landInTown;
        landInTown.ownerXuid = "200000002";
        landInTown.x         = 150;
        landInTown.z         = 150;
        landInTown.x_end     = 250;
        landInTown.z_end     = 250;
        landInTown.d         = 0;
        landInTown.id        = dataService->getMaxId<LandData>() + 1;

        PlayerInfo memberInfo("200000002", "小红", false);
        dataService->createItem<LandData>(landInTown, memberInfo);

        SECTION("Query Point in Both Land and Town") {
            // 测试同时在Land和Town内的点
            auto* land  = dataService->findLandAt(200, 200, 0);
            auto* town1 = dataService->findTownAt(200, 200, 0);

            REQUIRE(land != nullptr);
            REQUIRE(town1 != nullptr);
            REQUIRE(land->getOwnerXuid() == "200000002");
            REQUIRE(town1->getTownName() == "组合测试城镇");
        }

        SECTION("Query Point in Town but Outside Land") {
            // 测试在Town内但不在Land内的点
            auto* land  = dataService->findLandAt(350, 350, 0);
            auto* town2 = dataService->findTownAt(350, 350, 0);

            REQUIRE(land == nullptr);
            REQUIRE(town2 != nullptr);
            REQUIRE(town2->getTownName() == "组合测试城镇");
        }

        SECTION("Query Point Outside Both Land and Town") {
            // 测试既不在Land也不在Town内的点
            auto* land  = dataService->findLandAt(500, 500, 0);
            auto* town3 = dataService->findTownAt(500, 500, 0);

            REQUIRE(land == nullptr);
            REQUIRE(town3 == nullptr);
        }

        SECTION("Query Multiple Areas") {
            // 创建另一个领地在城镇外
            LandData landOutsideTown;
            landOutsideTown.ownerXuid = "200000003";
            landOutsideTown.x         = 500;
            landOutsideTown.z         = 500;
            landOutsideTown.x_end     = 600;
            landOutsideTown.z_end     = 600;
            landOutsideTown.d         = 0;
            landOutsideTown.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo3("200000003", "张三", false);
            dataService->createItem<LandData>(landOutsideTown, playerInfo3);

            // 测试多个区域查询
            struct TestCase {
                int         x, z, dim;
                bool        expectLand;
                bool        expectTown;
                std::string expectedLandOwner;
                std::string expectedTownName;
            };

            std::vector<TestCase> testCases = {
                {200, 200, 0, true,  true,  "200000002", "组合测试城镇"}, // 在Land和Town内
                {350, 350, 0, false, true,  "",          "组合测试城镇"}, // 在Town内但Land外
                {550, 550, 0, true,  false, "200000003", ""                  }, // 在Land内但Town外
                {700, 700, 0, false, false, "",          ""                  }, // 都不在
                {200, 200, 1, false, false, "",          ""                  }, // 不同维度
            };

            for (const auto& testCase : testCases) {
                auto* land  = dataService->findLandAt(testCase.x, testCase.z, testCase.dim);
                auto* town4 = dataService->findTownAt(testCase.x, testCase.z, testCase.dim);

                if (testCase.expectLand) {
                    REQUIRE(land != nullptr);
                    REQUIRE(land->getOwnerXuid() == testCase.expectedLandOwner);
                } else {
                    REQUIRE(land == nullptr);
                }

                if (testCase.expectTown) {
                    REQUIRE(town4 != nullptr);
                    REQUIRE(town4->getTownName() == testCase.expectedTownName);
                } else {
                    REQUIRE(town4 == nullptr);
                }
            }
        }
    }

    SECTION("Boundary Precision Tests") {
        SECTION("Exact Boundary Points") {
            // 创建精确边界的领地
            LandData precisionLand;
            precisionLand.ownerXuid = "200000001";
            precisionLand.x         = 100;
            precisionLand.z         = 100;
            precisionLand.x_end     = 200;
            precisionLand.z_end     = 200;
            precisionLand.d         = 0;
            precisionLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000001", "小明", false);
            dataService->createItem<LandData>(precisionLand, playerInfo);

            // 测试所有边界点
            struct BoundaryPoint {
                int  x, z;
                bool shouldFind;
            };

            std::vector<BoundaryPoint> boundaryPoints = {
                {100, 100, true }, // 左上角
                {200, 100, true }, // 右上角
                {100, 200, true }, // 左下角
                {200, 200, true }, // 右下角
                {150, 100, true }, // 上边界中点
                {150, 200, true }, // 下边界中点
                {100, 150, true }, // 左边界中点
                {200, 150, true }, // 右边界中点
                {99,  150, false}, // 左边界外1格
                {201, 150, false}, // 右边界外1格
                {150, 99,  false}, // 上边界外1格
                {150, 201, false}, // 下边界外1格
            };

            for (const auto& point : boundaryPoints) {
                auto* land = dataService->findLandAt(point.x, point.z, 0);
                if (point.shouldFind) {
                    REQUIRE(land != nullptr);
                    REQUIRE(land->getOwnerXuid() == "200000001");
                } else {
                    REQUIRE(land == nullptr);
                }
            }
        }

        SECTION("Large Area Boundary Test") {
            // 创建大尺寸领地进行边界测试（调整到符合1000格限制）
            LandData largeLand;
            largeLand.ownerXuid = "200000002";
            largeLand.x         = -500;
            largeLand.z         = -500;
            largeLand.x_end     = 500;
            largeLand.z_end     = 500;
            largeLand.d         = 0;
            largeLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000002", "小红", false);
            dataService->createItem<LandData>(largeLand, playerInfo);

            // 测试大领地的边界
            auto* land1 = dataService->findLandAt(-500, -500, 0); // 左上角
            REQUIRE(land1 != nullptr);

            auto* land2 = dataService->findLandAt(500, 500, 0); // 右下角
            REQUIRE(land2 != nullptr);

            auto* land3 = dataService->findLandAt(0, 0, 0); // 中心点
            REQUIRE(land3 != nullptr);

            auto* land4 = dataService->findLandAt(-501, 0, 0); // 边界外
            REQUIRE(land4 == nullptr);

            auto* land5 = dataService->findLandAt(501, 0, 0); // 边界外
            REQUIRE(land5 == nullptr);
        }
    }

    SECTION("Performance and Stress Tests") {
        SECTION("Multiple Small Lands Query Performance") {
            // 创建多个小领地
            std::vector<LandData> lands;
            for (int i = 0; i < 100; i++) {
                LandData smallLand;
                smallLand.ownerXuid = "200000001";
                smallLand.x         = i * 10;
                smallLand.z         = i * 10;
                smallLand.x_end     = i * 10 + 5;
                smallLand.z_end     = i * 10 + 5;
                smallLand.d         = 0;
                smallLand.id        = dataService->getMaxId<LandData>() + 1 + i;

                PlayerInfo playerInfo("200000001", "小明", false);
                dataService->createItem<LandData>(smallLand, playerInfo);
                lands.push_back(smallLand);
            }

            // 测试查询性能
            for (int i = 0; i < 100; i++) {
                auto* land = dataService->findLandAt(i * 10 + 2, i * 10 + 2, 0);
                REQUIRE(land != nullptr);
                REQUIRE(land->getOwnerXuid() == "200000001");
            }

            // 测试空区域查询
            for (int i = 0; i < 50; i++) {
                auto* land = dataService->findLandAt(i * 10 + 7, i * 10 + 7, 0);
                REQUIRE(land == nullptr);
            }
        }

        SECTION("Large Number of Query Operations") {
            // 创建一个领地
            LandData testLand;
            testLand.ownerXuid = "200000001";
            testLand.x         = 0;
            testLand.z         = 0;
            testLand.x_end     = 100;
            testLand.z_end     = 100;
            testLand.d         = 0;
            testLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000001", "小明", false);
            dataService->createItem<LandData>(testLand, playerInfo);

            // 执行大量查询操作
            for (int i = 0; i < 1000; i++) {
                int x = (i % 200) - 50;         // -50 到 149
                int z = ((i / 200) % 200) - 50; // -50 到 149

                auto* land = dataService->findLandAt(x, z, 0);

                if (x >= 0 && x <= 100 && z >= 0 && z <= 100) {
                    REQUIRE(land != nullptr);
                    REQUIRE(land->getOwnerXuid() == "200000001");
                } else {
                    REQUIRE(land == nullptr);
                }
            }
        }
    }

    SECTION("Edge Cases and Special Coordinates") {
        SECTION("Extreme Coordinate Values") {
            // 测试极端坐标值（在边界内）
            LandData extremeLand;
            extremeLand.ownerXuid = "200000001";
            extremeLand.x         = LAND_RANGE - 1000;
            extremeLand.z         = LAND_RANGE - 1000;
            extremeLand.x_end     = LAND_RANGE - 900;
            extremeLand.z_end     = LAND_RANGE - 900;
            extremeLand.d         = 0;
            extremeLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000001", "小明", false);
            dataService->createItem<LandData>(extremeLand, playerInfo);

            auto* land = dataService->findLandAt(LAND_RANGE - 950, LAND_RANGE - 950, 0);
            REQUIRE(land != nullptr);
            REQUIRE(land->getOwnerXuid() == "200000001");
        }

        SECTION("Negative Coordinates") {
            // 测试负坐标
            LandData negativeLand;
            negativeLand.ownerXuid = "200000002";
            negativeLand.x         = -200;
            negativeLand.z         = -200;
            negativeLand.x_end     = -100;
            negativeLand.z_end     = -100;
            negativeLand.d         = 0;
            negativeLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000002", "小红", false);
            dataService->createItem<LandData>(negativeLand, playerInfo);

            auto* land = dataService->findLandAt(-150, -150, 0);
            REQUIRE(land != nullptr);
            REQUIRE(land->getOwnerXuid() == "200000002");
        }

        SECTION("Zero Coordinates") {
            // 测试零坐标
            LandData zeroLand;
            zeroLand.ownerXuid = "200000003";
            zeroLand.x         = 0;
            zeroLand.z         = 0;
            zeroLand.x_end     = 50;
            zeroLand.z_end     = 50;
            zeroLand.d         = 0;
            zeroLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000003", "张三", false);
            dataService->createItem<LandData>(zeroLand, playerInfo);

            auto* land = dataService->findLandAt(25, 25, 0);
            REQUIRE(land != nullptr);
            REQUIRE(land->getOwnerXuid() == "200000003");
        }

        SECTION("Single Point Land") {
            // 测试单点领地
            LandData pointLand;
            pointLand.ownerXuid = "200000004";
            pointLand.x         = 100;
            pointLand.z         = 100;
            pointLand.x_end     = 100;
            pointLand.z_end     = 100;
            pointLand.d         = 0;
            pointLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000004", "李四", false);
            dataService->createItem<LandData>(pointLand, playerInfo);

            auto* land1 = dataService->findLandAt(100, 100, 0);
            REQUIRE(land1 != nullptr);
            REQUIRE(land1->getOwnerXuid() == "200000004");

            auto* land2 = dataService->findLandAt(101, 100, 0);
            REQUIRE(land2 == nullptr);

            auto* land3 = dataService->findLandAt(100, 101, 0);
            REQUIRE(land3 == nullptr);
        }
    }

    SECTION("Spatial Query with Data Modifications") {
        // 创建初始领地
        LandData initialLand;
        initialLand.ownerXuid = "200000001";
        initialLand.x         = 100;
        initialLand.z         = 100;
        initialLand.x_end     = 200;
        initialLand.z_end     = 200;
        initialLand.d         = 0;
        initialLand.id        = dataService->getMaxId<LandData>() + 1;

        PlayerInfo playerInfo("200000001", "小明", false);
        dataService->createItem<LandData>(initialLand, playerInfo);

        SECTION("Query After Land Deletion") {
            // 验证领地存在
            auto* landBefore = dataService->findLandAt(150, 150, 0);
            REQUIRE(landBefore != nullptr);

            // 删除领地
            dataService->deleteItem<LandData>(initialLand);

            // 验证查询返回空
            auto* landAfter = dataService->findLandAt(150, 150, 0);
            REQUIRE(landAfter == nullptr);
        }

        SECTION("Query After Land Creation") {
            // 验证初始区域为空
            auto* landBefore = dataService->findLandAt(300, 300, 0);
            REQUIRE(landBefore == nullptr);

            // 创建新领地
            LandData newLand;
            newLand.ownerXuid = "200000002";
            newLand.x         = 250;
            newLand.z         = 250;
            newLand.x_end     = 350;
            newLand.z_end     = 350;
            newLand.d         = 0;
            newLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo2("200000002", "小红", false);
            dataService->createItem<LandData>(newLand, playerInfo2);

            // 验证新领地可以查询到
            auto* landAfter = dataService->findLandAt(300, 300, 0);
            REQUIRE(landAfter != nullptr);
            REQUIRE(landAfter->getOwnerXuid() == "200000002");
        }

        SECTION("Query After Multiple Operations") {
            // 创建多个领地
            std::vector<LandData> lands;
            for (int i = 0; i < 5; i++) {
                LandData land;
                land.ownerXuid = "20000000" + std::to_string(1 + i);
                land.x         = 300 + i * 100;
                land.z         = 300 + i * 100;
                land.x_end     = 350 + i * 100;
                land.z_end     = 350 + i * 100;
                land.d         = 0;
                land.id        = dataService->getMaxId<LandData>() + 1 + i;

                PlayerInfo playerInfoLoop(
                    "20000000" + std::to_string(1 + i),
                    rlx_land::LeviLaminaAPI::getPlayerNameByXuid("20000000" + std::to_string(1 + i)),
                    false
                );
                dataService->createItem<LandData>(land, playerInfoLoop);
                lands.push_back(land);
            }

            // 验证所有领地都可以查询到
            for (int i = 0; i < 5; i++) {
                auto* land = dataService->findLandAt(325 + i * 100, 325 + i * 100, 0);
                REQUIRE(land != nullptr);
                REQUIRE(land->getOwnerXuid() == "20000000" + std::to_string(1 + i));
            }

            // 删除部分领地
            dataService->deleteItem<LandData>(lands[1]); // 删除第二个领地
            dataService->deleteItem<LandData>(lands[3]); // 删除第四个领地

            // 验证删除的领地查询不到
            auto* deletedLand1 = dataService->findLandAt(425, 425, 0);
            REQUIRE(deletedLand1 == nullptr);

            auto* deletedLand2 = dataService->findLandAt(625, 625, 0);
            REQUIRE(deletedLand2 == nullptr);

            // 验证其他领地仍然可以查询到
            auto* existingLand1 = dataService->findLandAt(325, 325, 0);
            REQUIRE(existingLand1 != nullptr);
            REQUIRE(existingLand1->getOwnerXuid() == "200000001");

            auto* existingLand2 = dataService->findLandAt(525, 525, 0);
            REQUIRE(existingLand2 != nullptr);
            REQUIRE(existingLand2->getOwnerXuid() == "200000003");

            auto* existingLand3 = dataService->findLandAt(725, 725, 0);
            REQUIRE(existingLand3 != nullptr);
            REQUIRE(existingLand3->getOwnerXuid() == "200000005");
        }
    }
}

} // namespace rlx_land::test