#include "PlayerEconomyData.h"
#include "common/ModConfig.h"
#include "mod/data/RLXMoneyAPI.h"
#include "service/EconomyConfig.h"
#include <exception>
#include <format>
#include <stdexcept>
#include <unordered_map>

namespace rlx_land {

// 静态成员：缓存默认币种ID（避免重复调用）
std::string PlayerEconomyData::defaultCurrencyId;
// 保留 EconomyData 结构用于兼容性（但不再存储实际数据）
std::unordered_map<std::string, PlayerEconomyData::EconomyData> PlayerEconomyData::playerEconomyMap;
bool                                                            PlayerEconomyData::isDataModified      = false;
bool                                                            PlayerEconomyData::s_moneyDllAvailable = false;

void PlayerEconomyData::initialize() {
    // 检查 money DLL 是否可用
    s_moneyDllAvailable = ModConfig::checkMoneyDllExists();

    if (!s_moneyDllAvailable) {
        // DLL 不存在
        bool requirePlugin = ModConfig::requireMoneyPlugin();
        if (requirePlugin) {
            // 必须存在 DLL，抛出异常
            throw std::runtime_error("RLXMoney plugin is required but not found. Please install RLXMoney plugin.");
        } else {
            // 可选，使用本地模式
            defaultCurrencyId = "default";
            isDataModified    = false;
            return;
        }
    }

    // DLL 存在，尝试初始化
    try {
        // 获取默认币种ID
        defaultCurrencyId = rlx_money::RLXMoneyAPI::getDefaultCurrencyId();
        // 数据现在由 RLXMoney DLL 管理，不需要本地加载
        isDataModified = false;
    } catch (const std::exception& e) {
        // 如果获取默认币种失败，检查是否必须
        bool requirePlugin = ModConfig::requireMoneyPlugin();
        if (requirePlugin) {
            // 必须存在且可用，抛出异常
            throw std::runtime_error(std::format("RLXMoney plugin is found but failed to initialize: {}", e.what()));
        } else {
            // 可选，使用本地模式
            defaultCurrencyId   = "default";
            isDataModified      = false;
            s_moneyDllAvailable = false; // 标记为不可用
        }
    }
}

bool PlayerEconomyData::isMoneyDllAvailable() { return s_moneyDllAvailable; }

void PlayerEconomyData::save() {
    // 数据现在由 RLXMoney DLL 管理，不需要本地保存
    // 保留此方法以保持接口兼容性
    isDataModified = false;
}

PlayerEconomyData::EconomyData& PlayerEconomyData::getPlayerEconomy(const std::string& xuid) {
    // 如果 DLL 不可用，直接使用本地缓存
    if (!s_moneyDllAvailable) {
        auto it = playerEconomyMap.find(xuid);
        if (it == playerEconomyMap.end()) {
            // 创建新玩家数据
            int initialMoney             = EconomyConfig::PLAYER_INITIAL_MONEY;
            playerEconomyMap[xuid].money = initialMoney;
        }
        return playerEconomyMap[xuid];
    }

    // 确保默认币种ID已初始化
    if (defaultCurrencyId.empty()) {
        try {
            defaultCurrencyId = rlx_money::RLXMoneyAPI::getDefaultCurrencyId();
        } catch (const std::exception&) {
            // 如果获取失败，使用默认值
            defaultCurrencyId = "default";
        }
    }

    // 为了兼容性，返回一个缓存的 EconomyData 结构
    // 但实际数据从 RLXMoney DLL 获取
    auto it = playerEconomyMap.find(xuid);
    if (it == playerEconomyMap.end()) {
        try {
            // 从 RLXMoney DLL 获取余额
            auto balance = rlx_money::RLXMoneyAPI::getBalance(xuid, defaultCurrencyId);

            if (balance.has_value()) {
                playerEconomyMap[xuid].money = balance.value();
            } else {
                // 玩家不存在，设置初始余额
                int initialMoney = EconomyConfig::PLAYER_INITIAL_MONEY;
                try {
                    rlx_money::RLXMoneyAPI::setBalance(xuid, defaultCurrencyId, initialMoney, "RLXLand初始化");
                    playerEconomyMap[xuid].money = initialMoney;
                } catch (const std::exception&) {
                    // 如果设置失败，使用默认值
                    playerEconomyMap[xuid].money = initialMoney;
                }
            }
        } catch (const std::exception&) {
            // 如果获取余额失败（可能是玩家不存在），使用默认值
            int initialMoney             = EconomyConfig::PLAYER_INITIAL_MONEY;
            playerEconomyMap[xuid].money = initialMoney;
        }
        return playerEconomyMap[xuid];
    }

    // 更新缓存的数据（从 RLXMoney DLL 获取最新值）
    try {
        auto balance = rlx_money::RLXMoneyAPI::getBalance(xuid, defaultCurrencyId);
        if (balance.has_value()) {
            it->second.money = balance.value();
        }
    } catch (const std::exception&) {
        // 如果获取失败，保持缓存值不变
    }

    return it->second;
}

void PlayerEconomyData::setPlayerMoney(const std::string& xuid, int amount) {
    // 如果 DLL 不可用，只更新本地缓存
    if (!s_moneyDllAvailable) {
        playerEconomyMap[xuid].money = amount;
        isDataModified               = true;
        return;
    }

    // 确保默认币种ID已初始化
    if (defaultCurrencyId.empty()) {
        try {
            defaultCurrencyId = rlx_money::RLXMoneyAPI::getDefaultCurrencyId();
        } catch (const std::exception&) {
            defaultCurrencyId = "default";
        }
    }

    try {
        // 使用 RLXMoney DLL 设置余额
        bool success = rlx_money::RLXMoneyAPI::setBalance(xuid, defaultCurrencyId, amount, "RLXLand设置余额");

        if (success) {
            // 更新本地缓存
            playerEconomyMap[xuid].money = amount;
            isDataModified               = true;
        }
    } catch (const std::exception&) {
        // 如果设置失败（可能是玩家不存在），仍然更新本地缓存
        playerEconomyMap[xuid].money = amount;
        isDataModified               = true;
    }
}

void PlayerEconomyData::addPlayerMoney(const std::string& xuid, int amount) {
    // 如果 DLL 不可用，只更新本地缓存
    if (!s_moneyDllAvailable) {
        playerEconomyMap[xuid].money += amount;
        isDataModified                = true;
        return;
    }

    // 确保默认币种ID已初始化
    if (defaultCurrencyId.empty()) {
        try {
            defaultCurrencyId = rlx_money::RLXMoneyAPI::getDefaultCurrencyId();
        } catch (const std::exception&) {
            defaultCurrencyId = "default";
        }
    }

    try {
        // 使用 RLXMoney DLL 增加金钱
        bool success = rlx_money::RLXMoneyAPI::addMoney(xuid, defaultCurrencyId, amount, "RLXLand增加金钱");

        if (success) {
            // 更新本地缓存
            try {
                auto balance = rlx_money::RLXMoneyAPI::getBalance(xuid, defaultCurrencyId);
                if (balance.has_value()) {
                    playerEconomyMap[xuid].money = balance.value();
                } else {
                    // 如果获取失败，使用估算值
                    playerEconomyMap[xuid].money += amount;
                }
            } catch (const std::exception&) {
                // 如果获取失败，使用估算值
                playerEconomyMap[xuid].money += amount;
            }
            isDataModified = true;
        }
    } catch (const std::exception&) {
        // 如果增加失败，仍然更新本地缓存（使用估算值）
        playerEconomyMap[xuid].money += amount;
        isDataModified                = true;
    }
}

bool PlayerEconomyData::deductPlayerMoney(const std::string& xuid, int amount) {
    // 如果 DLL 不可用，只检查本地缓存
    if (!s_moneyDllAvailable) {
        auto& data = getPlayerEconomy(xuid);
        if (data.money >= amount) {
            data.money     -= amount;
            isDataModified  = true;
            return true;
        }
        return false;
    }

    // 确保默认币种ID已初始化
    if (defaultCurrencyId.empty()) {
        try {
            defaultCurrencyId = rlx_money::RLXMoneyAPI::getDefaultCurrencyId();
        } catch (const std::exception&) {
            defaultCurrencyId = "default";
        }
    }

    try {
        // 使用 RLXMoney DLL 检查余额并扣除
        // 注意：根据 DLL 实现，reduceMoney 在余额不足时会抛出异常
        bool success = rlx_money::RLXMoneyAPI::reduceMoney(xuid, defaultCurrencyId, amount, "RLXLand扣除金钱");

        if (success) {
            // 更新本地缓存
            try {
                auto balance = rlx_money::RLXMoneyAPI::getBalance(xuid, defaultCurrencyId);
                if (balance.has_value()) {
                    playerEconomyMap[xuid].money = balance.value();
                } else {
                    // 如果获取失败，使用估算值
                    playerEconomyMap[xuid].money -= amount;
                }
            } catch (const std::exception&) {
                // 如果获取失败，使用估算值
                playerEconomyMap[xuid].money -= amount;
            }
            isDataModified = true;
            return true;
        }
        return false;
    } catch (const std::exception&) {
        // 如果扣除失败（可能是余额不足或玩家不存在），返回 false
        // 注意：根据 DLL 实现，余额不足时会抛出 MoneyException
        return false;
    }
}

int PlayerEconomyData::getPlayerMoney(const std::string& xuid) {
    // 如果 DLL 不可用，从本地缓存获取
    if (!s_moneyDllAvailable) {
        return getPlayerEconomy(xuid).money;
    }

    // 确保默认币种ID已初始化
    if (defaultCurrencyId.empty()) {
        try {
            defaultCurrencyId = rlx_money::RLXMoneyAPI::getDefaultCurrencyId();
        } catch (const std::exception&) {
            defaultCurrencyId = "default";
        }
    }

    try {
        // 从 RLXMoney DLL 获取余额
        // 注意：根据 DLL 实现，getBalance 在玩家不存在或币种无效时会抛出异常
        auto balance = rlx_money::RLXMoneyAPI::getBalance(xuid, defaultCurrencyId);

        if (balance.has_value()) {
            // 更新本地缓存
            playerEconomyMap[xuid].money = balance.value();
            return balance.value();
        } else {
            // 玩家不存在，设置初始余额
            int initialMoney = EconomyConfig::PLAYER_INITIAL_MONEY;
            try {
                bool success =
                    rlx_money::RLXMoneyAPI::setBalance(xuid, defaultCurrencyId, initialMoney, "RLXLand初始化");
                if (success) {
                    playerEconomyMap[xuid].money = initialMoney;
                    return initialMoney;
                }
            } catch (const std::exception&) {
                // 如果设置失败，使用默认值
            }
            playerEconomyMap[xuid].money = initialMoney;
            return initialMoney;
        }
    } catch (const std::exception&) {
        // 如果获取失败（可能是玩家不存在），返回默认值
        int initialMoney             = EconomyConfig::PLAYER_INITIAL_MONEY;
        playerEconomyMap[xuid].money = initialMoney;
        return initialMoney;
    }
}

void PlayerEconomyData::resetAllData() {
    // 清空本地缓存的经济数据
    playerEconomyMap.clear();

    // 重置数据修改标志
    isDataModified = false;

    // 重置默认币种ID，强制下次使用时重新初始化
    defaultCurrencyId.clear();

    // 重置 DLL 可用性状态，强制下次重新检测
    s_moneyDllAvailable = false;

    // 如果 RLXMoney DLL 可用，可能还需要处理 DLL 端的数据
    // 但在测试环境中，通常只使用本地模式，所以这里主要清理本地缓存
    // 在实际的 DLL 环境中，可能需要额外的清理操作
}

} // namespace rlx_land