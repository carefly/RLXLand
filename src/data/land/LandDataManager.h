#pragma once

#include "LandCore.h"
#include "common/LeviLaminaAPI.h"
#include "data/core/BaseDataManager.h"

namespace rlx_land {

class LandDataManager : public BaseDataManager<LandData, LandInformation> {
protected:
    void initInformation(LandInformation* info) override {
        info->ownerName = LeviLaminaAPI::getPlayerNameByXuid(info->ld.ownerXuid);
    }
};

} // namespace rlx_land