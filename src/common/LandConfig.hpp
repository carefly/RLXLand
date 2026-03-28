#pragma once

#include "ConfigManager.hpp"
#include <nlohmann/json.hpp>

namespace rlx_land {

/// @brief 领地系统配置数据结构
struct LandConfigData {
    // ===== 侧边栏显示 =====
    bool enableSidebar = true;

    // ===== 经济配置 =====
    bool enableEconomy = true; // 是否启用经济系统（需要 RLXMoney 插件）
};

// 定义 JSON 序列化（nlohmann/json 宏）
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LandConfigData, enableSidebar, enableEconomy);

/// @brief 获取领地配置的便捷访问函数（Header-only 单例模式）
inline const LandConfigData& getLandConfig() { return rlx::common::Config<LandConfigData>::getInstance().get(); }

} // namespace rlx_land
