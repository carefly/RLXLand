#include "BaseInformation.h"
#include "common/LeviLaminaAPI.h"
#include <algorithm>

namespace rlx_land {

BaseInformation::BaseInformation(BaseData data) : data(std::move(data)) {}

bool BaseInformation::hasBasicPermission(const std::string& xuid) const {
    if (isOwner(xuid)) return true;

    auto it = std::find(data.memberXuids.begin(), data.memberXuids.end(), xuid);
    return it != data.memberXuids.end();
}

bool BaseInformation::isOwner(const std::string& xuid) const { return checkIsOwner(xuid); }

std::string BaseInformation::getMembers() const {
    std::string memberNames;
    for (size_t i = 0; i < data.memberXuids.size(); ++i) {
        if (i > 0) memberNames += ",";
        memberNames += LeviLaminaAPI::getPlayerNameByXuid(data.memberXuids[i]);
    }
    return memberNames;
}

void BaseInformation::addMember(const std::string& xuid) {
    // 检查是否已经是成员
    auto it = std::find(data.memberXuids.begin(), data.memberXuids.end(), xuid);
    if (it == data.memberXuids.end()) {
        data.memberXuids.push_back(xuid);
    }
}

void BaseInformation::removeMember(const std::string& xuid) {
    auto it = std::find(data.memberXuids.begin(), data.memberXuids.end(), xuid);
    if (it != data.memberXuids.end()) {
        data.memberXuids.erase(it);
    }
}

} // namespace rlx_land