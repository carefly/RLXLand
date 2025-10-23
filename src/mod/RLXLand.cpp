#include "mod/RLXLand.h"
#include "data/core/PlayerEconomyData.h"
#include "data/service/DataService.h"
#include "mod/land/commands/LandCommands.h"
#include "mod/town/commands/TownCommands.h"
#include "mod/events/CommonEventHandlers.h"


namespace rlx_land {

RLXLand& RLXLand::getInstance() {
    static RLXLand instance;
    return instance;
}

bool RLXLand::load() {
    // 初始化玩家经济数据
    PlayerEconomyData::initialize();

    // 初始化数据服务
    DataService::getInstance()->initialize();

    // 注册命令
    LandCommands::registerCommands();
    TownCommands::registerCommands();

    // 注册事件处理器
    CommonEventHandlers::registerEventListeners();
    CommonEventHandlers::hookAllFunctions();

    return true;
}

bool RLXLand::enable() {
    // 启用模组时的日志输出
    getSelf().getLogger().info("RLXLand mod enabled");
    return true;
}

bool RLXLand::disable() {
    // 保存玩家经济数据
    PlayerEconomyData::save();

    // 禁用模组时的日志输出
    getSelf().getLogger().info("RLXLand mod disabled");
    return true;
}

} // namespace rlx_land
