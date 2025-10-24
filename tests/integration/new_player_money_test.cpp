#include "data/core/PlayerEconomyData.h"
#include "service/EconomyConfig.h"
#include <catch2/catch_test_macros.hpp>
#include <string>

namespace rlx_land::test {

TEST_CASE("New Player Default Money Test", "[economy]") {
    // 初始化经济数据
    PlayerEconomyData::initialize();
    
    SECTION("New player should get default money") {
        const std::string newXuid = "new_player_xuid";
        
        // 检查新玩家是否获得默认金钱
        int64_t money = PlayerEconomyData::getPlayerMoney(newXuid);
        REQUIRE(money == EconomyConfig::PLAYER_INITIAL_MONEY);
        REQUIRE(money == 100000); // 确保是100000而不是其他值
    }
    
    SECTION("Existing player should keep their money") {
        const std::string existingXuid = "existing_player_xuid";
        const int64_t existingAmount = 50000;
        
        // 设置现有玩家的金钱
        PlayerEconomyData::setPlayerMoney(existingXuid, existingAmount);
        
        // 检查现有玩家的金钱是否保持不变
        int64_t money = PlayerEconomyData::getPlayerMoney(existingXuid);
        REQUIRE(money == existingAmount);
        REQUIRE(money != EconomyConfig::PLAYER_INITIAL_MONEY);
    }
    
    SECTION("New player through getPlayerEconomy should get default money") {
        const std::string newXuid = "new_player_xuid_2";
        
        // 通过getPlayerEconomy检查新玩家是否获得默认金钱
        auto& economyData = PlayerEconomyData::getPlayerEconomy(newXuid);
        REQUIRE(economyData.money == EconomyConfig::PLAYER_INITIAL_MONEY);
        REQUIRE(economyData.money == 100000);
    }
}

} // namespace rlx_land::test