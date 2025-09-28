#include "LeviLaminaAPI.h"

#include <ll/api/service/Bedrock.h>
#include <mc/world/level/Level.h>

Player* LeviLaminaAPI::getPlayerByXuid(string xuid) {
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

Player* LeviLaminaAPI::getPlayerByName(string name) {
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

string LeviLaminaAPI::getPlayerNameByXuid(string xuid) {
    Player* foundPlayer = getPlayerByXuid(std::move(xuid));
    if (foundPlayer) {
        return {foundPlayer->mName};
    }
    return {};
}

string LeviLaminaAPI::getXuidByPlayerName(string name) {
    Player* foundPlayer = getPlayerByName(std::move(name));
    if (foundPlayer) {
        return {foundPlayer->getXuid()};
    }
    return {};
}