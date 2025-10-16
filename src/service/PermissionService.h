#pragma once

#include <string>
class Player;

namespace rlx_land {

class PermissionService {
public:
    /**
     * @brief 获取权限服务单例实例
     * @return 权限服务实例的引用
     */
    static PermissionService& getInstance();

    /**
     * @brief 检查玩家是否为操作员(腐竹)
     * @param player 玩家指针
     * @return 如果玩家是操作员则返回true，否则返回false
     */
    bool isOperator(Player const* player) const;
    bool isOperatorByXuid(const std::string& xuid) const; // 新增


private:
    PermissionService() = default;
};

} // namespace rlx_land