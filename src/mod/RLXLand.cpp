#include "mod/RLXLand.h"
#include "common/ConfigManager.hpp"
#include "common/LandConfig.hpp"
#include "data/core/PlayerEconomyData.h"
#include "data/service/DataService.h"
#include "mod/events/CommonEventHandlers.h"
#include "mod/land/commands/LandCommands.h"
#include "mod/town/commands/TownCommands.h"
#include "plugin/RLXStatus.h"
#include <ll/api/mod/RegisterHelper.h>


namespace rlx_land {

RLXLand& RLXLand::getInstance() {
    static RLXLand instance;
    return instance;
}

bool RLXLand::load() const {
    // 加载配置文件
    getSelf().getLogger().info("RLXLand 模组开始加载");

    // 初始化配置系统
    try {
        rlx::common::Config<LandConfigData>::initWithName("land_config.json");
        getSelf().getLogger().info("配置系统已初始化");
    } catch (const std::exception& e) {
        getSelf().getLogger().error("配置初始化失败：{}", e.what());
        return false;
    }

    // 检查 money DLL 和经济系统配置
    bool dllExists = rlx::common::checkDllExists("RLXMoney.dll", {"plugins/RLXMoney", "../plugins/RLXMoney"});
    bool enableEconomy = rlx_land::getLandConfig().enableEconomy;

    if (enableEconomy) {
        if (!dllExists) {
            getSelf().getLogger().error("经济系统已启用但未找到 RLXMoney 插件，请安装 RLXMoney 插件或在配置中禁用经济系统。");
            return false;
        }
        getSelf().getLogger().info("经济系统已启用，已找到 RLXMoney 插件。");
    } else {
        if (dllExists) {
            getSelf().getLogger().info("经济系统已禁用，RLXMoney 插件将被忽略。");
        } else {
            getSelf().getLogger().info("经济系统已禁用，土地免费申请。");
        }
    }

    // 初始化玩家经济数据（会根据 DLL 可用性自动选择模式）
    try {
        PlayerEconomyData::initialize();
    } catch (const std::exception& e) {
        getSelf().getLogger().error("初始化 PlayerEconomyData 失败：{}", e.what());
        if (enableEconomy) {
            return false;
        }
        // 如果经济系统未启用，继续运行
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
    
    // 根据配置决定是否启用状态显示（侧边栏）
    if (rlx_land::getLandConfig().enableSidebar) {
        RLXStatus::getInstance().load();
        RLXStatus::getInstance().enable();
        getSelf().getLogger().info("侧边栏已启用");
    } else {
        getSelf().getLogger().info("侧边栏已禁用（根据配置）");
    }
    
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