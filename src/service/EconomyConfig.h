#pragma once

namespace rlx_land {

// 经济系统配置
class EconomyConfig {
public:
    // 领地价格（每方块）
    static const int LAND_PRICE_PER_BLOCK = 1;
    
    // 玩家初始金钱
    static const int PLAYER_INITIAL_MONEY = 1000;
    
    // 其他经济相关配置可以在这里添加
};

} // namespace rlx_land