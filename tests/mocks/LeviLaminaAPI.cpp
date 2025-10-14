#include "../overrides/common/LeviLaminaAPI.h"

namespace rlx_land {

// 静态成员定义
std::map<std::string, Player*>     LeviLaminaAPI::mockPlayers;
std::map<std::string, std::string> LeviLaminaAPI::xuidToName;
std::map<std::string, std::string> LeviLaminaAPI::nameToXuid;

Player* LeviLaminaAPI::getPlayerByXuid(const std::string& xuid) {
    auto it = mockPlayers.find(xuid);
    if (it != mockPlayers.end()) {
        return it->second;
    }

    // 如果找不到，返回nullptr
    return nullptr;
}

Player* LeviLaminaAPI::getPlayerByName(const std::string& name) {
    auto it = nameToXuid.find(name);
    if (it != nameToXuid.end()) {
        return getPlayerByXuid(it->second);
    }

    // 如果找不到，返回nullptr
    return nullptr;
}

std::string LeviLaminaAPI::getPlayerNameByXuid(const std::string& xuid) {
    auto it = xuidToName.find(xuid);
    if (it != xuidToName.end()) {
        return it->second;
    }

    // 如果找不到，返回空字符串
    return {};
}

std::string LeviLaminaAPI::getXuidByPlayerName(const std::string& name) {
    auto it = nameToXuid.find(name);
    if (it != nameToXuid.end()) {
        return it->second;
    }

    // 如果找不到，返回空字符串
    return {};
}

void LeviLaminaAPI::addMockPlayer(const std::string& xuid, const std::string& name, bool isOp) {
    // 清理旧的Player对象（如果存在）
    auto it = mockPlayers.find(xuid);
    if (it != mockPlayers.end()) {
        delete it->second;
    }

    // 创建新的Player对象
    mockPlayers[xuid] = new Player(xuid, name, isOp);
    xuidToName[xuid]  = name;
    nameToXuid[name]  = xuid;
}

void LeviLaminaAPI::removeMockPlayer(const std::string& xuid) {
    auto it = mockPlayers.find(xuid);
    if (it != mockPlayers.end()) {
        std::string name = it->second->getName();
        delete it->second;
        mockPlayers.erase(it);

        xuidToName.erase(xuid);
        nameToXuid.erase(name);
    }
}

void LeviLaminaAPI::clearMockPlayers() {
    // 清理所有Player对象
    for (auto& [xuid, player] : mockPlayers) {
        delete player;
    }

    mockPlayers.clear();
    xuidToName.clear();
    nameToXuid.clear();
}

} // namespace rlx_land