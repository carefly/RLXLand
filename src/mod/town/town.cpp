#include "town.h"
#include "commands/TownCommands.h"
#include "mod/RLXLand.h"


#include <basetsd.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>
#include <ll/api/service/Bedrock.h>
#include <mc/server/ServerPlayer.h>

using namespace std;

namespace rlx_land {

static std::unique_ptr<Town> instance;

Town& Town::getInstance() { return *instance; }

bool Town::load() { return true; }

bool Town::enable() {
    rlx_land::RLXLand::getInstance().getSelf().getLogger().info("town load start");
    // 加载Town数据

    rlx_land::RLXLand::getInstance().getSelf().getLogger().info("town load completed");

    // 注册命令
    TownCommands::registerCommands();

    return true;
}

bool Town::disable() { return true; }

} // namespace rlx_land
