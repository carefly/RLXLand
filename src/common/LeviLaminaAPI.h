#pragma once

#include <string>
class Player;

// 测试环境宏定义
#ifdef TESTING
#include "../tests/overrides/common/LeviLaminaAPI.h"
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