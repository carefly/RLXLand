#include "LandCore.h"

namespace rlx_land {

LandInformation::LandInformation(LandData ld) { this->ld = std::move(ld); }

bool LandInformation::hasBasicPermission(const std::string& xuid) const {

    // 检查是否是拥有者
    if (xuid == this->ld.ownerXuid) return true;

    // 检查是否是成员
    if (find(this->ld.memberXuids.begin(), this->ld.memberXuids.end(), xuid) != this->ld.memberXuids.end()) {
        return true;
    } else return false;
}

bool LandInformation::isOwner(const std::string& xuid) const {
    if (xuid == this->ld.ownerXuid) return true;
    else return false;
}

} // namespace rlx_land