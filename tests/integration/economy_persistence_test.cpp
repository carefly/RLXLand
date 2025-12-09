#include "data/core/PlayerEconomyData.h"
#include "service/EconomyConfig.h"
#include <catch2/catch_test_macros.hpp>

namespace rlx_land::test {

// 经济数据功能测试（使用本地模式，因为测试环境中没有 RLXMoney DLL）
TEST_CASE("Economy Data Functionality Test", "[economy]") {
    // 初始化经济数据系统（在测试环境中，DLL 不可用，会使用本地模式）
    PlayerEconomyData::initialize();

    // 验证在本地模式下运行
    REQUIRE_FALSE(PlayerEconomyData::isMoneyDllAvailable());

    SECTION("New player default money") {
        const std::string newXuid       = "test_new_player_xuid";
        const int64_t     expectedMoney = EconomyConfig::PLAYER_INITIAL_MONEY;

        // 访问新玩家的金钱，应该获得默认金钱
        int64_t money = PlayerEconomyData::getPlayerMoney(newXuid);
        REQUIRE(money == expectedMoney);
    }

    SECTION("Modified player money") {
        const std::string testXuid      = "test_modify_player_xuid";
        const int64_t     initialMoney  = 50000;
        const int64_t     addedMoney    = 25000;
        const int64_t     expectedMoney = initialMoney + addedMoney;

        // 设置初始金钱
        PlayerEconomyData::setPlayerMoney(testXuid, initialMoney);
        REQUIRE(PlayerEconomyData::getPlayerMoney(testXuid) == initialMoney);

        // 增加金钱
        PlayerEconomyData::addPlayerMoney(testXuid, addedMoney);
        REQUIRE(PlayerEconomyData::getPlayerMoney(testXuid) == expectedMoney);
    }

    SECTION("Deducted player money") {
        const std::string testXuid      = "test_deduct_player_xuid";
        const int64_t     initialMoney  = 100000;
        const int64_t     deductedMoney = 30000;
        const int64_t     expectedMoney = initialMoney - deductedMoney;

        // 设置初始金钱
        PlayerEconomyData::setPlayerMoney(testXuid, initialMoney);

        // 扣除金钱
        bool result = PlayerEconomyData::deductPlayerMoney(testXuid, deductedMoney);
        REQUIRE(result);
        REQUIRE(PlayerEconomyData::getPlayerMoney(testXuid) == expectedMoney);
    }

    SECTION("Insufficient balance deduction") {
        const std::string testXuid      = "test_insufficient_xuid";
        const int64_t     initialMoney  = 10000;
        const int64_t     deductedMoney = 20000;

        // 设置初始金钱
        PlayerEconomyData::setPlayerMoney(testXuid, initialMoney);

        // 尝试扣除超过余额的金额，应该失败
        bool result = PlayerEconomyData::deductPlayerMoney(testXuid, deductedMoney);
        REQUIRE_FALSE(result);
        // 余额应该保持不变
        REQUIRE(PlayerEconomyData::getPlayerMoney(testXuid) == initialMoney);
    }
}

} // namespace rlx_land::test