#pragma once

class Player;
class Vec3;
class Actor;
class ActorDamageSource;

#include <string>

namespace rlx_land {

enum LandPerm : int {
    PERM_NULL           = 0,
    PERM_ATK            = 1,
    PERM_USE_ON         = 2,
    PERM_VILLAGER_ATK   = 4,
    PERM_BUILD          = 8,
    PERM_POPITEM        = 16,
    PERM_INTERWITHACTOR = 32,
    PERM_AMRORSTANDER   = 64,
    PERM_FISHINGHOOK    = 128,
    PERM_FIRE           = 256
};

class LandPermissionChecker {
public:
    static std::string showPerm(int perm, bool isOwner = false);
    static bool        hasPerm(Player* p, Vec3 pos, int perm);
    static bool        canHurt(Actor& actor, ActorDamageSource const& source);

private:
    static void NoticePerm(Player* p, std::string ownerName, int perm);
};

} // namespace rlx_land
