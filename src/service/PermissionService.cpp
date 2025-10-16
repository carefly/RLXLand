#include "service/PermissionService.h"
#include "common/LeviLaminaAPI.h"
#include <mc/world/actor/player/Player.h>

namespace rlx_land {

PermissionService& PermissionService::getInstance() {
    static PermissionService instance;
    return instance;
}

bool PermissionService::isOperator(Player const* player) const { return player->isOperator(); }

bool PermissionService::isOperatorByXuid(const std::string& xuid) const {
    // 通过LeviLaminaAPI获取Player对象，然后检查是否为OP
    auto* player = LeviLaminaAPI::getPlayerByXuid(xuid);
    return player ? player->isOperator() : false;
}

} // namespace rlx_land