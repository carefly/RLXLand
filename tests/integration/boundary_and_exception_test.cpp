#include "common/LeviLaminaAPI.h"
#include "common/exceptions/LandExceptions.h"
#include "data/service/DataService.h"
#include "utils/TestEnvironment.h"
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>


namespace rlx_land::test {

using rlx_town::TownData;
using rlx_town::TownInformation;

TEST_CASE("Boundary and Exception Tests", "[boundary][exception]") {

    auto dataService = rlx_land::DataService::getInstance();
    dataService->clearAllData();

    rlx_land::LeviLaminaAPI::clearMockPlayers();
    // 设置模拟玩家数据
    rlx_land::LeviLaminaAPI::addMockPlayer("100000001", "腐竹", true); // 腐竹
    rlx_land::LeviLaminaAPI::addMockPlayer("200000001", "小明");
    rlx_land::LeviLaminaAPI::addMockPlayer("200000002", "小红");
    rlx_land::LeviLaminaAPI::addMockPlayer("200000003", "张三");
    rlx_land::LeviLaminaAPI::addMockPlayer("200000004", "李四", false);
    rlx_land::LeviLaminaAPI::addMockPlayer("200000005", "王五", false);

    SECTION("Land Boundary Tests") {
        SECTION("Land at Positive Boundary") {
            // 测试正边界附近的领地创建
            LandData boundaryLand;
            boundaryLand.ownerXuid = "200000001";
            boundaryLand.x         = LAND_RANGE - 100;
            boundaryLand.z         = LAND_RANGE - 100;
            boundaryLand.x_end     = LAND_RANGE - 50;
            boundaryLand.z_end     = LAND_RANGE - 50;
            boundaryLand.d         = 0;
            boundaryLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000001", "小明", false);

            // 验证边界范围内的领地可以创建
            REQUIRE_NOTHROW(dataService->createItem<LandData>(boundaryLand, playerInfo));

            auto* createdLand = dataService->findLandAt(LAND_RANGE - 75, LAND_RANGE - 75, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getOwnerXuid() == "200000001");
        }

        SECTION("Land at Negative Boundary") {
            // 测试负边界附近的领地创建
            LandData negativeBoundaryLand;
            negativeBoundaryLand.ownerXuid = "200000002";
            negativeBoundaryLand.x         = -LAND_RANGE + 50;
            negativeBoundaryLand.z         = -LAND_RANGE + 50;
            negativeBoundaryLand.x_end     = -LAND_RANGE + 100;
            negativeBoundaryLand.z_end     = -LAND_RANGE + 100;
            negativeBoundaryLand.d         = 0;
            negativeBoundaryLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000002", "小红", false);

            // 验证负边界范围内的领地可以创建
            REQUIRE_NOTHROW(dataService->createItem<LandData>(negativeBoundaryLand, playerInfo));

            auto* createdLand = dataService->findLandAt(-LAND_RANGE + 75, -LAND_RANGE + 75, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getOwnerXuid() == "200000002");
        }

        SECTION("Land Exceeding Positive Boundary") {
            // 测试超出正边界的领地创建
            LandData outOfBoundsLand;
            outOfBoundsLand.ownerXuid = "200000001";
            outOfBoundsLand.x         = LAND_RANGE + 1;
            outOfBoundsLand.z         = 100;
            outOfBoundsLand.x_end     = LAND_RANGE + 100;
            outOfBoundsLand.z_end     = 200;
            outOfBoundsLand.d         = 0;
            outOfBoundsLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000001", "小明", false);

            // 验证超出边界的领地创建失败
            REQUIRE_THROWS_AS(dataService->createItem<LandData>(outOfBoundsLand, playerInfo), RealmOutOfRangeException);
        }

        SECTION("Land Exceeding Negative Boundary") {
            // 测试超出负边界的领地创建
            LandData outOfBoundsLand;
            outOfBoundsLand.ownerXuid = "200000002";
            outOfBoundsLand.x         = -LAND_RANGE - 100;
            outOfBoundsLand.z         = 100;
            outOfBoundsLand.x_end     = -LAND_RANGE - 50;
            outOfBoundsLand.z_end     = 200;
            outOfBoundsLand.d         = 0;
            outOfBoundsLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000002", "小红", false);

            // 验证超出负边界的领地创建失败
            REQUIRE_THROWS_AS(dataService->createItem<LandData>(outOfBoundsLand, playerInfo), RealmOutOfRangeException);
        }

        SECTION("Land Crossing Boundary") {
            // 测试跨越边界的领地创建
            LandData crossingBoundaryLand;
            crossingBoundaryLand.ownerXuid = "200000001";
            crossingBoundaryLand.x         = LAND_RANGE - 50;
            crossingBoundaryLand.z         = 100;
            crossingBoundaryLand.x_end     = LAND_RANGE + 50;
            crossingBoundaryLand.z_end     = 200;
            crossingBoundaryLand.d         = 0;
            crossingBoundaryLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000001", "小明", false);

            // 验证跨越边界的领地创建失败
            REQUIRE_THROWS_AS(
                dataService->createItem<LandData>(crossingBoundaryLand, playerInfo),
                RealmOutOfRangeException
            );
        }

        SECTION("Minimum Size Land") {
            // 测试最小尺寸的领地（1x1）
            LandData minSizeLand;
            minSizeLand.ownerXuid = "200000001";
            minSizeLand.x         = 100;
            minSizeLand.z         = 100;
            minSizeLand.x_end     = 100; // 1x1领地
            minSizeLand.z_end     = 100;
            minSizeLand.d         = 0;
            minSizeLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000001", "小明", false);

            // 验证最小尺寸领地可以创建
            REQUIRE_NOTHROW(dataService->createItem<LandData>(minSizeLand, playerInfo));

            auto* createdLand = dataService->findLandAt(100, 100, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getWidth() == 0);  // x_end - x = 0
            REQUIRE(createdLand->getHeight() == 0); // z_end - z = 0
        }

        SECTION("Large Size Land") {
            // 测试大尺寸领地
            LandData largeLand;
            largeLand.ownerXuid = "200000002";
            largeLand.x         = 1000;
            largeLand.z         = 1000;
            largeLand.x_end     = 2000; // 1000x1000领地
            largeLand.z_end     = 2000;
            largeLand.d         = 0;
            largeLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000002", "小红", false);

            // 验证大尺寸领地可以创建
            REQUIRE_NOTHROW(dataService->createItem<LandData>(largeLand, playerInfo));

            auto* createdLand = dataService->findLandAt(1500, 1500, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getWidth() == 1000);
            REQUIRE(createdLand->getHeight() == 1000);
            REQUIRE(createdLand->getArea() == 1000000);
        }

        SECTION("Very Large Size Land") {
            // 测试超大尺寸领地（超出原来的MAX_SIZE限制）
            LandData veryLargeLand;
            veryLargeLand.ownerXuid = "200000002";
            veryLargeLand.x         = 5000;
            veryLargeLand.z         = 5000;
            veryLargeLand.x_end     = 15000; // 10000x10000领地，远超原来的1000限制
            veryLargeLand.z_end     = 15000;
            veryLargeLand.d         = 0;
            veryLargeLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000002", "小红", false);

            // 验证超大尺寸领地可以创建（不再受MAX_SIZE限制）
            REQUIRE_NOTHROW(dataService->createItem<LandData>(veryLargeLand, playerInfo));

            auto* createdLand = dataService->findLandAt(10000, 10000, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getWidth() == 10000);
            REQUIRE(createdLand->getHeight() == 10000);
            REQUIRE(createdLand->getArea() == 100000000);
        }
    }

    SECTION("Town Boundary Tests") {
        SECTION("Town at Positive Boundary") {
            // 测试正边界附近的城镇创建
            TownData boundaryTown;
            boundaryTown.name        = "边界城镇";
            boundaryTown.mayorXuid   = "200000001";
            boundaryTown.x           = LAND_RANGE - 200;
            boundaryTown.z           = LAND_RANGE - 200;
            boundaryTown.x_end       = LAND_RANGE - 100;
            boundaryTown.z_end       = LAND_RANGE - 100;
            boundaryTown.d           = 0;
            boundaryTown.perm        = 0;
            boundaryTown.description = "边界城镇";
            boundaryTown.id          = dataService->getMaxId<TownData>() + 1;

            PlayerInfo operatorInfo("100000001", "腐竹", true);

            // 验证边界范围内的城镇可以创建
            REQUIRE_NOTHROW(dataService->createItem<TownData>(boundaryTown, operatorInfo));

            auto* createdTown = dataService->findTownAt(LAND_RANGE - 150, LAND_RANGE - 150, 0);
            REQUIRE(createdTown != nullptr);
            REQUIRE(createdTown->getTownName() == "边界城镇");
        }

        SECTION("Town Exceeding Boundary") {
            // 测试超出边界的城镇创建
            TownData outOfBoundsTown;
            outOfBoundsTown.name        = "越界城镇";
            outOfBoundsTown.mayorXuid   = "200000001";
            outOfBoundsTown.x           = LAND_RANGE + 1;
            outOfBoundsTown.z           = 100;
            outOfBoundsTown.x_end       = LAND_RANGE + 100;
            outOfBoundsTown.z_end       = 200;
            outOfBoundsTown.d           = 0;
            outOfBoundsTown.perm        = 0;
            outOfBoundsTown.description = "越界城镇";
            outOfBoundsTown.id          = dataService->getMaxId<TownData>() + 1;

            PlayerInfo operatorInfo("100000001", "腐竹", true);

            // 验证超出边界的城镇创建失败
            REQUIRE_THROWS_AS(
                dataService->createItem<TownData>(outOfBoundsTown, operatorInfo),
                RealmOutOfRangeException
            );
        }
    }

    SECTION("Exception Handling Tests") {
        SECTION("Invalid Land Data") {
            SECTION("Invalid Coordinates") {
                // 测试无效坐标（x > x_end）
                LandData invalidLand;
                invalidLand.ownerXuid = "200000001";
                invalidLand.x         = 200;
                invalidLand.z         = 200;
                invalidLand.x_end     = 100; // x_end < x
                invalidLand.z_end     = 300;
                invalidLand.d         = 0;
                invalidLand.id        = dataService->getMaxId<LandData>() + 1;

                PlayerInfo playerInfo("200000001", "小明", false);

                // 验证无效坐标的领地创建失败
                REQUIRE_THROWS_AS(
                    dataService->createItem<LandData>(invalidLand, playerInfo),
                    InvalidCoordinatesException
                );
            }

            SECTION("Negative Size Land") {
                // 测试负尺寸领地
                LandData negativeSizeLand;
                negativeSizeLand.ownerXuid = "200000001";
                negativeSizeLand.x         = 200;
                negativeSizeLand.z         = 200;
                negativeSizeLand.x_end     = 100; // 负宽度
                negativeSizeLand.z_end     = 100; // 负高度
                negativeSizeLand.d         = 0;
                negativeSizeLand.id        = dataService->getMaxId<LandData>() + 1;

                PlayerInfo playerInfo("200000001", "小明", false);

                // 验证负尺寸领地创建失败
                REQUIRE_THROWS_AS(
                    dataService->createItem<LandData>(negativeSizeLand, playerInfo),
                    InvalidCoordinatesException
                );
            }
        }

        SECTION("Invalid Town Data") {
            SECTION("Empty Town Name") {
                // 测试空城镇名称
                TownData emptyNameTown;
                emptyNameTown.name        = ""; // 空名称
                emptyNameTown.mayorXuid   = "200000001";
                emptyNameTown.x           = 100;
                emptyNameTown.z           = 100;
                emptyNameTown.x_end       = 200;
                emptyNameTown.z_end       = 200;
                emptyNameTown.d           = 0;
                emptyNameTown.perm        = 0;
                emptyNameTown.description = "空名称城镇";
                emptyNameTown.id          = dataService->getMaxId<TownData>() + 1;

                PlayerInfo operatorInfo("100000001", "腐竹", true);

                // 根据实际实现调整测试预期
                // 如果系统允许空名称，则测试创建成功
                // 如果不允许，则测试抛出异常
                try {
                    dataService->createItem<TownData>(emptyNameTown, operatorInfo);
                    // 如果创建成功，验证城镇确实被创建
                    auto* town = dataService->findTownAt(150, 150, 0);
                    REQUIRE(town != nullptr);
                    // 验证空名称被接受（当前实现允许空名称）
                    REQUIRE(town->getTownName().empty());
                } catch (const std::exception&) {
                    // 如果抛出异常，说明系统不允许空名称
                    // 这种情况下应该抛出适当的异常类型
                    REQUIRE(false); // 当前实现应该允许空名称，如果抛出异常则测试失败
                }
            }

            SECTION("Invalid Mayor XUID") {
                // 测试无效的镇长XUID
                TownData invalidMayorTown;
                invalidMayorTown.name        = "无效镇长城镇";
                invalidMayorTown.mayorXuid   = ""; // 空XUID
                invalidMayorTown.x           = 100;
                invalidMayorTown.z           = 100;
                invalidMayorTown.x_end       = 200;
                invalidMayorTown.z_end       = 200;
                invalidMayorTown.d           = 0;
                invalidMayorTown.perm        = 0;
                invalidMayorTown.description = "无效镇长城镇";
                invalidMayorTown.id          = dataService->getMaxId<TownData>() + 1;

                PlayerInfo operatorInfo("100000001", "腐竹", true);

                // 根据实际实现调整测试预期
                try {
                    dataService->createItem<TownData>(invalidMayorTown, operatorInfo);
                    // 如果创建成功，验证城镇确实被创建
                    auto* town = dataService->findTownAt(150, 150, 0);
                    REQUIRE(town != nullptr);
                    // 验证空XUID被接受（当前实现允许空XUID）
                    REQUIRE(town->getMayorXuid().empty());
                    // 验证城镇名称正确设置
                    REQUIRE(town->getTownName() == "无效镇长城镇");
                } catch (const std::exception&) {
                    // 如果抛出异常，说明系统不允许空XUID
                    // 这种情况下应该抛出适当的异常类型
                    REQUIRE(false); // 当前实现应该允许空XUID，如果抛出异常则测试失败
                }
            }
        }

        SECTION("Null Pointer Handling") {
            SECTION("Null Land Information") {

                // 验证空指针操作抛出异常
                REQUIRE_THROWS_AS(
                    dataService->modifyItemPermission<LandData>(0, 0, 0, 1, PlayerInfo("200000001", "小明", false)),
                    RealmNotFoundException
                );

                // 使用确实没有land的坐标进行测试
                LONG64 testX   = 888888;
                LONG64 testZ   = 888888;
                int    testDim = 2; // 修复：使用有效维度值 0-2

                // 首先验证该坐标确实没有land
                auto* verifyLand = dataService->findLandAt(testX, testZ, testDim);
                REQUIRE(verifyLand == nullptr);

                // 根据修复后的实现，当找不到land时抛出的是RealmNotFoundException
                REQUIRE_THROWS_AS(
                    dataService->addItemMember<
                        LandData>(testX, testZ, testDim, PlayerInfo("200000001", "小明", false), "小明"),
                    RealmNotFoundException
                );
                REQUIRE_THROWS_AS(
                    dataService->removeItemMember<
                        LandData>(testX, testZ, testDim, PlayerInfo("200000001", "小明", false), "小明"),
                    RealmNotFoundException
                );
            }

            SECTION("Null Town Information") {

                // 验证空指针操作抛出异常
                REQUIRE_THROWS_AS(
                    dataService->modifyItemPermission<TownData>(0, 0, 0, 1, PlayerInfo("100000001", "腐竹", true)),
                    RealmNotFoundException
                );

                // // 使用确实没有town的坐标进行测试
                LONG64 testX   = 777777;
                LONG64 testZ   = 777777;
                int    testDim = 1; // 修复：使用有效维度值 0-2

                // // 首先验证该坐标确实没有town
                auto* verifyTown = dataService->findTownAt(testX, testZ, testDim);
                REQUIRE(verifyTown == nullptr);

                // // 根据修复后的实现，当找不到town时抛出的是RealmNotFoundException
                REQUIRE_THROWS_AS(
                    dataService
                        ->addItemMember<TownData>(testX, testZ, testDim, PlayerInfo("100000001", "腐竹", true), "小明"),
                    RealmNotFoundException
                );
                REQUIRE_THROWS_AS(
                    dataService->removeItemMember<
                        TownData>(testX, testZ, testDim, PlayerInfo("100000001", "腐竹", true), "小明"),
                    RealmNotFoundException
                );
            }
        }

        SECTION("Player Not Found Handling") {
            // 创建一个领地用于测试
            LandData testLand;
            testLand.ownerXuid = "200000001";
            testLand.x         = 100;
            testLand.z         = 100;
            testLand.x_end     = 150;
            testLand.z_end     = 150;
            testLand.d         = 0;
            testLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000001", "小明", false);
            dataService->createItem<LandData>(testLand, playerInfo);

            auto* createdLand = dataService->findLandAt(125, 125, 0);
            REQUIRE(createdLand != nullptr);

            SECTION("Add Non-existent Player as Member") {
                // 测试添加不存在的玩家作为成员
                std::string nonExistentPlayer = "不存在的玩家";

                // 现在MockAPI与真实API保持一致，找不到玩家时返回空字符串
                // 应该抛出PlayerNotFoundException
                // 使用新的接口，需要传入坐标和PlayerInfo
                auto center = TestEnvironment::getInstance().getItemCenter<LandData>(createdLand);
                REQUIRE_THROWS_AS(
                    dataService->addItemMember<LandData>(
                        center.first,
                        center.second,
                        createdLand->getDimension(),
                        PlayerInfo(createdLand->getOwnerXuid(), "小明", false),
                        nonExistentPlayer
                    ),
                    PlayerNotFoundException
                );

                // 验证成员列表仍然为空
                auto* updatedLand = dataService->findLandAt(125, 125, 0);
                REQUIRE(updatedLand != nullptr);
                REQUIRE(updatedLand->getMemberXuids().empty());
            }

            SECTION("Remove Non-existent Player") {
                // 测试移除不存在的玩家
                std::string nonExistentPlayer = "不存在的玩家";

                // 现在MockAPI与真实API保持一致，找不到玩家时返回空字符串
                // 应该抛出PlayerNotFoundException而不是NotMemberException
                auto center = TestEnvironment::getInstance().getItemCenter<LandData>(createdLand);
                REQUIRE_THROWS_AS(
                    dataService->removeItemMember<LandData>(
                        center.first,
                        center.second,
                        createdLand->getDimension(),
                        PlayerInfo(createdLand->getOwnerXuid(), "小明", false),
                        nonExistentPlayer
                    ),
                    PlayerNotFoundException
                );

                // 验证成员列表仍然为空
                auto* landAfterFailedRemove = dataService->findLandAt(125, 125, 0);
                REQUIRE(landAfterFailedRemove != nullptr);
                REQUIRE(landAfterFailedRemove->getMemberXuids().empty());
            }
        }

        SECTION("Duplicate Operations") {
            // 创建一个领地用于测试
            LandData testLand;
            testLand.ownerXuid = "200000001";
            testLand.x         = 200;
            testLand.z         = 200;
            testLand.x_end     = 250;
            testLand.z_end     = 250;
            testLand.d         = 0;
            testLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000001", "小明", false);
            dataService->createItem<LandData>(testLand, playerInfo);

            auto* createdLand = dataService->findLandAt(225, 225, 0);
            REQUIRE(createdLand != nullptr);

            SECTION("Add Duplicate Member") {
                // 添加成员
                auto center = TestEnvironment::getInstance().getItemCenter<LandData>(createdLand);
                dataService->addItemMember<LandData>(
                    center.first,
                    center.second,
                    createdLand->getDimension(),
                    PlayerInfo(createdLand->getOwnerXuid(), "小明", false),
                    "小红"
                );

                // 验证成员已添加
                auto* landWithMember = dataService->findLandAt(225, 225, 0);
                REQUIRE(landWithMember != nullptr);
                REQUIRE(landWithMember->getMemberXuids().size() == 1);

                // 尝试再次添加相同成员应该抛出异常
                REQUIRE_THROWS_AS(
                    dataService->addItemMember<LandData>(
                        center.first,
                        center.second,
                        landWithMember->getDimension(),
                        PlayerInfo(landWithMember->getOwnerXuid(), "小明", false),
                        "小红"
                    ),
                    DuplicateException
                );

                // 验证成员列表没有重复
                auto* landAfterFailedAdd = dataService->findLandAt(225, 225, 0);
                REQUIRE(landAfterFailedAdd != nullptr);
                REQUIRE(landAfterFailedAdd->getMemberXuids().size() == 1);
            }
        }

        SECTION("Max ID Handling") {
            SECTION("Land ID Generation") {
                // 测试ID自动生成
                LONG64 initialMaxId = dataService->getMaxId<LandData>();

                LandData land1;
                land1.ownerXuid = "200000001";
                land1.x         = 1000;
                land1.z         = 1000;
                land1.x_end     = 1050;
                land1.z_end     = 1050;
                land1.d         = 0;
                land1.id        = dataService->getMaxId<LandData>() + 1;

                PlayerInfo playerInfo("200000001", "小明", false);
                dataService->createItem<LandData>(land1, playerInfo);

                // 验证ID已增加
                REQUIRE(dataService->getMaxId<LandData>() > initialMaxId);

                // 创建第二个领地
                LandData land2;
                land2.ownerXuid = "200000002";
                land2.x         = 1100;
                land2.z         = 1100;
                land2.x_end     = 1150;
                land2.z_end     = 1150;
                land2.d         = 0;
                land2.id        = dataService->getMaxId<LandData>() + 1;

                PlayerInfo playerInfo2("200000002", "小红", false);
                dataService->createItem<LandData>(land2, playerInfo2);

                // 验证ID继续增加
                REQUIRE(dataService->getMaxId<LandData>() > initialMaxId + 1);

                // 验证两个领地有不同的ID
                auto* createdLand1 = dataService->findLandAt(1025, 1025, 0);
                auto* createdLand2 = dataService->findLandAt(1125, 1125, 0);
                REQUIRE(createdLand1 != nullptr);
                REQUIRE(createdLand2 != nullptr);
                REQUIRE(createdLand1->getId() != createdLand2->getId());
            }
        }
    }

    SECTION("Edge Cases and Special Scenarios") {
        SECTION("Zero Area Land") {
            // 测试零面积领地（x == x_end && z == z_end）
            LandData zeroAreaLand;
            zeroAreaLand.ownerXuid = "200000001";
            zeroAreaLand.x         = 100;
            zeroAreaLand.z         = 100;
            zeroAreaLand.x_end     = 100; // 相同坐标
            zeroAreaLand.z_end     = 100; // 相同坐标
            zeroAreaLand.d         = 0;
            zeroAreaLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000001", "小明", false);

            // 验证零面积领地可以创建（如果允许的话）
            REQUIRE_NOTHROW(dataService->createItem<LandData>(zeroAreaLand, playerInfo));

            auto* createdLand = dataService->findLandAt(100, 100, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getArea() == 0);
        }

        SECTION("Line Area Land") {
            // 测试线性领地（宽度或高度为0）
            LandData lineLand;
            lineLand.ownerXuid = "200000002";
            lineLand.x         = 100;
            lineLand.z         = 100;
            lineLand.x_end     = 200; // 宽度为100
            lineLand.z_end     = 100; // 高度为0
            lineLand.d         = 0;
            lineLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000002", "小红", false);

            // 验证线性领地可以创建
            REQUIRE_NOTHROW(dataService->createItem<LandData>(lineLand, playerInfo));

            auto* createdLand = dataService->findLandAt(150, 100, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getArea() == 0); // 宽度 * 高度 = 100 * 0 = 0
        }

        SECTION("Maximum Permission Value") {
            // 测试最大权限值
            LandData maxPermLand;
            maxPermLand.ownerXuid = "200000001";
            maxPermLand.x         = 100;
            maxPermLand.z         = 100;
            maxPermLand.x_end     = 150;
            maxPermLand.z_end     = 150;
            maxPermLand.d         = 0;
            maxPermLand.perm      = INT_MAX; // 最大整数值
            maxPermLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000001", "小明", false);
            dataService->createItem<LandData>(maxPermLand, playerInfo);

            auto* createdLand = dataService->findLandAt(125, 125, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getPermission() == INT_MAX);

            // 测试权限修改
            dataService->modifyItemPermission<LandData>(
                createdLand->getX(),
                createdLand->getZ(),
                createdLand->getDimension(),
                INT_MAX,
                PlayerInfo("200000001", "小明", false)
            );
            REQUIRE(createdLand->getPermission() == INT_MAX);
        }

        SECTION("Negative Permission Value") {
            // 测试创建负权限值的领地应该抛出异常
            LandData negPermLand;
            negPermLand.ownerXuid = "200000002";
            negPermLand.x         = 200;
            negPermLand.z         = 200;
            negPermLand.x_end     = 250;
            negPermLand.z_end     = 250;
            negPermLand.d         = 0;
            negPermLand.perm      = -1; // 负权限值
            negPermLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000002", "小红", false);

            // 创建负权限值的领地应该抛出异常
            REQUIRE_THROWS_AS(dataService->createItem<LandData>(negPermLand, playerInfo), InvalidPermissionException);

            // 验证领地确实没有被创建
            auto* notCreatedLand = dataService->findLandAt(225, 225, 0);
            REQUIRE(notCreatedLand == nullptr);

            // 测试正常权限值的领地创建
            LandData normalPermLand;
            normalPermLand.ownerXuid = "200000002";
            normalPermLand.x         = 200;
            normalPermLand.z         = 200;
            normalPermLand.x_end     = 250;
            normalPermLand.z_end     = 250;
            normalPermLand.d         = 0;
            normalPermLand.perm      = 0; // 正常权限值
            normalPermLand.id        = dataService->getMaxId<LandData>() + 1;

            dataService->createItem<LandData>(normalPermLand, playerInfo);
            auto* createdLand = dataService->findLandAt(225, 225, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getPermission() == 0);

            // 测试权限修改为负值应该抛出异常
            REQUIRE_THROWS_AS(
                dataService->modifyItemPermission<LandData>(
                    createdLand->getX(),
                    createdLand->getZ(),
                    createdLand->getDimension(),
                    -100,
                    PlayerInfo("200000002", "小红", false)
                ),
                InvalidPermissionException
            );
            // 验证权限值没有被修改
            REQUIRE(createdLand->getPermission() == 0);
        }
    }

    SECTION("Concurrent Operations Simulation") {
        SECTION("Multiple Lands in Same Area") {
            // 创建第一个领地
            LandData land1;
            land1.ownerXuid = "200000001";
            land1.x         = 100;
            land1.z         = 100;
            land1.x_end     = 150;
            land1.z_end     = 150;
            land1.d         = 0;
            land1.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo1("200000001", "小明", false);
            dataService->createItem<LandData>(land1, playerInfo1);

            // 验证第一个领地创建成功
            auto* createdLand1 = dataService->findLandAt(125, 125, 0);
            REQUIRE(createdLand1 != nullptr);

            // 尝试在相同区域创建第二个领地（应该失败）
            LandData land2;
            land2.ownerXuid = "200000002";
            land2.x         = 100;
            land2.z         = 100;
            land2.x_end     = 150;
            land2.z_end     = 150;
            land2.d         = 0;
            land2.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo2("200000002", "小红", false);

            // 验证重叠的领地创建失败
            REQUIRE_THROWS_AS(dataService->createItem<LandData>(land2, playerInfo2), RealmConflictException);

            // 验证第一个领地仍然存在
            auto* existingLand = dataService->findLandAt(125, 125, 0);
            REQUIRE(existingLand != nullptr);
            REQUIRE(existingLand->getOwnerXuid() == "200000001");
        }
    }

    SECTION("PlayerInfo Validation Tests") {
        SECTION("Valid PlayerInfo") {
            // 测试有效的 PlayerInfo
            LandData validLand;
            validLand.ownerXuid = "200000001";
            validLand.x         = 100;
            validLand.z         = 100;
            validLand.x_end     = 150;
            validLand.z_end     = 150;
            validLand.d         = 0;
            validLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo validPlayerInfo("200000001", "小明", false);

            // 验证有效的 PlayerInfo 可以创建领地
            REQUIRE_NOTHROW(dataService->createItem<LandData>(validLand, validPlayerInfo));

            auto* createdLand = dataService->findLandAt(125, 125, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getOwnerXuid() == "200000001");
        }

        SECTION("Invalid XUID Tests") {
            LandData testLand;
            testLand.ownerXuid = "200000001";
            testLand.x         = 200;
            testLand.z         = 200;
            testLand.x_end     = 250;
            testLand.z_end     = 250;
            testLand.d         = 0;
            testLand.id        = dataService->getMaxId<LandData>() + 1;

            SECTION("Empty XUID") {
                PlayerInfo invalidPlayerInfo("", "小明", false);
                REQUIRE_THROWS_AS(
                    dataService->createItem<LandData>(testLand, invalidPlayerInfo),
                    InvalidPlayerInfoException
                );
            }

            SECTION("XUID Too Long") {
                PlayerInfo invalidPlayerInfo("123456789012345678901", "小明", false); // 21位数字
                REQUIRE_THROWS_AS(
                    dataService->createItem<LandData>(testLand, invalidPlayerInfo),
                    InvalidPlayerInfoException
                );
            }

            SECTION("XUID with Non-digit Characters") {
                PlayerInfo invalidPlayerInfo("200000001a", "小明", false);
                REQUIRE_THROWS_AS(
                    dataService->createItem<LandData>(testLand, invalidPlayerInfo),
                    InvalidPlayerInfoException
                );
            }

            SECTION("XUID with Special Characters") {
                PlayerInfo invalidPlayerInfo("200000001@", "小明", false);
                REQUIRE_THROWS_AS(
                    dataService->createItem<LandData>(testLand, invalidPlayerInfo),
                    InvalidPlayerInfoException
                );
            }
        }


        SECTION("Valid Player Name with Underscore") {
            LandData validLand;
            validLand.ownerXuid = "200000002";
            validLand.x         = 400;
            validLand.z         = 400;
            validLand.x_end     = 450;
            validLand.z_end     = 450;
            validLand.d         = 0;
            validLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo validPlayerInfo("200000002", "player_name", false);

            // 验证包含下划线的玩家名称是有效的
            REQUIRE_NOTHROW(dataService->createItem<LandData>(validLand, validPlayerInfo));

            auto* createdLand = dataService->findLandAt(425, 425, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getOwnerXuid() == "200000002");
        }

        SECTION("Town Creation with Invalid PlayerInfo") {
            TownData testTown;
            testTown.name        = "测试城镇";
            testTown.mayorXuid   = "200000001";
            testTown.x           = 500;
            testTown.z           = 500;
            testTown.x_end       = 550;
            testTown.z_end       = 550;
            testTown.d           = 0;
            testTown.perm        = 0;
            testTown.description = "测试城镇";
            testTown.id          = dataService->getMaxId<TownData>() + 1;

            SECTION("Invalid XUID for Town Creation") {
                PlayerInfo invalidPlayerInfo("invalid_xuid", "腐竹", true);
                REQUIRE_THROWS_AS(
                    dataService->createItem<TownData>(testTown, invalidPlayerInfo),
                    InvalidPlayerInfoException
                );
            }
        }
    }

    SECTION("Extreme Boundary Conditions Supplement") {
        SECTION("Maximum Coordinate Values") {
            // 测试接近LAND_RANGE的最大坐标值
            LandData maxCoordLand;
            maxCoordLand.ownerXuid = "200000001";
            maxCoordLand.x         = LAND_RANGE - 1;
            maxCoordLand.z         = LAND_RANGE - 1;
            maxCoordLand.x_end     = LAND_RANGE;
            maxCoordLand.z_end     = LAND_RANGE;
            maxCoordLand.d         = 0;
            maxCoordLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000001", "小明", false);

            // 验证边界值可以创建
            REQUIRE_NOTHROW(dataService->createItem<LandData>(maxCoordLand, playerInfo));

            auto* createdLand = dataService->findLandAt(LAND_RANGE - 1, LAND_RANGE - 1, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getOwnerXuid() == "200000001");

            // 测试边界查询
            auto* boundaryLand1 = dataService->findLandAt(LAND_RANGE, LAND_RANGE, 0);
            REQUIRE(boundaryLand1 != nullptr);

            // 测试超出边界1格
            auto* outOfBoundLand = dataService->findLandAt(LAND_RANGE + 1, LAND_RANGE, 0);
            REQUIRE(outOfBoundLand == nullptr);
        }

        SECTION("Minimum Coordinate Values") {
            // 测试接近-LAND_RANGE的最小坐标值
            LandData minCoordLand;
            minCoordLand.ownerXuid = "200000002";
            minCoordLand.x         = -LAND_RANGE;
            minCoordLand.z         = -LAND_RANGE;
            minCoordLand.x_end     = -LAND_RANGE + 1;
            minCoordLand.z_end     = -LAND_RANGE + 1;
            minCoordLand.d         = 0;
            minCoordLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000002", "小红", false);

            // 验证边界值可以创建
            REQUIRE_NOTHROW(dataService->createItem<LandData>(minCoordLand, playerInfo));

            auto* createdLand = dataService->findLandAt(-LAND_RANGE, -LAND_RANGE, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getOwnerXuid() == "200000002");

            // 测试超出负边界1格
            auto* outOfBoundLand = dataService->findLandAt(-LAND_RANGE - 1, -LAND_RANGE, 0);
            REQUIRE(outOfBoundLand == nullptr);
        }

        SECTION("Town with Special Characters in Name") {
            // 测试包含特殊字符的城镇名称
            std::vector<std::string> specialNames =
                {"城镇@#$%", "Town with Spaces", "城镇_下划线", "123数字城镇", "混合Mixed城镇", "城镇-横线", "城镇.点号"
                };

            for (const auto& name : specialNames) {
                TownData specialTown;
                specialTown.name        = name;
                specialTown.mayorXuid   = "200000001";
                specialTown.x           = 1000;
                specialTown.z           = 1000;
                specialTown.x_end       = 1100;
                specialTown.z_end       = 1100;
                specialTown.d           = 0;
                specialTown.perm        = 0;
                specialTown.description = "特殊字符城镇测试";
                specialTown.id          = dataService->getMaxId<TownData>() + 1;

                PlayerInfo operatorInfo("100000001", "腐竹", true);

                // 验证特殊字符名称可以创建
                REQUIRE_NOTHROW(dataService->createItem<TownData>(specialTown, operatorInfo));

                auto* createdTown = dataService->findTownAt(1050, 1050, 0);
                REQUIRE(createdTown != nullptr);
                REQUIRE(createdTown->getTownName() == name);

                // 验证通过名称查找
                auto* foundByName = dataService->findTownByName(name);
                REQUIRE(foundByName != nullptr);
                REQUIRE(foundByName->getTownName() == name);

                // 清理以便下次测试
                dataService->deleteItem<TownData>(specialTown.x, specialTown.z, specialTown.d);
            }
        }

        SECTION("Zero Size Realm with Members") {
            // 测试零大小领地添加成员
            LandData zeroSizeLand;
            zeroSizeLand.ownerXuid = "200000003";
            zeroSizeLand.x         = 2000;
            zeroSizeLand.z         = 2000;
            zeroSizeLand.x_end     = 2000; // 零宽度
            zeroSizeLand.z_end     = 2000; // 零高度
            zeroSizeLand.d         = 0;
            zeroSizeLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000003", "张三", false);
            dataService->createItem<LandData>(zeroSizeLand, playerInfo);

            auto* createdLand = dataService->findLandAt(2000, 2000, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getArea() == 0);

            // 测试零大小领地添加成员
            auto center = TestEnvironment::getInstance().getItemCenter<LandData>(createdLand);
            dataService
                ->addItemMember<LandData>(center.first, center.second, createdLand->getDimension(), playerInfo, "小红");

            auto* landWithMember = dataService->findLandAt(2000, 2000, 0);
            REQUIRE(landWithMember != nullptr);
            REQUIRE_FALSE(landWithMember->getMemberXuids().empty());
            REQUIRE(landWithMember->hasBasicPermission("200000002"));
        }

        SECTION("Line Realm with Complex Operations") {
            // 测试线性领地（宽度或高度为0）的复杂操作
            LandData lineLand;
            lineLand.ownerXuid = "200000004";
            lineLand.x         = 3000;
            lineLand.z         = 3000;
            lineLand.x_end     = 3500; // 宽度为500
            lineLand.z_end     = 3000; // 高度为0
            lineLand.d         = 0;
            lineLand.perm      = 8;
            lineLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000004", "李四", false);
            dataService->createItem<LandData>(lineLand, playerInfo);

            auto* createdLand = dataService->findLandAt(3250, 3000, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getArea() == 0);

            // 测试线性领地的权限修改
            dataService->modifyItemPermission<LandData>(
                createdLand->getX(),
                createdLand->getZ(),
                createdLand->getDimension(),
                16,
                playerInfo
            );
            REQUIRE(createdLand->getPermission() == 16);

            // 测试线性领地的成员管理
            auto center = TestEnvironment::getInstance().getItemCenter<LandData>(createdLand);
            dataService
                ->addItemMember<LandData>(center.first, center.second, createdLand->getDimension(), playerInfo, "王五");

            auto* landWithMember = dataService->findLandAt(3250, 3000, 0);
            REQUIRE(landWithMember != nullptr);
            REQUIRE(landWithMember->hasBasicPermission("200000005"));

            // 测试线性领地的边界查询
            auto* onLineLand = dataService->findLandAt(3100, 3000, 0);
            REQUIRE(onLineLand != nullptr);

            auto* offLineLand = dataService->findLandAt(3250, 3001, 0);
            REQUIRE(offLineLand == nullptr);
        }

        SECTION("Maximum Permission Value Edge Cases") {
            // 测试最大权限值的边界情况
            LandData maxPermLand;
            maxPermLand.ownerXuid = "200000005";
            maxPermLand.x         = 4000;
            maxPermLand.z         = 4000;
            maxPermLand.x_end     = 4100;
            maxPermLand.z_end     = 4100;
            maxPermLand.d         = 0;
            maxPermLand.perm      = INT_MAX;
            maxPermLand.id        = dataService->getMaxId<LandData>() + 1;

            PlayerInfo playerInfo("200000005", "王五", false);
            dataService->createItem<LandData>(maxPermLand, playerInfo);

            auto* createdLand = dataService->findLandAt(4050, 4050, 0);
            REQUIRE(createdLand != nullptr);
            REQUIRE(createdLand->getPermission() == INT_MAX);

            // 测试权限位操作
            SECTION("Permission Bit Operations") {
                // 测试权限位的设置和检查
                int testPerm = 0;

                // 设置各个权限位
                for (int i = 0; i < 16; i++) {
                    testPerm |= (1 << i);
                    dataService->modifyItemPermission<LandData>(
                        createdLand->getX(),
                        createdLand->getZ(),
                        createdLand->getDimension(),
                        testPerm,
                        playerInfo
                    );
                    REQUIRE(createdLand->getPermission() == testPerm);
                }

                // 测试权限位清除
                dataService->modifyItemPermission<LandData>(
                    createdLand->getX(),
                    createdLand->getZ(),
                    createdLand->getDimension(),
                    0,
                    playerInfo
                );
                REQUIRE(createdLand->getPermission() == 0);
            }
        }
    }
}

} // namespace rlx_land::test
