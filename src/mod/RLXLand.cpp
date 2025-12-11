#include "mod/RLXLand.h"
#include "common/ModConfig.h"
#include "data/core/PlayerEconomyData.h"
#include "data/service/DataService.h"
#include "mod/events/CommonEventHandlers.h"
#include "mod/land/commands/LandCommands.h"
#include "mod/town/commands/TownCommands.h"
#include <ll/api/mod/RegisterHelper.h>


namespace rlx_land {

RLXLand& RLXLand::getInstance() {
    static RLXLand instance;
    return instance;
}

bool RLXLand::load() const {
    // 加载配置文件
    getSelf().getLogger().info("RLXLand 模组开始加载");

    ModConfig::load();

    // 检查 money DLL
    bool dllExists     = ModConfig::checkMoneyDllExists();
    bool requirePlugin = ModConfig::requireMoneyPlugin();

    if (!dllExists) {
        if (requirePlugin) {
            getSelf().getLogger().error("必须安装 RLXMoney 插件但未找到，请安装 RLXMoney 插件。");
            return false;
        } else {
            getSelf().getLogger().warn("未找到 RLXMoney 插件，将以本地模式运行（金币数据不会持久化）。");
        }
    } else {
        getSelf().getLogger().info("已找到 RLXMoney 插件，将正常使用。");
    }

    // 初始化玩家经济数据（会根据 DLL 可用性自动选择模式）
    try {
        PlayerEconomyData::initialize();
    } catch (const std::exception& e) {
        getSelf().getLogger().error("初始化 PlayerEconomyData 失败：{}", e.what());
        if (requirePlugin) {
            return false;
        }
        // 如果 DLL 是可选的，继续运行
    }


    return true;
}

bool RLXLand::enable() const {

    getSelf().getLogger().info("RLXLand 正在启用中...");

    // 初始化数据服务
    DataService::getInstance()->initialize();

    // 注册命令
    LandCommands::registerCommands();
    rlx_town::TownCommands::registerCommands();
    getSelf().getLogger().info("RLXLand 模组命令已注册");


    // 注册事件处理器
    CommonEventHandlers::registerEventListeners();
    CommonEventHandlers::hookAllFunctions();
    // 启用模组时的日志输出
    getSelf().getLogger().info("RLXLand 模组已启用");
    return true;
}

bool RLXLand::disable() const {
    // 保存玩家经济数据
    PlayerEconomyData::save();

    // 禁用模组时的日志输出
    getSelf().getLogger().info("RLXLand 模组已禁用");
    return true;
}

} // namespace rlx_land

LL_REGISTER_MOD(rlx_land::RLXLand, rlx_land::RLXLand::getInstance());