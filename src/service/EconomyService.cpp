#include "EconomyService.h"

namespace rlx_land {

bool EconomyService::hasSufficientFunds(const std::string& xuid, int64_t amount) {
    return PlayerEconomyData::getPlayerMoney(xuid) >= amount;
}

bool EconomyService::deductLandPurchaseFee(const std::string& xuid, int64_t amount) {
    return PlayerEconomyData::deductPlayerMoney(xuid, amount);
}

int64_t EconomyService::getPlayerBalance(const std::string& xuid) {
    return PlayerEconomyData::getPlayerMoney(xuid);
}

void EconomyService::setInitialMoney(const std::string& xuid, int64_t amount) {
    PlayerEconomyData::setPlayerMoney(xuid, amount);
}

void EconomyService::addIncome(const std::string& xuid, int64_t amount) {
    PlayerEconomyData::addPlayerMoney(xuid, amount);
}

int64_t EconomyService::getLandPurchaseCost(int area) {
    return static_cast<int64_t>(area) * EconomyConfig::LAND_PRICE_PER_BLOCK;
}

} // namespace rlx_land