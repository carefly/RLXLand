#pragma once

#include <mc/server/ServerPlayer.h>
#include <string>
#include <utility>
#include <vector>



class Player;

namespace rlx_land {

class RLXScoreboard {
public:
    /// @brief 设置客户端侧边栏
    /// @param title 侧边栏标题
    /// @param data 侧边栏数据，pair<显示文本, 分数>
    /// @param player 目标玩家
    static void
    setClientSidebar(const std::string& title, const std::vector<std::pair<std::string, int>>& data, Player* player);

private:
    static void removeClientSidebar(Player* pl);
};

} // namespace rlx_land
