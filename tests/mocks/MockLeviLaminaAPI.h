#pragma once

#include <string>

namespace rlx_land::test::mock {

class MockLeviLaminaAPI {
public:
    static std::string getPlayerNameByXuid(const std::string& xuid);
    static std::string getXuidByPlayerName(const std::string& name);

    // 测试辅助方法
    static void addMockPlayer(const std::string& xuid, const std::string& name);
    static void clearMockPlayers();
};

} // namespace rlx_land::test::mock