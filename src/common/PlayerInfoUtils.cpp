#include "PlayerInfoUtils.h"
#include "LeviLaminaAPI.h"
#include "service/PermissionService.h"

namespace rlx_land {

PlayerInfo PlayerInfoUtils::fromXuid(const std::string& xuid) {
    if (xuid.empty()) return PlayerInfo("", "", false);

    return PlayerInfo(
        xuid,
        LeviLaminaAPI::getPlayerNameByXuid(xuid),
        PermissionService::getInstance().isOperatorByXuid(xuid)
    );
}

} // namespace rlx_land