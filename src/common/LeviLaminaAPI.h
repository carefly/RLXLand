#pragma once

#include <string>
class Player;

namespace rlx_land {


class LeviLaminaAPI {

public:
    static Player* getPlayerByXuid(std::string xuid);
    static Player* getPlayerByName(std::string name);

    static std::string getPlayerNameByXuid(std::string xuid);
    static std::string getXuidByPlayerName(std::string name);
};

} // namespace rlx_land