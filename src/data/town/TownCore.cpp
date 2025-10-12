#include "TownCore.h"
#include "common/LeviLaminaAPI.h"
#include <string>

namespace rlx_land {

TownInformation::TownInformation(TownData td) : BaseInformation(static_cast<BaseData&>(td)), townData(std::move(td)) {
    setOwnerName(LeviLaminaAPI::getPlayerNameByXuid(townData.mayorXuid));
}

bool TownInformation::checkIsOwner(const std::string& xuid) const { return townData.mayorXuid == xuid; }

void TownInformation::setMayorXuid(const std::string& xuid) {
    townData.mayorXuid = xuid;
    setOwnerName(LeviLaminaAPI::getPlayerNameByXuid(xuid));
}

} // namespace rlx_land