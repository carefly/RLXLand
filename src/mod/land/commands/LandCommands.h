#pragma once

#include <mc/server/commands/CommandRawText.h>

namespace rlx_land {

enum LandCommandBasicOperation : int { buy = 1, sell = 2, query = 3, a = 4, b = 5, exit = 6 };
enum LandCommandTrustOperation : int { trust = 0, untrust = 1 };
enum LandCommandPermOperation : int { perm = 0 };

struct LandBasicCommad {
    LandCommandBasicOperation Operation{static_cast<LandCommandBasicOperation>(0)};
};

struct LandTrustCommand {
    LandCommandTrustOperation Operation{static_cast<LandCommandTrustOperation>(0)};
    CommandRawText            Name;
};

struct LandPermCommand {
    LandCommandPermOperation Operation{static_cast<LandCommandPermOperation>(0)};
    int                      Perm{0};
};

class LandCommands {
public:
    static void registerCommands();
};

} // namespace rlx_land
