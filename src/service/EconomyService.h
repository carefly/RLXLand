#pragma once
#include "data/core/PlayerEconomyData.h"
#include "service/EconomyConfig.h"
#include <string>

namespace rlx_land {

class EconomyService {
public:
    // 检查玩家是否有足够的金钱
    static bool hasSufficientFunds(const std::string& xuid, int64_t amount);
    
    // 扣除玩家金钱（用于购买领地）
    static bool deductLandPurchaseFee(const std::string& xuid, int64_t amount);
    
    // 获取玩家当前余额
    static int64_t getPlayerBalance(const std::string& xuid);
    
    // 设置玩家初始金钱（可用于测试或新玩家奖励）
    static void setInitialMoney(const std::string& xuid, int64_t amount);
    
    // 增加玩家金钱（可用于出售领地或其他收入）
    static void addIncome(const std::string& xuid, int64_t amount);
    
    // 获取领地购买费用
    static int64_t getLandPurchaseCost(int area);
};

} // namespace rlx_land