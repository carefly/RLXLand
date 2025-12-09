#include "mod/RLXLand.h"
#include "common/ModConfig.h"
#include "data/core/PlayerEconomyData.h"
#include "data/service/DataService.h"
#include "mod/events/CommonEventHandlers.h"
#include "mod/land/commands/LandCommands.h"
#include "mod/town/commands/TownCommands.h"


namespace rlx_land {

RLXLand& RLXLand::getInstance() {
    static RLXLand instance;
    return instance;
}

bool RLXLand::load() {
    // 加载配置文件
    ModConfig::load();

    // 检查 money DLL
    bool dllExists     = ModConfig::checkMoneyDllExists();
    bool requirePlugin = ModConfig::requireMoneyPlugin();

    if (!dllExists) {
        if (requirePlugin) {
            getSelf().getLogger().error("RLXMoney plugin is required but not found. Please install RLXMoney plugin.");
            return false;
        } else {
            getSelf().getLogger().warn("RLXMoney plugin not found. Running in local mode (money data will not persist)."
            );
        }
    } else {
        getSelf().getLogger().info("RLXMoney plugin found and will be used.");
    }

    // 初始化玩家经济数据（会根据 DLL 可用性自动选择模式）
    try {
        PlayerEconomyData::initialize();
    } catch (const std::exception& e) {
        getSelf().getLogger().error("Failed to initialize PlayerEconomyData: {}", e.what());
        if (requirePlugin) {
            return false;
        }
        // 如果 DLL 是可选的，继续运行
    }

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
