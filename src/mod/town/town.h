#pragma once

#
class Player;
class Vec3;

namespace rlx_land {

class TownInformation;


class Town {

public:
    static Town& getInstance();

    /// @return True if the plugin is loaded successfully.
    bool load();

    /// @return True if the plugin is enabled successfully.
    bool enable();

    /// @return True if the plugin is disabled successfully.
    bool disable();

    TownInformation* getTownAt(Vec3 pos, int dim);

private:
    void registerCommands();


    static bool hasTownPerm(Player* player, Vec3 pos, int perm);
};

} // namespace rlx_land