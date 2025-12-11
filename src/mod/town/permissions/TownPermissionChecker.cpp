#include "TownPermissionChecker.h"
#include "data/service/DataService.h"

#ifndef TESTING
#include <basetsd.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>
#include <ll/api/service/Bedrock.h>
#include <mc/server/ServerPlayer.h>
#endif

using namespace std;

namespace rlx_town {

// 检查玩家是否可以在指定位置圈地（不依赖Player的重载版本）
bool TownPermissionChecker::canClaimLand(const rlx_land::PlayerInfo& playerInfo, int x, int z, int dim) {

    auto town = rlx_land::DataService::getInstance()->findTownAt(x, z, dim);

    // 如果不在任何Town中，则在Wilderness中，允许圈地
    if (town == nullptr) {
        return true;
    }

    // 如果在Town中，玩家必须是Town的成员（包括镇长）
    if (town->hasBasicPermission(playerInfo.xuid) || playerInfo.isOperator) {
        return true;
    }

    // 不是Town的成员，不允许圈地
    return false;
}

// 检查玩家在指定位置是否有权限
bool TownPermissionChecker::hasTownPerm(
    const rlx_land::PlayerInfo& playerInfo,
    int                         x,
    int                         z,
    int,
    int dim,
    int perm
) {
    // 检查玩家在指定位置是否有权限
    auto town = rlx_land::DataService::getInstance()->findTownAt(x, z, dim);

    // 检查玩家是否具有全局操作权限（腐竹）
    if (playerInfo.isOperator) {
        return true;
    }

    // 如果没有Town，则默认允许
    if (town == nullptr) {
        return true;
    }

    // 检查是否为Town的成员或镇长（包括镇长）
    if (town->hasBasicPermission(playerInfo.xuid)) {
        // 是Town成员（包括镇长），默认拥有所有权限
        return true;
    }

    // 非成员玩家，检查Town的权限设置
    if (town->getPermission() & perm) {
        return true;
    }

    return false;
}

} // namespace rlx_town
