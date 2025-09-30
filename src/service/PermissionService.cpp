#include "service/PermissionService.h"
#include <mc/world/actor/player/Player.h>

namespace rlx_land {

PermissionService& PermissionService::getInstance() {
    static PermissionService instance;
    return instance;
}

bool PermissionService::isOperator(Player const* player) const { return player->isOperator(); }

} // namespace rlx_land