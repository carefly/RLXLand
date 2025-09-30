#pragma once

class Player;
class Vec3;
class Actor;
class ActorDamageSource;

#include <string>

namespace rlx_land {

class LandPermissionChecker {
public:
    static std::string showPerm(int perm, bool isOwner = false);
    static bool        hasPerm(Player* p, Vec3 pos, int perm);
    static bool        canHurt(Actor& actor, ActorDamageSource const& source);

private:
    static void NoticePerm(Player* p, std::string ownerName, int perm);
};

} // namespace rlx_land
