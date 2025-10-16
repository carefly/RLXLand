#pragma once
#include "data/service/DataService.h"

namespace rlx_land {

class PlayerInfoUtils {
public:
    // 从xuid创建PlayerInfo（生产环境使用）
    static PlayerInfo fromXuid(const std::string& xuid);
};

} // namespace rlx_land