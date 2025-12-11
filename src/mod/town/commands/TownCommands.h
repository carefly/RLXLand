#pragma once

#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandRawText.h>

namespace rlx_town {

enum TownCommandOperation : int {
    info = 1,
};

enum TownCommandBasicOperation : int { create = 1, remove = 2, list = 3, transfer = 4 };

enum TownCommandMemberOperation : int { member_add = 1, member_del = 2 };

enum TownCommandPermOperation : int { perm = 1 };

struct TownBasicCommand {
    TownCommandBasicOperation Operation{static_cast<TownCommandBasicOperation>(0)};
    CommandRawText            TownName{};
    CommandRawText            PlayerName{};
    int                       X{};
    int                       Z{};
    int                       DX{};
    int                       DZ{};
};

struct TownMemberCommand {
    TownCommandMemberOperation Operation{static_cast<TownCommandMemberOperation>(0)};
    CommandRawText             PlayerName{};
};

struct TownPermCommand {
    TownCommandPermOperation Operation{static_cast<TownCommandPermOperation>(0)};
    int                      Perm{};
};

struct TownInfoCommand {
    TownCommandOperation Operation{static_cast<TownCommandOperation>(0)};
};

class TownCommands {
public:
    static void registerCommands();
};

} // namespace rlx_town