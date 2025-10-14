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

// Town JSON文件验证辅助函数
namespace TownJsonTestUtils {
// 获取城镇JSON文件的基础路径
std::string getTownsBaseDir() { return "../RLXModeResources/data/towns"; }

// 从JSON文件加载城镇数据
std::vector<TownData> loadTownsFromJson() {
    std::vector<TownData> towns;
    std::filesystem::path filePath = std::filesystem::path(getTownsBaseDir()) / "towns.json";

    if (!std::filesystem::exists(filePath)) {
        return towns; // 返回空列表
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return towns;
    }

    try {
        nlohmann::json json;
        file >> json;
        file.close();

        if (json.is_array()) {
            for (const auto& item : json) {
                TownData town;
                town.id          = item.value("id", 0LL);
                town.name        = item.value("name", "");
                town.mayorXuid   = item.value("mayorXuid", "");
                town.x           = item.value("x", 0);
                town.z           = item.value("z", 0);
                town.x_end       = item.value("x_end", 0);
                town.z_end       = item.value("z_end", 0);
                town.d           = item.value("d", 0);
                town.perm        = item.value("perm", 0);
                town.description = item.value("description", "");
                town.memberXuids = item.value("memberXuids", std::vector<std::string>());
                towns.push_back(town);
            }
        }
    } catch (const std::exception&) {
        // 解析失败，返回空列表
    }

    return towns;
}

// 验证城镇是否存在于JSON文件中
bool verifyTownFileExists(const std::string& townName) {
    auto towns = loadTownsFromJson();
    return std::ranges::any_of(towns, [&](const TownData& town) { return town.name == townName; });
}

// 验证城镇数据是否匹配
bool verifyTownData(const TownData& expected, const TownData& actual) {
    return expected.id == actual.id && expected.name == actual.name && expected.mayorXuid == actual.mayorXuid
        && expected.x == actual.x && expected.z == actual.z && expected.x_end == actual.x_end
        && expected.z_end == actual.z_end && expected.d == actual.d && expected.perm == actual.perm
        && expected.description == actual.description && expected.memberXuids == actual.memberXuids;
}

// 验证城镇文件中是否包含指定的城镇
bool verifyTownExistsInJson(const std::string& /*townName*/, const TownData& expectedTown) {
    auto towns = loadTownsFromJson();

    return std::ranges::any_of(towns, [&](const TownData& town) { return verifyTownData(expectedTown, town); });
}

// 验证城镇文件中的城镇数量
bool verifyTownCountInJson(size_t expectedCount) {
    auto towns = loadTownsFromJson();
    return towns.size() == expectedCount;
}

// 验证城镇文件是否被删除
bool verifyTownFileDeleted(const std::string& townName) { return !verifyTownFileExists(townName); }
} // namespace TownJsonTestUtils

TEST_CASE("Town Management Integration Tests", "[town][integration]") {

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

    SECTION("Basic Town Creation via DataService") {
        TownData townData;
        townData.name        = "测试城镇";
        townData.mayorXuid   = "200000001";
        townData.x           = 100;
        townData.z           = 100;
        townData.x_end       = 200;
        townData.z_end       = 200;
        townData.d           = 0;
        townData.perm        = 0;
        townData.description = "测试用城镇";
        townData.id          = dataService->getMaxId<TownData>() + 1;

        PlayerInfo operatorInfo("100000001", "腐竹", true);
        dataService->createItem<TownData>(townData, operatorInfo);

        SECTION("Town Data Validation via DataService") {
            auto* town = dataService->findTownAt(150, 150, 0);
            REQUIRE(town != nullptr);
            REQUIRE(town->getTownName() == "测试城镇");
            REQUIRE(town->getMayorXuid() == "200000001");
            REQUIRE(town->getX() == 100);
            REQUIRE(town->getZ() == 100);
            REQUIRE(town->getXEnd() == 200);
            REQUIRE(town->getZEnd() == 200);
            REQUIRE(town->getOwnerName() == "小明");
            REQUIRE_FALSE(town->isOwner("小红"));

            // 验证JSON文件是否正确生成
            SECTION("JSON File Validation after Town Creation") {
                // 验证城镇文件是否存在
                REQUIRE(TownJsonTestUtils::verifyTownFileExists("测试城镇"));

                // 验证城镇数据是否正确保存到JSON文件
                REQUIRE(TownJsonTestUtils::verifyTownExistsInJson("测试城镇", townData));

                // 验证城镇数量是否正确
                REQUIRE(TownJsonTestUtils::verifyTownCountInJson(1));
            }
        }
    }

    SECTION("Town Creation Permission Validation") {
        SECTION("Non-operator Cannot Create Town") {
            TownData townData;
            townData.name        = "非法城镇";
            townData.mayorXuid   = "200000002";
            townData.x           = 300;
            townData.z           = 300;
            townData.x_end       = 400;
            townData.z_end       = 400;
            townData.d           = 0;
            townData.perm        = 0;
            townData.description = "非法创建的城镇";
            townData.id          = dataService->getMaxId<TownData>() + 1;

            PlayerInfo nonOperatorInfo("200000002", "小红", false);

            // 验证非腐竹无法创建城镇
            REQUIRE_THROWS_AS(dataService->createItem<TownData>(townData, nonOperatorInfo), RealmPermissionException);

            // 验证城镇没有被创建
            auto* town = dataService->findTownAt(350, 350, 0);
            REQUIRE(town == nullptr);
        }

        SECTION("Operator Can Create Town") {
            TownData townData;
            townData.name        = "合法城镇";
            townData.mayorXuid   = "200000003";
            townData.x           = 500;
            townData.z           = 500;
            townData.x_end       = 600;
            townData.z_end       = 600;
            townData.d           = 0;
            townData.perm        = 0;
            townData.description = "腐竹创建的城镇";
            townData.id          = dataService->getMaxId<TownData>() + 1;

            PlayerInfo operatorInfo("100000001", "腐竹", true);

            // 验证腐竹可以创建城镇
            REQUIRE_NOTHROW(dataService->createItem<TownData>(townData, operatorInfo));

            // 验证城镇创建成功
            auto* town = dataService->findTownAt(550, 550, 0);
            REQUIRE(town != nullptr);
            REQUIRE(town->getTownName() == "合法城镇");
        }
    }

    SECTION("Town Conflict Detection") {
        // 创建第一个城镇
        TownData firstTown;
        firstTown.name        = "第一个城镇";
        firstTown.mayorXuid   = "200000001";
        firstTown.x           = 100;
        firstTown.z           = 100;
        firstTown.x_end       = 200;
        firstTown.z_end       = 200;
        firstTown.d           = 0;
        firstTown.perm        = 0;
        firstTown.description = "第一个测试城镇";
        firstTown.id          = dataService->getMaxId<TownData>() + 1;

        PlayerInfo operatorInfo("100000001", "腐竹", true);
        dataService->createItem<TownData>(firstTown, operatorInfo);

        // 验证第一个城镇创建成功
        auto* createdTown = dataService->findTownAt(150, 150, 0);
        REQUIRE(createdTown != nullptr);
        REQUIRE(createdTown->getTownName() == "第一个城镇");

        // 尝试创建与第一个城镇完全重叠的城镇
        TownData conflictingTown1;
        conflictingTown1.name        = "冲突城镇1";
        conflictingTown1.mayorXuid   = "200000002";
        conflictingTown1.x           = 100;
        conflictingTown1.z           = 100;
        conflictingTown1.x_end       = 200;
        conflictingTown1.z_end       = 200;
        conflictingTown1.d           = 0;
        conflictingTown1.perm        = 0;
        conflictingTown1.description = "完全重叠的城镇";
        conflictingTown1.id          = dataService->getMaxId<TownData>() + 1;

        // 验证完全重叠的城镇创建失败
        REQUIRE_THROWS_AS(dataService->createItem<TownData>(conflictingTown1, operatorInfo), RealmConflictException);

        // 尝试创建与第一个城镇部分重叠的城镇
        TownData conflictingTown2;
        conflictingTown2.name        = "冲突城镇2";
        conflictingTown2.mayorXuid   = "200000002";
        conflictingTown2.x           = 150;
        conflictingTown2.z           = 150;
        conflictingTown2.x_end       = 250;
        conflictingTown2.z_end       = 250;
        conflictingTown2.d           = 0;
        conflictingTown2.perm        = 0;
        conflictingTown2.description = "部分重叠的城镇";
        conflictingTown2.id          = dataService->getMaxId<TownData>() + 1;

        // 验证部分重叠的城镇创建失败
        REQUIRE_THROWS_AS(dataService->createItem<TownData>(conflictingTown2, operatorInfo), RealmConflictException);

        // 验证完全不重叠的城镇可以成功创建
        TownData nonConflictingTown;
        nonConflictingTown.name        = "不冲突城镇";
        nonConflictingTown.mayorXuid   = "200000002";
        nonConflictingTown.x           = 300;
        nonConflictingTown.z           = 300;
        nonConflictingTown.x_end       = 400;
        nonConflictingTown.z_end       = 400;
        nonConflictingTown.d           = 0;
        nonConflictingTown.perm        = 0;
        nonConflictingTown.description = "不重叠的城镇";
        nonConflictingTown.id          = dataService->getMaxId<TownData>() + 1;

        // 验证不重叠的城镇创建成功
        REQUIRE_NOTHROW(dataService->createItem<TownData>(nonConflictingTown, operatorInfo));

        auto* createdNonConflictingTown = dataService->findTownAt(350, 350, 0);
        REQUIRE(createdNonConflictingTown != nullptr);
        REQUIRE(createdNonConflictingTown->getTownName() == "不冲突城镇");
    }

    SECTION("Town Deletion via DataService") {
        // 创建一个城镇用于删除测试
        TownData townToDelete;
        townToDelete.name        = "待删除城镇";
        townToDelete.mayorXuid   = "200000001";
        townToDelete.x           = 700;
        townToDelete.z           = 700;
        townToDelete.x_end       = 800;
        townToDelete.z_end       = 800;
        townToDelete.d           = 0;
        townToDelete.perm        = 0;
        townToDelete.description = "用于删除测试的城镇";
        townToDelete.id          = dataService->getMaxId<TownData>() + 1;

        PlayerInfo operatorInfo("100000001", "腐竹", true);
        dataService->createItem<TownData>(townToDelete, operatorInfo);

        // 验证城镇创建成功
        auto* createdTown = dataService->findTownAt(750, 750, 0);
        REQUIRE(createdTown != nullptr);
        REQUIRE(createdTown->getTownName() == "待删除城镇");

        SECTION("Basic Town Deletion") {
            // 删除城镇
            dataService->deleteItem<TownData>(townToDelete);

            // 验证城镇已被删除
            auto* deletedTown = dataService->findTownAt(750, 750, 0);
            REQUIRE(deletedTown == nullptr);

            // 验证城镇范围内的其他点也被删除
            auto* deletedTown2 = dataService->findTownAt(700, 700, 0);
            REQUIRE(deletedTown2 == nullptr);
            auto* deletedTown3 = dataService->findTownAt(800, 800, 0);
            REQUIRE(deletedTown3 == nullptr);
        }

        SECTION("Multiple Towns Deletion") {
            // 创建多个城镇
            std::vector<TownData> townsToDelete;

            for (int i = 0; i < 3; i++) {
                TownData town;
                town.name        = "城镇" + std::to_string(i);
                town.mayorXuid   = "20000000" + std::to_string(1 + i);
                town.x           = 900 + i * 200;
                town.z           = 900 + i * 200;
                town.x_end       = 1000 + i * 200;
                town.z_end       = 1000 + i * 200;
                town.d           = 0;
                town.perm        = 0;
                town.description = "用于批量删除测试的城镇" + std::to_string(i);
                town.id          = dataService->getMaxId<TownData>() + 1 + i;

                dataService->createItem<TownData>(town, operatorInfo);
                townsToDelete.push_back(town);
            }

            // 验证所有城镇都创建成功
            for (int i = 0; i < 3; i++) {
                auto* town = dataService->findTownAt(950 + i * 200, 950 + i * 200, 0);
                REQUIRE(town != nullptr);
                REQUIRE(town->getTownName() == "城镇" + std::to_string(i));
            }

            // 删除所有城镇
            for (const auto& town : townsToDelete) {
                dataService->deleteItem<TownData>(town);
            }

            // 验证所有城镇都被删除
            for (int i = 0; i < 3; i++) {
                auto* deletedTown = dataService->findTownAt(950 + i * 200, 950 + i * 200, 0);
                REQUIRE(deletedTown == nullptr);
            }

            // 原始城镇应该仍然存在
            auto* originalTown = dataService->findTownAt(750, 750, 0);
            REQUIRE(originalTown != nullptr);
            REQUIRE(originalTown->getTownName() == "待删除城镇");
        }
    }

    SECTION("Town Permission Modification via DataService") {
        // 创建一个城镇用于权限修改测试
        TownData townForPermTest;
        townForPermTest.name        = "权限测试城镇";
        townForPermTest.mayorXuid   = "200000001";
        townForPermTest.x           = 1300;
        townForPermTest.z           = 1300;
        townForPermTest.x_end       = 1400;
        townForPermTest.z_end       = 1400;
        townForPermTest.d           = 0;
        townForPermTest.perm        = 0; // 初始权限为0
        townForPermTest.description = "用于权限测试的城镇";
        townForPermTest.id          = dataService->getMaxId<TownData>() + 1;

        PlayerInfo operatorInfo("100000001", "腐竹", true);
        dataService->createItem<TownData>(townForPermTest, operatorInfo);

        // 验证城镇创建成功
        auto* createdTown = dataService->findTownAt(1350, 1350, 0);
        REQUIRE(createdTown != nullptr);
        REQUIRE(createdTown->getPermission() == 0);

        SECTION("Basic Permission Modification") {
            // 修改权限为1
            dataService->modifyItemPermission<TownData>(createdTown, 1);

            // 验证权限已修改
            auto* updatedTown = dataService->findTownAt(1350, 1350, 0);
            REQUIRE(updatedTown != nullptr);
            REQUIRE(updatedTown->getPermission() == 1);

            // 修改权限为2
            dataService->modifyItemPermission<TownData>(updatedTown, 2);

            // 验证权限再次修改
            auto* finalTown = dataService->findTownAt(1350, 1350, 0);
            REQUIRE(finalTown != nullptr);
            REQUIRE(finalTown->getPermission() == 2);

            // 验证JSON文件状态
            SECTION("JSON File Validation after Permission Modification") {
                // 更新预期数据的权限值
                townForPermTest.perm = 2;
                REQUIRE(TownJsonTestUtils::verifyTownExistsInJson("权限测试城镇", townForPermTest));
                REQUIRE(TownJsonTestUtils::verifyTownCountInJson(1));
            }
        }

        SECTION("Permission Modification with Different Values") {
            // 测试不同的权限值
            std::vector<int> testPerms = {1, 2, 3, 4, 5, 10, 100, 0};

            for (int perm : testPerms) {
                dataService->modifyItemPermission<TownData>(createdTown, perm);

                auto* town = dataService->findTownAt(1350, 1350, 0);
                REQUIRE(town != nullptr);
                REQUIRE(town->getPermission() == perm);

                // 更新预期数据
                townForPermTest.perm = perm;
                REQUIRE(TownJsonTestUtils::verifyTownExistsInJson("权限测试城镇", townForPermTest));
            }
        }
    }

    SECTION("Town Member Management via DataService") {
        // 创建一个城镇用于成员管理测试
        TownData townForMemberTest;
        townForMemberTest.name        = "成员测试城镇";
        townForMemberTest.mayorXuid   = "200000001";
        townForMemberTest.x           = 1500;
        townForMemberTest.z           = 1500;
        townForMemberTest.x_end       = 1600;
        townForMemberTest.z_end       = 1600;
        townForMemberTest.d           = 0;
        townForMemberTest.perm        = 0;
        townForMemberTest.description = "用于成员测试的城镇";
        townForMemberTest.id          = dataService->getMaxId<TownData>() + 1;

        PlayerInfo operatorInfo("100000001", "腐竹", true);
        dataService->createItem<TownData>(townForMemberTest, operatorInfo);

        // 验证城镇创建成功
        auto* createdTown = dataService->findTownAt(1550, 1550, 0);
        REQUIRE(createdTown != nullptr);
        REQUIRE(createdTown->getMemberXuids().empty());

        SECTION("Add Member to Town") {
            // 添加成员
            dataService->addItemMember<TownData>(createdTown, "小红");

            // 验证成员已添加
            auto* updatedTown = dataService->findTownAt(1550, 1550, 0);
            REQUIRE(updatedTown != nullptr);
            REQUIRE_FALSE(updatedTown->getMemberXuids().empty());
            REQUIRE(
                std::find(updatedTown->getMemberXuids().begin(), updatedTown->getMemberXuids().end(), "200000002")
                != updatedTown->getMemberXuids().end()
            );

            // 验证JSON文件状态
            SECTION("JSON File Validation after Adding Member") {
                // 更新预期数据
                townForMemberTest.memberXuids = {"200000002"};
                REQUIRE(TownJsonTestUtils::verifyTownExistsInJson("成员测试城镇", townForMemberTest));
                REQUIRE(TownJsonTestUtils::verifyTownCountInJson(1));
            }
        }

        SECTION("Add Multiple Members to Town") {
            // 添加多个成员
            std::vector<std::string> membersToAdd = {"小红", "张三", "李四", "王五"};

            for (const auto& memberName : membersToAdd) {
                dataService->addItemMember<TownData>(createdTown, memberName);
            }

            // 验证所有成员都已添加
            auto* townWithMembers = dataService->findTownAt(1550, 1550, 0);
            REQUIRE(townWithMembers != nullptr);
            REQUIRE(townWithMembers->getMemberXuids().size() == membersToAdd.size());

            // 验证每个成员都存在
            for (const auto& memberName : membersToAdd) {
                std::string memberXuid = rlx_land::LeviLaminaAPI::getXuidByPlayerName(memberName);
                REQUIRE(
                    std::find(
                        townWithMembers->getMemberXuids().begin(),
                        townWithMembers->getMemberXuids().end(),
                        memberXuid
                    )
                    != townWithMembers->getMemberXuids().end()
                );
            }

            // 验证JSON文件状态
            SECTION("JSON File Validation after Adding Multiple Members") {
                // 更新预期数据
                townForMemberTest.memberXuids = {"200000002", "200000003", "200000004", "200000005"};
                REQUIRE(TownJsonTestUtils::verifyTownExistsInJson("成员测试城镇", townForMemberTest));
                REQUIRE(TownJsonTestUtils::verifyTownCountInJson(1));
            }
        }

        SECTION("Remove Member from Town") {
            // 先添加成员
            dataService->addItemMember<TownData>(createdTown, "小红");
            dataService->addItemMember<TownData>(createdTown, "张三");

            // 验证成员已添加
            auto* townWithMembers = dataService->findTownAt(1550, 1550, 0);
            REQUIRE(townWithMembers != nullptr);
            REQUIRE(townWithMembers->getMemberXuids().size() == 2);

            // 移除一个成员
            dataService->removeItemMember<TownData>(townWithMembers, "小红");

            // 验证成员已移除
            auto* townAfterRemoval = dataService->findTownAt(1550, 1550, 0);
            REQUIRE(townAfterRemoval != nullptr);
            REQUIRE(townAfterRemoval->getMemberXuids().size() == 1);
            REQUIRE(
                std::find(
                    townAfterRemoval->getMemberXuids().begin(),
                    townAfterRemoval->getMemberXuids().end(),
                    "200000002"
                )
                == townAfterRemoval->getMemberXuids().end()
            );
            REQUIRE(
                std::find(
                    townAfterRemoval->getMemberXuids().begin(),
                    townAfterRemoval->getMemberXuids().end(),
                    "200000003"
                )
                != townAfterRemoval->getMemberXuids().end()
            );

            // 验证JSON文件状态
            SECTION("JSON File Validation after Removing Member") {
                // 更新预期数据
                townForMemberTest.memberXuids = {"200000003"};
                REQUIRE(TownJsonTestUtils::verifyTownExistsInJson("成员测试城镇", townForMemberTest));
                REQUIRE(TownJsonTestUtils::verifyTownCountInJson(1));
            }
        }

        SECTION("Member Management Edge Cases") {
            SECTION("Add Already Existing Member") {
                // 添加成员
                dataService->addItemMember<TownData>(createdTown, "小红");

                // 验证成员已添加
                auto* townWithMember = dataService->findTownAt(1550, 1550, 0);
                REQUIRE(townWithMember != nullptr);
                REQUIRE(townWithMember->getMemberXuids().size() == 1);

                // 尝试再次添加相同成员应该抛出异常
                REQUIRE_THROWS_AS(dataService->addItemMember<TownData>(townWithMember, "小红"), DuplicateException);

                // 验证成员列表没有重复
                auto* townAfterFailedAdd = dataService->findTownAt(1550, 1550, 0);
                REQUIRE(townAfterFailedAdd != nullptr);
                REQUIRE(townAfterFailedAdd->getMemberXuids().size() == 1);
            }

            SECTION("Remove Non-existent Member") {
                // 尝试从不存在的成员列表中移除成员应该抛出异常
                REQUIRE_THROWS_AS(dataService->removeItemMember<TownData>(createdTown, "小红"), NotMemberException);

                // 验证成员列表仍然为空
                auto* townAfterFailedRemove = dataService->findTownAt(1550, 1550, 0);
                REQUIRE(townAfterFailedRemove != nullptr);
                REQUIRE(townAfterFailedRemove->getMemberXuids().empty());
            }
        }
    }

    SECTION("Town Special Operations") {
        // 创建一个城镇用于特殊操作测试
        TownData townForSpecialTest;
        townForSpecialTest.name        = "特殊操作城镇";
        townForSpecialTest.mayorXuid   = "200000001";
        townForSpecialTest.x           = 1700;
        townForSpecialTest.z           = 1700;
        townForSpecialTest.x_end       = 1800;
        townForSpecialTest.z_end       = 1800;
        townForSpecialTest.d           = 0;
        townForSpecialTest.perm        = 0;
        townForSpecialTest.description = "用于特殊操作测试的城镇";
        townForSpecialTest.id          = dataService->getMaxId<TownData>() + 1;

        PlayerInfo operatorInfo("100000001", "腐竹", true);
        dataService->createItem<TownData>(townForSpecialTest, operatorInfo);

        // 验证城镇创建成功
        auto* createdTown = dataService->findTownAt(1750, 1750, 0);
        REQUIRE(createdTown != nullptr);
        REQUIRE(createdTown->getTownName() == "特殊操作城镇");

        SECTION("Find Town by Name") {
            // 通过名称查找城镇
            auto* foundTown = dataService->findTownByName("特殊操作城镇");
            REQUIRE(foundTown != nullptr);
            REQUIRE(foundTown->getTownName() == "特殊操作城镇");
            REQUIRE(foundTown->getMayorXuid() == "200000001");

            // 查找不存在的城镇
            auto* notFoundTown = dataService->findTownByName("不存在的城镇");
            REQUIRE(notFoundTown == nullptr);
        }

        SECTION("Transfer Town Mayor") {
            // 验证当前镇长
            REQUIRE(createdTown->getMayorXuid() == "200000001");
            REQUIRE(createdTown->getOwnerName() == "小明");

            // 转让镇长
            dataService->transferTownMayor(createdTown, "小红");

            // 验证镇长已转让
            auto* updatedTown = dataService->findTownAt(1750, 1750, 0);
            REQUIRE(updatedTown != nullptr);
            REQUIRE(updatedTown->getMayorXuid() == "200000002");
            REQUIRE(updatedTown->getOwnerName() == "小红");

            // 验证JSON文件状态
            SECTION("JSON File Validation after Mayor Transfer") {
                // 更新预期数据
                townForSpecialTest.mayorXuid = "200000002";
                // 验证城镇数量和镇长转让
                REQUIRE(TownJsonTestUtils::verifyTownCountInJson(1));

                // 验证镇长是否正确转让
                auto towns             = TownJsonTestUtils::loadTownsFromJson();
                bool foundCorrectMayor = false;
                for (const auto& town : towns) {
                    if (town.name == "特殊操作城镇" && town.mayorXuid == "200000002") {
                        foundCorrectMayor = true;
                        break;
                    }
                }
                REQUIRE(foundCorrectMayor);
            }
        }
    }

    SECTION("Town Boundary Validation") {
        SECTION("Town Coordinates Range Validation") {
            // 测试超出范围的城镇坐标
            TownData outOfRangeTown;
            outOfRangeTown.name        = "越界城镇";
            outOfRangeTown.mayorXuid   = "200000001";
            outOfRangeTown.x           = LAND_RANGE + 1; // 超出范围
            outOfRangeTown.z           = 100;
            outOfRangeTown.x_end       = LAND_RANGE + 100;
            outOfRangeTown.z_end       = 200;
            outOfRangeTown.d           = 0;
            outOfRangeTown.perm        = 0;
            outOfRangeTown.description = "超出范围的城镇";
            outOfRangeTown.id          = dataService->getMaxId<TownData>() + 1;

            PlayerInfo operatorInfo("100000001", "腐竹", true);

            // 验证超出范围的城镇创建失败
            REQUIRE_THROWS_AS(
                dataService->createItem<TownData>(outOfRangeTown, operatorInfo),
                RealmOutOfRangeException
            );

            // 测试负数超出范围
            TownData negativeOutOfRangeTown;
            negativeOutOfRangeTown.name        = "负越界城镇";
            negativeOutOfRangeTown.mayorXuid   = "200000001";
            negativeOutOfRangeTown.x           = -LAND_RANGE - 1; // 超出负数范围
            negativeOutOfRangeTown.z           = 100;
            negativeOutOfRangeTown.x_end       = -LAND_RANGE + 100;
            negativeOutOfRangeTown.z_end       = 200;
            negativeOutOfRangeTown.d           = 0;
            negativeOutOfRangeTown.perm        = 0;
            negativeOutOfRangeTown.description = "负数超出范围的城镇";
            negativeOutOfRangeTown.id          = dataService->getMaxId<TownData>() + 1;

            // 验证负数超出范围的城镇创建失败
            REQUIRE_THROWS_AS(
                dataService->createItem<TownData>(negativeOutOfRangeTown, operatorInfo),
                RealmOutOfRangeException
            );
        }

        SECTION("Valid Boundary Town Creation") {
            // 测试边界范围内的城镇
            TownData boundaryTown;
            boundaryTown.name        = "边界城镇";
            boundaryTown.mayorXuid   = "200000001";
            boundaryTown.x           = LAND_RANGE - 100; // 边界范围内
            boundaryTown.z           = 100;
            boundaryTown.x_end       = LAND_RANGE - 50;
            boundaryTown.z_end       = 200;
            boundaryTown.d           = 0;
            boundaryTown.perm        = 0;
            boundaryTown.description = "边界范围内的城镇";
            boundaryTown.id          = dataService->getMaxId<TownData>() + 1;

            PlayerInfo operatorInfo("100000001", "腐竹", true);

            // 验证边界范围内的城镇可以创建
            REQUIRE_NOTHROW(dataService->createItem<TownData>(boundaryTown, operatorInfo));

            auto* createdBoundaryTown = dataService->findTownAt(LAND_RANGE - 75, 150, 0);
            REQUIRE(createdBoundaryTown != nullptr);
            REQUIRE(createdBoundaryTown->getTownName() == "边界城镇");
        }

        SECTION("Very Large Size Town") {
            // 测试超大尺寸城镇（超出原来的MAX_SIZE限制）
            TownData veryLargeTown;
            veryLargeTown.name        = "超大城镇";
            veryLargeTown.mayorXuid   = "200000001";
            veryLargeTown.x           = 10000;
            veryLargeTown.z           = 10000;
            veryLargeTown.x_end       = 20000; // 10000x10000城镇，远超原来的5000限制
            veryLargeTown.z_end       = 20000;
            veryLargeTown.d           = 0;
            veryLargeTown.perm        = 0;
            veryLargeTown.description = "超大尺寸测试城镇";
            veryLargeTown.id          = dataService->getMaxId<TownData>() + 1;

            PlayerInfo operatorInfo("100000001", "腐竹", true);

            // 验证超大尺寸城镇可以创建（不再受MAX_SIZE限制）
            REQUIRE_NOTHROW(dataService->createItem<TownData>(veryLargeTown, operatorInfo));

            auto* createdTown = dataService->findTownAt(15000, 15000, 0);
            REQUIRE(createdTown != nullptr);
            REQUIRE(createdTown->getTownName() == "超大城镇");
            REQUIRE(createdTown->getX() == 10000);
            REQUIRE(createdTown->getZ() == 10000);
            REQUIRE(createdTown->getXEnd() == 20000);
            REQUIRE(createdTown->getZEnd() == 20000);
        }
    }
}

} // namespace rlx_land::test