#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>

namespace rlx_land {

class PlayerEconomyData {
public:
    // 玩家经济数据结构
    struct EconomyData {
        int64_t money;  // 玩家金钱
        // 可以添加其他经济相关属性，如积分、声望等
        
        EconomyData() : money(0) {}
        explicit EconomyData(int64_t m) : money(m) {}
    };
    
    // 初始化数据管理器
    static void initialize();
    
    // 保存数据
    static void save();
    
    // 获取玩家经济数据
    static EconomyData& getPlayerEconomy(const std::string& xuid);
    
    // 设置玩家金钱
    static void setPlayerMoney(const std::string& xuid, int64_t amount);
    
    // 增加玩家金钱
    static void addPlayerMoney(const std::string& xuid, int64_t amount);
    
    // 扣除玩家金钱
    static bool deductPlayerMoney(const std::string& xuid, int64_t amount);
    
    // 获取玩家金钱
    static int64_t getPlayerMoney(const std::string& xuid);
    
private:
    // 玩家经济数据存储
    static std::unordered_map<std::string, EconomyData> playerEconomyMap;
    
    // 数据是否已修改
    static bool isDataModified;
};

} // namespace rlx_land