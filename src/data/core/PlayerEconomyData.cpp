#include "PlayerEconomyData.h"
#include "common/ConfigManager.hpp"
#include "common/LandConfig.hpp"
#include <RLXMoney/api/RLXMoneyAPI.h>
#include <format>
#include <stdexcept>
#include <unordered_map>


namespace rlx_land {

// 静态成员定义
std::string                                                     PlayerEconomyData::defaultCurrencyId;
std::unordered_map<std::string, PlayerEconomyData::EconomyData> PlayerEconomyData::playerEconomyMap;
bool                                                            PlayerEconomyData::isDataModified      = false;
bool                                                            PlayerEconomyData::s_moneyDllAvailable = false;

// ============================================================================
// 私有辅助方法
// ============================================================================

bool PlayerEconomyData::ensureCurrencyId() {
    if (!defaultCurrencyId.empty()) {
        return true;
    }
    try {
        defaultCurrencyId = rlx_money::RLXMoneyAPI::getDefaultCurrencyId();
        return true;
    } catch (const std::exception&) {
        defaultCurrencyId = "default";
        return false;
    }
}

bool PlayerEconomyData::updateLocalCache(const std::string& xuid) {
    if (!s_moneyDllAvailable) {
        return false;
    }
    if (!ensureCurrencyId()) {
        return false;
    }

    try {
        auto balance = rlx_money::RLXMoneyAPI::getBalance(xuid, defaultCurrencyId);
        if (balance.has_value()) {
            playerEconomyMap[xuid].money = balance.value();
            return true;
        }
    } catch (const std::exception&) {
        // 获取失败，忽略
    }
    return false;
}

// ============================================================================
// 公共方法
// ============================================================================

void PlayerEconomyData::initialize() {
    // 如果经济系统未启用，不初始化 RLXMoney 插件
    if (!rlx_land::getLandConfig().enableEconomy) {
        s_moneyDllAvailable = false;
        defaultCurrencyId = "default";
        return;
    }

    s_moneyDllAvailable =
        rlx::common::checkDllExists("RLXMoney.dll", {"plugins/RLXMoney", "../plugins/RLXMoney"});

    if (!s_moneyDllAvailable) {
        defaultCurrencyId = "default";
        return;
    }

    try {
        defaultCurrencyId = rlx_money::RLXMoneyAPI::getDefaultCurrencyId();
    } catch (const std::exception& e) {
        defaultCurrencyId   = "default";
        s_moneyDllAvailable = false;
    }
}

bool PlayerEconomyData::isMoneyDllAvailable() {
    return s_moneyDllAvailable;
}

bool PlayerEconomyData::isEconomyEnabled() {
    return rlx_land::getLandConfig().enableEconomy && s_moneyDllAvailable;
}

void PlayerEconomyData::save() {
    isDataModified = false;
}

PlayerEconomyData::EconomyData& PlayerEconomyData::getPlayerEconomy(const std::string& xuid) {
    auto it = playerEconomyMap.find(xuid);
    if (it != playerEconomyMap.end()) {
        // DLL 模式下同步最新余额
        if (s_moneyDllAvailable) {
            updateLocalCache(xuid);
        }
        return it->second;
    }

    // 新玩家：初始化数据
    if (!s_moneyDllAvailable) {
        // 经济系统未启用或插件不可用，使用 0 金币
        playerEconomyMap[xuid].money = 0;
        return playerEconomyMap[xuid];
    }

    // DLL 模式：从 RLXMoney 获取余额（不给初始金币，由 RLXMoney 管理）
    if (ensureCurrencyId()) {
        try {
            auto balance = rlx_money::RLXMoneyAPI::getBalance(xuid, defaultCurrencyId);
            if (balance.has_value()) {
                playerEconomyMap[xuid].money = balance.value();
            } else {
                // RLXMoney 会自动给初始余额，这里直接返回 0
                playerEconomyMap[xuid].money = 0;
            }
            return playerEconomyMap[xuid];
        } catch (const std::exception&) {
            // 获取失败，使用默认值
        }
    }

    playerEconomyMap[xuid].money = 0;
    return playerEconomyMap[xuid];
}

// ============================================================================
// 金钱操作方法
// ============================================================================

std::optional<int> PlayerEconomyData::tryGetPlayerMoney(const std::string& xuid) {
    if (!s_moneyDllAvailable) {
        return getPlayerEconomy(xuid).money;
    }

    if (!ensureCurrencyId()) {
        return std::nullopt;
    }

    try {
        auto balance = rlx_money::RLXMoneyAPI::getBalance(xuid, defaultCurrencyId);
        if (balance.has_value()) {
            playerEconomyMap[xuid].money = balance.value();
            return balance.value();
        }
        return std::nullopt;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

int PlayerEconomyData::getPlayerMoney(const std::string& xuid) {
    // 尝试获取余额，失败时返回默认值
    auto balance = tryGetPlayerMoney(xuid);
    if (balance.has_value()) {
        return balance.value();
    }
    // 获取失败时返回 0，不再给初始金币
    playerEconomyMap[xuid].money = 0;
    return 0;
}

void PlayerEconomyData::setPlayerMoney(const std::string& xuid, int amount) {
    if (!s_moneyDllAvailable) {
        playerEconomyMap[xuid].money = amount;
        isDataModified               = true;
        return;
    }

    if (!ensureCurrencyId()) {
        playerEconomyMap[xuid].money = amount;
        isDataModified               = true;
        return;
    }

    try {
        rlx_money::RLXMoneyAPI::setBalance(xuid, defaultCurrencyId, amount, "RLXLand设置余额");
    } catch (const std::exception&) {
        // DLL 调用失败，仍更新本地缓存
    }
    playerEconomyMap[xuid].money = amount;
    isDataModified               = true;
}

void PlayerEconomyData::addPlayerMoney(const std::string& xuid, int amount) {
    if (!s_moneyDllAvailable) {
        playerEconomyMap[xuid].money += amount;
        isDataModified                = true;
        return;
    }

    if (!ensureCurrencyId()) {
        playerEconomyMap[xuid].money += amount;
        isDataModified                = true;
        return;
    }

    try {
        rlx_money::RLXMoneyAPI::addMoney(xuid, defaultCurrencyId, amount, "RLXLand增加金钱");
        updateLocalCache(xuid);
    } catch (const std::exception&) {
        // DLL 调用失败，使用估算值
        playerEconomyMap[xuid].money += amount;
    }
    isDataModified = true;
}

bool PlayerEconomyData::deductPlayerMoney(const std::string& xuid, int amount) {
    if (!s_moneyDllAvailable) {
        auto& data = getPlayerEconomy(xuid);
        if (data.money >= amount) {
            data.money -= amount;
            isDataModified = true;
            return true;
        }
        return false;
    }

    if (!ensureCurrencyId()) {
        return false;
    }

    try {
        bool success = rlx_money::RLXMoneyAPI::reduceMoney(xuid, defaultCurrencyId, amount, "RLXLand扣除金钱");
        if (success) {
            updateLocalCache(xuid);
            isDataModified = true;
            return true;
        }
        return false;
    } catch (const std::exception&) {
        // 余额不足或玩家不存在
        return false;
    }
}

void PlayerEconomyData::resetAllData() {
    playerEconomyMap.clear();
    isDataModified      = false;
    defaultCurrencyId.clear();
    s_moneyDllAvailable = false;
}

} // namespace rlx_land
