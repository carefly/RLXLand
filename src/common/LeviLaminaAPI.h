#pragma once

#include <string>
class Player;

// 测试环境宏定义
#ifdef TESTING
#include "../tests/mocks/MockLeviLaminaAPI.h"
#define LeviLaminaAPI rlx_land::test::mock::MockLeviLaminaAPI
#endif

namespace rlx_land {

#ifndef TESTING
class LeviLaminaAPI {

public:
    static Player* getPlayerByXuid(std::string xuid);
    static Player* getPlayerByName(std::string name);

    static std::string getPlayerNameByXuid(std::string xuid);
    static std::string getXuidByPlayerName(std::string name);
};
#endif

} // namespace rlx_land