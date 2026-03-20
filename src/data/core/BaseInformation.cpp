#include "BaseInformation.h"
#include "common/JsonLoader.h"
#include <algorithm>

namespace rlx_land {

BaseInformation::BaseInformation(BaseData& data) : dataRef(&data) {}

bool BaseInformation::hasBasicPermission(const std::string& xuid) const {
    if (isOwner(xuid)) return true;

    auto it = std::find(dataRef->memberXuids.begin(), dataRef->memberXuids.end(), xuid);
    return it != dataRef->memberXuids.end();
}

bool BaseInformation::isOwner(const std::string& xuid) const { return checkIsOwner(xuid); }

std::string BaseInformation::getMembers() const {
    std::string memberNames;
    for (size_t i = 0; i < dataRef->memberXuids.size(); ++i) {
        if (i > 0) memberNames += ",";
        memberNames += JsonLoader::getPlayerNameWithFallback(dataRef->memberXuids[i]);
    }
    return memberNames;
}

void BaseInformation::addMember(const std::string& xuid) {
    // 检查是否已经是成员
    auto it = std::find(dataRef->memberXuids.begin(), dataRef->memberXuids.end(), xuid);
    if (it == dataRef->memberXuids.end()) {
        dataRef->memberXuids.push_back(xuid);
    }
}

void BaseInformation::removeMember(const std::string& xuid) {
    auto it = std::find(dataRef->memberXuids.begin(), dataRef->memberXuids.end(), xuid);
    if (it != dataRef->memberXuids.end()) {
        dataRef->memberXuids.erase(it);
    }
}

} // namespace rlx_land