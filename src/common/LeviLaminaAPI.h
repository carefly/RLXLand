#pragma once

#include "mc/world/actor/player/Player.h"
#include <mc/server/ServerPlayer.h>
#include <string>

using namespace std;

class LeviLaminaAPI {

public:
    static Player* getPlayerByXuid(string xuid);
    static Player* getPlayerByName(string name);

    static string getPlayerNameByXuid(string xuid);
    static string getXuidByPlayerName(string name);
};