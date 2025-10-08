#include "data/land/LandCore.h"
#include "mocks/MockLeviLaminaAPI.h"

#ifdef BUILDING_TESTS

namespace rlx_land {

// 在测试环境中重写LandInformation构造函数
LandInformation::LandInformation(LandData ld) : BaseInformation(static_cast<BaseData&>(ld)), ld(std::move(ld)) {
    // 使用模拟API而不是真实的LeviLaminaAPI
    ownerName = test::mock::MockLeviLaminaAPI::getPlayerNameByXuid(this->ld.ownerXuid);
}

bool LandInformation::checkIsOwner(const std::string& xuid) const {
    return static_cast<const LandData&>(data).ownerXuid == xuid;
}

} // namespace rlx_land

#endif // BUILDING_TESTS