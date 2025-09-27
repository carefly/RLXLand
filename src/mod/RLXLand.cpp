#include "mod/RLXLand.h"

#include "ll/api/mod/RegisterHelper.h"

namespace rlx_land {

RLXLand& RLXLand::getInstance() {
    static RLXLand instance;
    return instance;
}

bool RLXLand::load() {
    getSelf().getLogger().debug("Loading...");
    // Code for loading the mod goes here.
    return true;
}

bool RLXLand::enable() {
    getSelf().getLogger().debug("Enabling...");
    // Code for enabling the mod goes here.
    return true;
}

bool RLXLand::disable() {
    getSelf().getLogger().debug("Disabling...");
    // Code for disabling the mod goes here.
    return true;
}

} // namespace rlx_land

LL_REGISTER_MOD(rlx_land::RLXLand, rlx_land::RLXLand::getInstance());
