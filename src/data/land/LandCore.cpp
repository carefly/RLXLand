#include "LandCore.h"
#include "common/LeviLaminaAPI.h"

namespace rlx_land {

LandInformation::LandInformation(LandData ld) : BaseInformation(static_cast<BaseData&>(ld)), landData(std::move(ld)) {
    setOwnerName(LeviLaminaAPI::getPlayerNameByXuid(landData.ownerXuid));
}

bool LandInformation::checkIsOwner(const std::string& xuid) const { return landData.ownerXuid == xuid; }

void LandInformation::setOwnerXuid(const std::string& xuid) {
    landData.ownerXuid = xuid;
    setOwnerName(LeviLaminaAPI::getPlayerNameByXuid(xuid));
}

} // namespace rlx_land