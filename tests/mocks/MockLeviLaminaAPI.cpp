#include "MockLeviLaminaAPI.h"
#include <map>
#include <string>

namespace rlx_land::test::mock {

static std::map<std::string, std::string> mockPlayers; // xuid -> name
static std::map<std::string, std::string> mockNames;   // name -> xuid

std::string MockLeviLaminaAPI::getPlayerNameByXuid(const std::string& xuid) {
    auto it = mockPlayers.find(xuid);
    if (it != mockPlayers.end()) {
        return it->second;
    }
    // 返回默认名称用于测试
    return "MockPlayer_" + xuid;
}

std::string MockLeviLaminaAPI::getXuidByPlayerName(const std::string& name) {
    auto it = mockNames.find(name);
    if (it != mockNames.end()) {
        return it->second;
    }
    // 返回默认XUID用于测试
    return "mock_xuid_" + name;
}

void MockLeviLaminaAPI::addMockPlayer(const std::string& xuid, const std::string& name) {
    mockPlayers[xuid] = name;
    mockNames[name]   = xuid;
}

void MockLeviLaminaAPI::clearMockPlayers() {
    mockPlayers.clear();
    mockNames.clear();
}

} // namespace rlx_land::test::mock