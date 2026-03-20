#pragma once

#include "ConfigManager.hpp"
#include <nlohmann/json.hpp>

namespace rlx_land {

/// @brief 领地系统配置数据结构
struct LandConfigData {
    // ===== 侧边栏显示 =====
    bool enableSidebar = true;

    // ===== 经济配置 =====
    int playerInitialMoney = 100000;   // 玩家初始金钱

    // ===== 其他配置 =====
    bool requireMoneyPlugin = false;   // 是否强制要求 RLXMoney 插件
};

// 定义 JSON 序列化（nlohmann/json 宏）
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LandConfigData,
    enableSidebar,
    playerInitialMoney,
    requireMoneyPlugin
);

/// @brief 获取领地配置的便捷访问函数（Header-only 单例模式）
inline const LandConfigData& getLandConfig() {
    return rlx::common::Config<LandConfigData>::getInstance().get();
}

} // namespace rlx_land
