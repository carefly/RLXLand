#include "data/core/BaseInformation.h"
#include "mocks/MockLeviLaminaAPI.h"
#include <algorithm>

#ifdef BUILDING_TESTS

namespace rlx_land {

BaseInformation::BaseInformation(BaseData data) : data(std::move(data)) {}

bool BaseInformation::hasBasicPermission(const std::string& xuid) const {
    if (isOwner(xuid)) return true;

    const auto& members = getMemberXuids();
    auto        it      = std::find(members.begin(), members.end(), xuid);
    return it != members.end();
}

bool BaseInformation::isOwner(const std::string& xuid) const { return checkIsOwner(xuid); }

std::string BaseInformation::getMembers() const {
    std::string memberNames;
    const auto& members = getMemberXuids();
    for (size_t i = 0; i < members.size(); ++i) {
        if (i > 0) memberNames += ",";
        // 使用模拟API而不是真实的LeviLaminaAPI
        memberNames += test::mock::MockLeviLaminaAPI::getPlayerNameByXuid(members[i]);
    }
    return memberNames;
}

void BaseInformation::addMember(const std::string& xuid) {
    // 检查是否已经是成员
    const auto& members = getMemberXuids();
    auto        it      = std::find(members.begin(), members.end(), xuid);
    if (it == members.end()) {
        // 需要通过 getBaseData() 来修改成员列表
        getBaseData().memberXuids.push_back(xuid);
    }
}

void BaseInformation::removeMember(const std::string& xuid) {
    // 需要通过 getBaseData() 来修改成员列表
    auto& members = getBaseData().memberXuids;
    auto  it      = std::find(members.begin(), members.end(), xuid);
    if (it != members.end()) {
        members.erase(it);
    }
}

} // namespace rlx_land

#endif // BUILDING_TESTS