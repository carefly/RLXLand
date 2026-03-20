#include "EconomyService.h"
#include "data/core/PlayerEconomyData.h"
#include "service/EconomyConfig.h"
#include <optional>
#include <string>

namespace rlx_land {

bool EconomyService::hasSufficientFunds(const std::string& xuid, int amount) {
    // 如果金钱系统可用但获取不到余额，直接返回 false
    auto balance = PlayerEconomyData::tryGetPlayerMoney(xuid);
    if (!balance.has_value()) {
        return false;
    }
    return balance.value() >= amount;
}

bool EconomyService::deductLandPurchaseFee(const std::string& xuid, int amount) {
    return PlayerEconomyData::deductPlayerMoney(xuid, amount);
}

int EconomyService::getPlayerBalance(const std::string& xuid) { return PlayerEconomyData::getPlayerMoney(xuid); }

void EconomyService::setInitialMoney(const std::string& xuid, int amount) {
    PlayerEconomyData::setPlayerMoney(xuid, amount);
}

void EconomyService::addIncome(const std::string& xuid, int amount) { PlayerEconomyData::addPlayerMoney(xuid, amount); }

int EconomyService::getLandPurchaseCost(int area) { return (area * EconomyConfig::LAND_PRICE_PER_BLOCK); }

} // namespace rlx_land