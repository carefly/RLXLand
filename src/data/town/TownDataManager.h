#pragma once
#include "TownCore.h"
#include "common/LeviLaminaAPI.h"
#include "common/exceptions/LandExceptions.h"
#include "data/core/BaseDataManager.h"
#include <algorithm>


namespace rlx_town {

using rlx_land::BaseDataManager;
using rlx_land::JsonLoader;
using rlx_land::LeviLaminaAPI;
using ::PlayerNotFoundException;
using ::RealmNotFoundException;

class TownDataManager : public BaseDataManager<TownData, TownInformation> {
protected:
    void initInformation(TownInformation* info) override {
        // 使用新的访问方式
        info->refreshOwnerName();
    }

public:
    // Town特有的方法
    void transferMayor(TownInformation* ti, const std::string& playerName) {
        if (ti == nullptr) throw RealmNotFoundException("Town not found");

        std::string xuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
        if (xuid.empty()) throw ::PlayerNotFoundException("Player not found: " + playerName);

        // 转让镇长职位
        std::string oldMayorXuid = ti->getMayorXuid();
        ti->setMayorXuid(xuid);

        // 将原镇长添加为成员（如果还不是成员）
        const auto& members = ti->getMemberXuids();
        if (std::find(members.begin(), members.end(), oldMayorXuid) == members.end()) {
            ti->addMember(oldMayorXuid);
        }

        // 从成员列表中移除新镇长
        ti->removeMember(xuid);

        // 保存到文件
        std::vector<TownData> towns = JsonLoader::loadTownsFromFile();
        auto townIt = std::find_if(towns.begin(), towns.end(), [ti](const TownData& t) { return t.id == ti->getId(); });

        if (townIt != towns.end()) {
            townIt->mayorXuid   = ti->getMayorXuid();
            townIt->memberXuids = ti->getMemberXuids();
            JsonLoader::saveTownsToFile(towns);
        }

        // 更新内存中的镇长名（通过 setMayorXuid 已经自动更新）
        // ti->refreshOwnerName(); // 已经在 setMayorXuid 中调用
    }
};

} // namespace rlx_town