#include "common/PathConfig.h"
#include "data/core/PlayerEconomyData.h"
#include "data/economy/EconomyDataManager.h"
#include "service/EconomyConfig.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace rlx_land::test {

// 经济数据持久化测试
TEST_CASE("Economy Data Persistence Test", "[economy][persistence]") {
    // 确保数据目录存在
    PathConfig::ensureDataDirectoriesExist();

    // 获取经济数据文件路径
    std::string economyDataFile = PathConfig::ECONOMY_JSON_FILE;

    // 删除已有的经济数据文件（如果存在）
    if (std::filesystem::exists(economyDataFile)) {
        std::filesystem::remove(economyDataFile);
    }

    // 初始化经济数据系统
    PlayerEconomyData::initialize();

    SECTION("New player default money should be saved to file") {
        const std::string newXuid       = "test_new_player_xuid";
        const int64_t     expectedMoney = EconomyConfig::PLAYER_INITIAL_MONEY;

        // 访问新玩家的金钱，触发默认金钱设置和保存
        int64_t money = PlayerEconomyData::getPlayerMoney(newXuid);
        REQUIRE(money == expectedMoney);

        // 检查文件是否存在
        REQUIRE(std::filesystem::exists(economyDataFile));

        // 从文件加载数据并验证
        std::vector<EconomyData> loadedData  = EconomyDataManager::loadFromFile();
        bool                     foundPlayer = false;
        for (const auto& data : loadedData) {
            if (data.xuid == newXuid) {
                REQUIRE(data.money == expectedMoney);
                foundPlayer = true;
                break;
            }
        }
        REQUIRE(foundPlayer);
    }

    SECTION("Modified player money should be saved to file") {
        const std::string testXuid      = "test_modify_player_xuid";
        const int64_t     initialMoney  = 50000;
        const int64_t     addedMoney    = 25000;
        const int64_t     expectedMoney = initialMoney + addedMoney;

        // 设置初始金钱
        PlayerEconomyData::setPlayerMoney(testXuid, initialMoney);

        // 增加金钱
        PlayerEconomyData::addPlayerMoney(testXuid, addedMoney);

        // 检查文件是否存在
        REQUIRE(std::filesystem::exists(economyDataFile));

        // 从文件加载数据并验证
        std::vector<EconomyData> loadedData  = EconomyDataManager::loadFromFile();
        bool                     foundPlayer = false;
        for (const auto& data : loadedData) {
            if (data.xuid == testXuid) {
                REQUIRE(data.money == expectedMoney);
                foundPlayer = true;
                break;
            }
        }
        REQUIRE(foundPlayer);
    }

    SECTION("Deducted player money should be saved to file") {
        const std::string testXuid      = "test_deduct_player_xuid";
        const int64_t     initialMoney  = 100000;
        const int64_t     deductedMoney = 30000;
        const int64_t     expectedMoney = initialMoney - deductedMoney;

        // 设置初始金钱
        PlayerEconomyData::setPlayerMoney(testXuid, initialMoney);

        // 扣除金钱
        bool result = PlayerEconomyData::deductPlayerMoney(testXuid, deductedMoney);
        REQUIRE(result);

        // 检查文件是否存在
        REQUIRE(std::filesystem::exists(economyDataFile));

        // 从文件加载数据并验证
        std::vector<EconomyData> loadedData  = EconomyDataManager::loadFromFile();
        bool                     foundPlayer = false;
        for (const auto& data : loadedData) {
            if (data.xuid == testXuid) {
                REQUIRE(data.money == expectedMoney);
                foundPlayer = true;
                break;
            }
        }
        REQUIRE(foundPlayer);
    }

    // 清理测试数据
    if (std::filesystem::exists(economyDataFile)) {
        std::filesystem::remove(economyDataFile);
    }
}

} // namespace rlx_land::test