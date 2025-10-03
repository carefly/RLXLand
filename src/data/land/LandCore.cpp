#include "LandCore.h"
#include "common/LeviLaminaAPI.h"

namespace rlx_land {

LandInformation::LandInformation(LandData ld) : BaseInformation(static_cast<BaseData&>(ld)) {
    ownerName = LeviLaminaAPI::getPlayerNameByXuid(ld.ownerXuid);
}

bool LandInformation::checkIsOwner(const std::string& xuid) const {
    return static_cast<const LandData&>(data).ownerXuid == xuid;
}

// 获取成员名称,用逗号分隔
std::string LandInformation::getMembers() const {
    std::string memberNames;
    const auto& members = static_cast<const LandData&>(data).memberXuids;

    for (size_t i = 0; i < members.size(); ++i) {
        if (i > 0) {
            memberNames += ",";
        }
        memberNames += LeviLaminaAPI::getPlayerNameByXuid(members[i]);
    }

    return memberNames;
}

} // namespace rlx_land