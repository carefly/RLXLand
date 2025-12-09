#pragma once

#include <string>

namespace rlx_land {

// 测试环境下的简化配置：始终视作无需 money 插件，且 DLL 不存在
class ModConfig {
public:
    static inline bool load(const std::string& = "") { return true; }
    static inline bool requireMoneyPlugin() { return false; }
    static inline bool checkMoneyDllExists() { return false; }
};

} // namespace rlx_land
