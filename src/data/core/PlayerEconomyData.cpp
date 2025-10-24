#include "PlayerEconomyData.h"
#include "data/economy/EconomyDataManager.h"
#include "service/EconomyConfig.h"
#include <vector>

namespace rlx_land {

// 初始化静态成员
std::unordered_map<std::string, PlayerEconomyData::EconomyData> PlayerEconomyData::playerEconomyMap;
bool                                                            PlayerEconomyData::isDataModified = false;

void PlayerEconomyData::initialize() {
    // 从文件加载数据
    std::vector<rlx_land::EconomyData> loadedData = EconomyDataManager::loadFromFile();

    // 将数据加载到内存中
    for (const auto& data : loadedData) {
        playerEconomyMap[data.xuid].money = data.money;
    }

    // 重置修改标志
    isDataModified = false;
}

void PlayerEconomyData::save() {
    // 只有在数据被修改时才保存
    if (!isDataModified) {
        return;
    }
    
    // 将内存中的数据转换为保存格式
    std::vector<rlx_land::EconomyData> saveData;
    // 预先分配容器容量以提高性能
    saveData.reserve(playerEconomyMap.size());
    
    for (const auto& [xuid, economyData] : playerEconomyMap) {
        saveData.emplace_back(xuid, economyData.money);
    }
    
    // 保存到文件
    EconomyDataManager::saveToFile(saveData);
    
    // 重置修改标志
    isDataModified = false;
}

PlayerEconomyData::EconomyData& PlayerEconomyData::getPlayerEconomy(const std::string& xuid) {
    // 如果玩家不存在，会自动创建一个默认的EconomyData，并给予初始金钱
    auto it = playerEconomyMap.find(xuid);
    if (it == playerEconomyMap.end()) {
        // 新玩家，给予默认金钱
        playerEconomyMap[xuid].money = EconomyConfig::PLAYER_INITIAL_MONEY;
        isDataModified = true;
        save();
        return playerEconomyMap[xuid];
    }
    return it->second;
}

void PlayerEconomyData::setPlayerMoney(const std::string& xuid, int64_t amount) {
    playerEconomyMap[xuid].money = amount;
    isDataModified               = true;
    save();
}

void PlayerEconomyData::addPlayerMoney(const std::string& xuid, int64_t amount) {
    playerEconomyMap[xuid].money += amount;
    isDataModified                = true;
    save();
}

bool PlayerEconomyData::deductPlayerMoney(const std::string& xuid, int64_t amount) {
    // 检查余额是否足够
    if (playerEconomyMap[xuid].money >= amount) {
        playerEconomyMap[xuid].money -= amount;
        isDataModified                = true;
        save();
        return true;
    }
    return false; // 余额不足
}

int64_t PlayerEconomyData::getPlayerMoney(const std::string& xuid) {
    auto it = playerEconomyMap.find(xuid);
    if (it == playerEconomyMap.end()) {
        // 新玩家，给予默认金钱
        playerEconomyMap[xuid].money = EconomyConfig::PLAYER_INITIAL_MONEY;
        isDataModified = true;
        save();
        return playerEconomyMap[xuid].money;
    }
    return it->second.money;
}

} // namespace rlx_land