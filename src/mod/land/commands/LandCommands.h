#pragma once

#include <mc/server/commands/CommandRawText.h>
#include <string>
#include <unordered_map>
#include <utility>

namespace rlx_land {

enum LandCommandBasicOperation : int { buy = 0, a = 1, b = 2, exit = 3 };
enum LandCommandSellOperation : int { sell = 0 };
enum LandCommandQueryOperation : int { query = 0 };
enum LandCommandTrustOperation : int { trust = 0, untrust = 1 };
enum LandCommandPermOperation : int { perm = 0 };

struct LandBasicCommand {
    LandCommandBasicOperation Operation{static_cast<LandCommandBasicOperation>(0)};
};

struct LandSellCommand {
    LandCommandSellOperation Operation{static_cast<LandCommandSellOperation>(0)};
};

struct LandQueryCommand {
    LandCommandQueryOperation Operation{static_cast<LandCommandQueryOperation>(0)};
};

struct LandTrustCommand {
    LandCommandTrustOperation Operation{static_cast<LandCommandTrustOperation>(0)};
    CommandRawText            Name;
};

struct LandPermCommand {
    LandCommandPermOperation Operation{static_cast<LandCommandPermOperation>(0)};
    int                      Perm{0};
};

enum LandBuyState : char { N = 0, A = 1, B = 2 };

extern std::unordered_map<std::string, LandBuyState>   landBuyState;
extern std::unordered_map<std::string, std::pair<int, int>> landBuyA;
extern std::unordered_map<std::string, std::pair<int, int>> landBuyB;

class LandCommands {
public:
    static void registerCommands();
};

} // namespace rlx_land
