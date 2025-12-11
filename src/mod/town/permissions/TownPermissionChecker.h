#pragma once

namespace rlx_land {
struct PlayerInfo;
}

namespace rlx_town {

class TownPermissionChecker {
public:
    static bool hasTownPerm(const rlx_land::PlayerInfo& playerInfo, int x, int z, int, int dim, int perm);

    // 不依赖Player的重载方法
    static bool canClaimLand(const rlx_land::PlayerInfo& playerInfo, int x, int z, int dim);
};

} // namespace rlx_town
