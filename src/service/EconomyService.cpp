#include "EconomyService.h"
#include "data/core/PlayerEconomyData.h"
#include "service/EconomyConfig.h"
#include <string>

namespace rlx_land {

bool EconomyService::hasSufficientFunds(const std::string& xuid, int amount) {
    return PlayerEconomyData::getPlayerMoney(xuid) >= amount;
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