#pragma once
#include <utility>

#include "BaseDataManager.h"
#include "LandCore.h"
#include "common/LeviLaminaAPI.h"

namespace rlx_land {

class LandDataManager : public BaseDataManager<LandData, LandInformation> {
protected:
    std::string getFilePath() const override { return JsonLoader::LANDS_JSON_PATH; }

    void initInformation(LandInformation* info) override {
        info->ownerName = LeviLaminaAPI::getPlayerNameByXuid(info->ld.ownerXuid);
    }

public:
    void create(LandData data) {
        BaseDataManager::create(std::move(data));
        // Land特定逻辑可以在这里添加
    }
};

} // namespace rlx_land