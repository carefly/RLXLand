#pragma once
#include <string>
#include <unordered_map>


namespace rlx_land {

class PlayerEconomyData {
public:
    // 玩家经济数据结构
    struct EconomyData {
        int money; // 玩家金钱
        // 可以添加其他经济相关属性，如积分、声望等

        EconomyData() : money(0) {}
        explicit EconomyData(int m) : money(m) {}
    };

    // 初始化数据管理器
    static void initialize();

    // 保存数据
    static void save();

    // 获取玩家经济数据
    static EconomyData& getPlayerEconomy(const std::string& xuid);

    // 设置玩家金钱
    static void setPlayerMoney(const std::string& xuid, int amount);

    // 增加玩家金钱
    static void addPlayerMoney(const std::string& xuid, int amount);

    // 扣除玩家金钱
    static bool deductPlayerMoney(const std::string& xuid, int amount);

    // 获取玩家金钱
    static int getPlayerMoney(const std::string& xuid);

    // 检查 money DLL 是否可用
    static bool isMoneyDllAvailable();

    // 重置所有经济数据（主要用于测试）
    static void resetAllData();

private:
    // 默认币种ID（从 RLXMoney DLL 获取）
    static std::string defaultCurrencyId;

    // 玩家经济数据存储（用于兼容性，实际数据由 RLXMoney DLL 管理）
    static std::unordered_map<std::string, EconomyData> playerEconomyMap;

    // 数据是否已修改（用于兼容性）
    static bool isDataModified;

    // money DLL 是否可用
    static bool s_moneyDllAvailable;
};

} // namespace rlx_land