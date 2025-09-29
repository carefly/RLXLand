#include "TownCore.h"
#include "common/LeviLaminaAPI.h"
#include <algorithm>
#include <string>

namespace rlx_land {

TownInformation::TownInformation(TownData td) { this->td = std::move(td); }

bool TownInformation::isMayor(const std::string& xuid) const { return this->td.mayorXuid == xuid; }

bool TownInformation::hasBasicPermission(const std::string& xuid) const {
    // 检查玩家是否为镇长
    if (isMayor(xuid)) return true;

    // 检查玩家是否为成员
    return std::ranges::any_of(this->td.memberXuids, [&xuid](const std::string& member) { return member == xuid; });
}

// 获取成员名称,用逗号分隔
std::string TownInformation::getMembers() const {
    std::string memberNames;

    for (size_t i = 0; i < this->td.memberXuids.size(); ++i) {
        if (i > 0) {
            memberNames += ",";
        }
        memberNames += LeviLaminaAPI::getPlayerNameByXuid(this->td.memberXuids[i]);
    }

    return memberNames;
}

} // namespace rlx_land