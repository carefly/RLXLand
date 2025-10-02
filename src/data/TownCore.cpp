#include "TownCore.h"
#include "common/LeviLaminaAPI.h"
#include <string>

namespace rlx_land {

TownInformation::TownInformation(TownData td) : BaseInformation(static_cast<BaseData&>(td)) {
    ownerName = LeviLaminaAPI::getPlayerNameByXuid(td.mayorXuid);
}

bool TownInformation::checkIsOwner(const std::string& xuid) const {
    return static_cast<const TownData&>(data).mayorXuid == xuid;
}

} // namespace rlx_land