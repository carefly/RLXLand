#include "TownPermissionChecker.h"
#include "data/SpatialMap.h"
#include "data/TownCore.h"
#include "service/PermissionService.h"


#include <basetsd.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>
#include <ll/api/service/Bedrock.h>
#include <mc/server/ServerPlayer.h>

using namespace std;

namespace rlx_land {

// 检查玩家是否可以在指定位置圈地
bool TownPermissionChecker::canClaimLand(Player* player, int x, int z, int dim) {
    // 检查玩家是否可以在指定位置圈地
    // 1. 如果在Wilderness中，允许圈地
    // 2. 如果在Town中，玩家必须是Town成员

    auto town = TownMap::getInstance()->find(x, z, dim);

    // 如果不在任何Town中，则在Wilderness中，允许圈地
    if (town == nullptr) {
        return true;
    }

    // 如果在Town中，玩家必须是Town的成员或镇长
    if (town->hasBasicPermission(player->getXuid()) || town->isOwner(player->getXuid())
        || rlx_land::PermissionService::getInstance().isOperator(player)) {
        return true;
    }

    // 不是Town的成员，不允许圈地
    return false;
}

// 检查玩家在指定位置是否有权限
bool TownPermissionChecker::hasTownPerm(Player* player, Vec3 pos, int perm) {
    // 检查玩家在指定位置是否有权限
    auto town = TownMap::getInstance()->find((LONG64)pos.x, (LONG64)pos.z, (int)pos.y);

    // 检查是否为Town的成员或镇长
    if (town && (town->isOwner(player->getXuid()) || town->hasBasicPermission(player->getXuid()))) {
        // 是Town成员，检查Town权限
        if (town->td.perm & perm) {
            return true;
        }
    }

    // 检查玩家是否具有全局操作权限（腐竹）
    if (rlx_land::PermissionService::getInstance().isOperator(player)) {
        return true;
    }

    return false;
}

} // namespace rlx_land
