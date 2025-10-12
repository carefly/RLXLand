#pragma once

#include "LandCore.h"
#include "data/core/BaseDataManager.h"

namespace rlx_land {

class LandDataManager : public BaseDataManager<LandData, LandInformation> {
protected:
    void initInformation(LandInformation* info) override {
        // 使用新的访问方式
        info->refreshOwnerName();
    }
};

} // namespace rlx_land