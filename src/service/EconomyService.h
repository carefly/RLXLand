#pragma once

#include <string>

namespace rlx_land {

class EconomyService {
public:
    // 检查玩家是否有足够的金钱
    static bool hasSufficientFunds(const std::string& xuid, int amount);

    // 扣除玩家金钱（用于购买领地）
    static bool deductLandPurchaseFee(const std::string& xuid, int amount);

    // 获取玩家当前余额
    static int getPlayerBalance(const std::string& xuid);

    // 设置玩家初始金钱（可用于测试或新玩家奖励）
    static void setInitialMoney(const std::string& xuid, int amount);

    // 增加玩家金钱（可用于出售领地或其他收入）
    static void addIncome(const std::string& xuid, int amount);

    // 获取领地购买费用
    static int getLandPurchaseCost(int area);
};

} // namespace rlx_land