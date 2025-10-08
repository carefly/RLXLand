#include "data/core/BaseInformation.h"
#include "mocks/MockLeviLaminaAPI.h"

#ifdef BUILDING_TESTS

namespace rlx_land {

bool BaseInformation::hasBasicPermission(const std::string& xuid) const {
    if (isOwner(xuid)) return true;

    auto it = std::find(data.memberXuids.begin(), data.memberXuids.end(), xuid);
    return it != data.memberXuids.end();
}

bool BaseInformation::isOwner(const std::string& xuid) const { return checkIsOwner(xuid); }

std::string BaseInformation::getOwnerName() const { return ownerName; }

std::string BaseInformation::getMembers() const {
    std::string memberNames;
    for (size_t i = 0; i < data.memberXuids.size(); ++i) {
        if (i > 0) memberNames += ",";
        // 使用模拟API而不是真实的LeviLaminaAPI
        memberNames += test::mock::MockLeviLaminaAPI::getPlayerNameByXuid(data.memberXuids[i]);
    }
    return memberNames;
}

} // namespace rlx_land

#endif // BUILDING_TESTS