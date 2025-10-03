#include "LandCore.h"
#include "common/LeviLaminaAPI.h"

namespace rlx_land {

LandInformation::LandInformation(LandData ld) : BaseInformation(static_cast<BaseData&>(ld)) {
    ownerName = LeviLaminaAPI::getPlayerNameByXuid(ld.ownerXuid);
}

bool LandInformation::checkIsOwner(const std::string& xuid) const {
    return static_cast<const LandData&>(data).ownerXuid == xuid;
}

} // namespace rlx_land