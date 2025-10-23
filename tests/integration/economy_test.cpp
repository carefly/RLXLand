#include "common/PathConfig.h"
#include "data/core/PlayerEconomyData.h"
#include "data/economy/EconomyData.h"
#include "data/economy/EconomyDataManager.h"
#include "service/EconomyService.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>


namespace rlx_land::test {

// 经济系统测试辅助函数
namespace EconomyTestUtils {
// 获取经济数据文件路径
std::string getEconomyDataFilePath() { return PathConfig::ECONOMY_JSON_FILE; }

// 验证经济数据文件是否存在
bool verifyEconomyFileExists() {
    std::filesystem::path filePath = getEconomyDataFilePath();
    return std::filesystem::exists(filePath);
}

// 从JSON文件直接加载经济数据
std::vector<EconomyData> loadEconomyDataFromJson() {
    std::vector<EconomyData> economyData;
    std::filesystem::path    filePath = getEconomyDataFilePath();

    if (!std::filesystem::exists(filePath)) {
        return economyData; // 返回空列表
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        return economyData;
    }

    try {
        nlohmann::json json;
        file >> json;
        file.close();

        if (json.is_array()) {
            for (const auto& item : json) {
                EconomyData data = EconomyData::fromJson(item);
                economyData.push_back(data);
            }
        }
    } catch (const std::exception&) {
        // 忽略解析错误，返回已加载的数据
    }

    return economyData;
}

// 清理经济数据文件
void cleanupEconomyFile() {
    std::filesystem::path filePath = getEconomyDataFilePath();
    if (std::filesystem::exists(filePath)) {
        std::filesystem::remove(filePath);
    }
}
} // namespace EconomyTestUtils

// 经济系统测试用例
TEST_CASE("Economy System Tests", "[economy]") {

    // 清理之前的经济数据
    EconomyTestUtils::cleanupEconomyFile();

    SECTION("Player Economy Data Initialization") {
        // 测试初始化功能
        PlayerEconomyData::initialize();

        // 验证初始化后没有数据文件（因为没有初始数据）
        REQUIRE_FALSE(EconomyTestUtils::verifyEconomyFileExists());
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

    SECTION("Economy Data Persistence") {
        // 初始化经济数据
        PlayerEconomyData::initialize();

        const std::string testPlayerXuid1 = "test_player_3";
        const std::string testPlayerXuid2 = "test_player_4";
        const int64_t     amount1         = 1500;
        const int64_t     amount2         = 2500;

        // 设置多个玩家的金钱
        PlayerEconomyData::setPlayerMoney(testPlayerXuid1, amount1);
        PlayerEconomyData::setPlayerMoney(testPlayerXuid2, amount2);

        // 保存数据
        PlayerEconomyData::save();

        // 验证数据文件已创建
        REQUIRE(EconomyTestUtils::verifyEconomyFileExists());

        // 从文件加载数据验证
        std::vector<EconomyData> loadedData = EconomyTestUtils::loadEconomyDataFromJson();
        REQUIRE(loadedData.size() >= 2);

        // 查找特定玩家的数据
        bool foundPlayer1 = false;
        bool foundPlayer2 = false;

        for (const auto& data : loadedData) {
            if (data.xuid == testPlayerXuid1) {
                REQUIRE(data.money == amount1);
                foundPlayer1 = true;
            }
            if (data.xuid == testPlayerXuid2) {
                REQUIRE(data.money == amount2);
                foundPlayer2 = true;
            }
        }

        REQUIRE(foundPlayer1);
        REQUIRE(foundPlayer2);

        // 重新初始化并验证数据加载
        PlayerEconomyData::initialize();
        REQUIRE(PlayerEconomyData::getPlayerMoney(testPlayerXuid1) == amount1);
        REQUIRE(PlayerEconomyData::getPlayerMoney(testPlayerXuid2) == amount2);
    }

    SECTION("EconomyDataManager Direct File Operations") {
        // 测试经济数据管理器的直接文件操作

        // 清理之前的文件
        EconomyTestUtils::cleanupEconomyFile();

        const std::string testPlayerXuid = "test_player_5";
        const int64_t     testAmount     = 3000;

        // 创建测试数据
        std::vector<EconomyData> testData;
        testData.emplace_back(testPlayerXuid, testAmount);

        // 保存到文件
        EconomyDataManager::saveToFile(testData);

        // 验证文件存在
        REQUIRE(EconomyTestUtils::verifyEconomyFileExists());

        // 从文件加载
        std::vector<EconomyData> loadedData = EconomyDataManager::loadFromFile();
        REQUIRE(loadedData.size() == 1);
        REQUIRE(loadedData[0].xuid == testPlayerXuid);
        REQUIRE(loadedData[0].money == testAmount);
    }
}

} // namespace rlx_land::test