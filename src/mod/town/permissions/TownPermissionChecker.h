#pragma once

namespace rlx_land {

// 前向声明
struct PlayerInfo;

class TownPermissionChecker {
public:
    static bool hasTownPerm(const PlayerInfo& playerInfo, int x, int z, int, int dim, int perm);

    // 不依赖Player的重载方法
    static bool canClaimLand(const PlayerInfo& playerInfo, int x, int z, int dim);
};

} // namespace rlx_land
