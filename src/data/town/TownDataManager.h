#pragma once
#include "TownCore.h"
#include "common/LeviLaminaAPI.h"
#include "common/exceptions/LandExceptions.h"
#include "data/core/BaseDataManager.h"
#include <algorithm>


namespace rlx_land {

class TownDataManager : public BaseDataManager<TownData, TownInformation> {
protected:
    void initInformation(TownInformation* info) override {
        info->mayorName = LeviLaminaAPI::getPlayerNameByXuid(info->td.mayorXuid);
    }

public:
    // Town特有的方法
    void transferMayor(TownInformation* ti, const std::string& playerName) {
        if (ti == nullptr) throw LandNotFoundException("Town not found");

        std::string xuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
        if (xuid.empty()) throw PlayerNotFoundException("Player not found: " + playerName);

        // 转让镇长职位
        std::string oldMayorXuid = ti->td.mayorXuid;
        ti->td.mayorXuid         = xuid;

        // 将原镇长添加为成员（如果还不是成员）
        if (std::find(ti->td.memberXuids.begin(), ti->td.memberXuids.end(), oldMayorXuid) == ti->td.memberXuids.end()) {
            ti->td.memberXuids.push_back(oldMayorXuid);
        }

        // 从成员列表中移除新镇长
        auto it = std::find(ti->td.memberXuids.begin(), ti->td.memberXuids.end(), xuid);
        if (it != ti->td.memberXuids.end()) {
            ti->td.memberXuids.erase(it);
        }

        // 保存到文件
        std::vector<TownData> towns = JsonLoader::loadTownsFromFile();
        auto townIt = std::find_if(towns.begin(), towns.end(), [ti](const TownData& t) { return t.id == ti->td.id; });

        if (townIt != towns.end()) {
            townIt->mayorXuid   = ti->td.mayorXuid;
            townIt->memberXuids = ti->td.memberXuids;
            JsonLoader::saveTownsToFile(towns);
        }

        // 更新内存中的镇长名
        ti->mayorName = LeviLaminaAPI::getPlayerNameByXuid(xuid);
    }
};

} // namespace rlx_land