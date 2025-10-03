#pragma once

#include "LandCore.h"
#include "common/LeviLaminaAPI.h"
#include "data/core/BaseDataManager.h"

namespace rlx_land {

class LandDataManager : public BaseDataManager<LandData, LandInformation> {
protected:
    [[nodiscard]] std::string getFilePath() const override { return JsonLoader::LANDS_JSON_PATH; }

    void initInformation(LandInformation* info) override {
        info->ownerName = LeviLaminaAPI::getPlayerNameByXuid(info->ld.ownerXuid);
    }
};

} // namespace rlx_land