#pragma once

class Player;
class Vec3;

namespace rlx_land {

class TownPermissionChecker {
public:
    static bool canClaimLand(Player* player, int x, int z, int dim);
    static bool hasTownPerm(Player* player, Vec3 pos, int perm);
};

} // namespace rlx_land
