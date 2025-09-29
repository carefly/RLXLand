
#include <ll/api/service/Bedrock.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/Level.h>

#include "LeviLaminaAPI.h"


namespace rlx_land {
Player* LeviLaminaAPI::getPlayerByXuid(std::string xuid) {
    Player* foundPlayer = nullptr;
    ll::service::getLevel()->forEachPlayer([&](Player& player) {
        if (player.getXuid() == xuid) {
            foundPlayer = &player;
            return false;
        }
        return true;
    });

    return foundPlayer;
}

Player* LeviLaminaAPI::getPlayerByName(std::string name) {
    Player* foundPlayer = nullptr;
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    ll::service::getLevel()->forEachPlayer([&](Player& player) {
        std::string pName = player.mName;
        transform(pName.begin(), pName.end(), pName.begin(), ::tolower);
        if (pName == name) {
            foundPlayer = &player;
            return false;
        }
        return true;
    });

    return foundPlayer;
}

std::string LeviLaminaAPI::getPlayerNameByXuid(std::string xuid) {
    Player* foundPlayer = getPlayerByXuid(std::move(xuid));
    if (foundPlayer) {
        return {foundPlayer->mName};
    }
    return {};
}

std::string LeviLaminaAPI::getXuidByPlayerName(std::string name) {
    Player* foundPlayer = getPlayerByName(std::move(name));
    if (foundPlayer) {
        return {foundPlayer->getXuid()};
    }
    return {};
}
} // namespace rlx_land
