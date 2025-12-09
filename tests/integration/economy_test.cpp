#include "data/core/PlayerEconomyData.h"
#include "service/EconomyConfig.h"
#include "service/EconomyService.h"
#include <catch2/catch_test_macros.hpp>
#include <string>


namespace rlx_land::test {

// 经济系统测试用例（使用本地模式，因为测试环境中没有 RLXMoney DLL）
TEST_CASE("Economy System Tests", "[economy]") {
    SECTION("Player Economy Data Initialization") {
        // 测试初始化功能
        PlayerEconomyData::initialize();

        // 验证在本地模式下运行（DLL 不可用）
        REQUIRE_FALSE(PlayerEconomyData::isMoneyDllAvailable());
    }

    SECTION("Player Money Management") {
        // 初始化经济数据
        PlayerEconomyData::initialize();

        const std::string testPlayerXuid = "test_player_1";
        const int64_t     initialAmount  = 1000;
        const int64_t     addAmount      = 500;
        const int64_t     deductAmount   = 300;

        // 测试设置玩家初始金钱
        PlayerEconomyData::setPlayerMoney(testPlayerXuid, initialAmount);
        REQUIRE(PlayerEconomyData::getPlayerMoney(testPlayerXuid) == initialAmount);

        // 测试增加玩家金钱
        PlayerEconomyData::addPlayerMoney(testPlayerXuid, addAmount);
        REQUIRE(PlayerEconomyData::getPlayerMoney(testPlayerXuid) == initialAmount + addAmount);

        // 测试扣除玩家金钱（余额足够）
        bool deductResult = PlayerEconomyData::deductPlayerMoney(testPlayerXuid, deductAmount);
        REQUIRE(deductResult);
        REQUIRE(PlayerEconomyData::getPlayerMoney(testPlayerXuid) == initialAmount + addAmount - deductAmount);

        // 测试扣除玩家金钱（余额不足）
        bool deductResult2 = PlayerEconomyData::deductPlayerMoney(testPlayerXuid, initialAmount * 2);
        REQUIRE_FALSE(deductResult2);
        // 余额应该保持不变
        REQUIRE(PlayerEconomyData::getPlayerMoney(testPlayerXuid) == initialAmount + addAmount - deductAmount);
    }

    SECTION("Economy Service Functions") {
        // 初始化经济数据
        PlayerEconomyData::initialize();

        const std::string testPlayerXuid = "test_player_2";
        const int64_t     initialAmount  = 2000;
        const int64_t     purchaseAmount = 500;

        // 设置玩家初始金钱
        EconomyService::setInitialMoney(testPlayerXuid, initialAmount);
        REQUIRE(EconomyService::getPlayerBalance(testPlayerXuid) == initialAmount);

        // 测试余额检查
        REQUIRE(EconomyService::hasSufficientFunds(testPlayerXuid, purchaseAmount));
        REQUIRE_FALSE(EconomyService::hasSufficientFunds(testPlayerXuid, initialAmount * 2));

        // 测试扣除购买费用
        bool deductResult = EconomyService::deductLandPurchaseFee(testPlayerXuid, purchaseAmount);
        REQUIRE(deductResult);
        REQUIRE(EconomyService::getPlayerBalance(testPlayerXuid) == initialAmount - purchaseAmount);

        // 测试收入增加
        const int64_t incomeAmount = 300;
        EconomyService::addIncome(testPlayerXuid, incomeAmount);
        REQUIRE(EconomyService::getPlayerBalance(testPlayerXuid) == initialAmount - purchaseAmount + incomeAmount);
    }

    SECTION("Land Purchase Cost Calculation") {
        // 测试领地购买费用计算
        const int area1 = 100;
        const int area2 = 500;
        const int area3 = 1000;

        int64_t cost1 = EconomyService::getLandPurchaseCost(area1);
        int64_t cost2 = EconomyService::getLandPurchaseCost(area2);
        int64_t cost3 = EconomyService::getLandPurchaseCost(area3);

        // 验证费用计算正确性（假设每方块1金币）
        REQUIRE(cost1 == area1 * EconomyConfig::LAND_PRICE_PER_BLOCK);
        REQUIRE(cost2 == area2 * EconomyConfig::LAND_PRICE_PER_BLOCK);
        REQUIRE(cost3 == area3 * EconomyConfig::LAND_PRICE_PER_BLOCK);
    }

    SECTION("Multiple Players Money Management") {
        // 初始化经济数据
        PlayerEconomyData::initialize();

        const std::string testPlayerXuid1 = "test_player_3";
        const std::string testPlayerXuid2 = "test_player_4";
        const int64_t     amount1         = 1500;
        const int64_t     amount2         = 2500;

        // 设置多个玩家的金钱
        PlayerEconomyData::setPlayerMoney(testPlayerXuid1, amount1);
        PlayerEconomyData::setPlayerMoney(testPlayerXuid2, amount2);

        // 验证每个玩家的金钱
        REQUIRE(PlayerEconomyData::getPlayerMoney(testPlayerXuid1) == amount1);
        REQUIRE(PlayerEconomyData::getPlayerMoney(testPlayerXuid2) == amount2);
    }
}

} // namespace rlx_land::test