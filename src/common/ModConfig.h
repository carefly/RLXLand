#pragma once

#include <string>

namespace rlx_land {

/// @brief 模组配置管理器
class ModConfig {
public:
    /// @brief 加载配置文件
    /// @param configPath 配置文件路径
    /// @return 是否加载成功
    static bool load(const std::string& configPath = "RLXModeResources/config.json");

    /// @brief 获取配置：是否必须存在 money plugin
    /// @return true 表示必须存在，false 表示可选
    static bool requireMoneyPlugin();

    /// @brief 检查 money.dll 是否存在
    /// @return true 表示存在，false 表示不存在
    static bool checkMoneyDllExists();

private:
    static bool s_requireMoneyPlugin;
    static bool s_configLoaded;

    // DLL 名称硬编码
    static constexpr const char* MONEY_DLL_NAME = "RLXMoney.dll";
};

} // namespace rlx_land
