#include "Land.h"
#include "commands/LandCommands.h"
#include "data/service/DataService.h"
#include "mod/events/CommonEventHandlers.h"
#include "mod/RLXLand.h"


namespace rlx_land {

static std::unique_ptr<Land> instance;

Land& Land::getInstance() { return *instance; }

bool Land::load() { return true; }
bool Land::enable() {
    RLXLand::getInstance().getSelf().getLogger().info("land load start");
    DataService::getInstance()->loadItems<LandData>();
    RLXLand::getInstance().getSelf().getLogger().info("land load completed");

    // 注册命令
    LandCommands::registerCommands();

    // 注册事件监听器
    CommonEventHandlers::registerEventListeners();

    // Hook函数注册
    CommonEventHandlers::hookAllFunctions();

    return true;
}
bool Land::disable() { return true; }

} // namespace rlx_land
