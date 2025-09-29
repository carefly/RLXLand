#include "DataManager.h"
#include "LandCore.h"
#include "common/LeviLaminaAPI.h"
#include "common/RLXJsonLoader.h"
#include "common/exceptions/LandExceptions.h"
#include "mod/RLXLand.h"
#include <algorithm>
#include <memory>

namespace rlx_land {

std::shared_ptr<DataManager> DataManager::getInstance() {
    static std::shared_ptr<DataManager> instance = std::make_shared<DataManager>();
    return instance;
}

LONG64 DataManager::getLandMaxId() {
    LONG64 max = 0;
    for (const auto& landInfo : getInstance()->landInformationList) {
        if (max < landInfo->ld.id) max = landInfo->ld.id;
    }

    return max;
}

void DataManager::loadLands() {

    std::vector<LandData> lands = rlx_land::RLXJsonLoader::loadLandsFromFile(rlx_land::RLXJsonLoader::LANDS_JSON_PATH);

    rlx_land::RLXLand::getInstance().getSelf().getLogger().info(std::format("load {} lands from JSON", lands.size()));

    for (const auto& land : lands) {
        LONG64 x1 = land.x;
        LONG64 x2 = land.dx;
        LONG64 z1 = land.z;
        LONG64 z2 = land.dz;
        int    d  = land.d;

        if (x1 >= LAND_RANGE || x1 <= -LAND_RANGE || x2 >= LAND_RANGE || x2 <= -LAND_RANGE || z1 >= LAND_RANGE
            || z1 <= -LAND_RANGE || z2 >= LAND_RANGE || z2 <= -LAND_RANGE)
            continue;

        auto li = new LandInformation(land);

        li->ownerName = LeviLaminaAPI::getPlayerNameByXuid(land.ownerXuid);

        this->landInformationList.push_back(li);

        for (LONG64 xi = x1; xi <= x2; xi++)
            for (LONG64 zi = z1; zi <= z2; zi++) {
                LandMap::getInstance()->set(li, xi, zi, d);
            }
    }
}

void DataManager::createLand(LandData data) {

    std::vector<LandData> lands = rlx_land::RLXJsonLoader::loadLandsFromFile(rlx_land::RLXJsonLoader::LANDS_JSON_PATH);

    auto it = std::find_if(lands.begin(), lands.end(), [&data](const LandData& l) { return l.id == data.id; });

    if (it != lands.end()) {
        throw LandDuplicateException("Land duplicate with ID: " + std::to_string(data.id));
    }

    lands.push_back(data);

    rlx_land::RLXJsonLoader::saveLandsToFile(rlx_land::RLXJsonLoader::LANDS_JSON_PATH, lands);

    auto li = new LandInformation(data);

    li->ownerName = LeviLaminaAPI::getPlayerNameByXuid(data.ownerXuid);

    this->landInformationList.push_back(li);

    for (LONG64 x = data.x; x <= data.dx; x++) {
        for (LONG64 z = data.z; z <= data.dz; z++) {
            LandMap::getInstance()->set(li, x, z, data.d);
        }
    }
}

void DataManager::deleteLand(LandData data) {
    auto infoIt = std::find_if(landInformationList.begin(), landInformationList.end(), [&data](LandInformation* li) {
        return li->ld.id == data.id;
    });

    if (infoIt != landInformationList.end()) {
        delete *infoIt;
        landInformationList.erase(infoIt);
    }

    std::vector<LandData> lands = rlx_land::RLXJsonLoader::loadLandsFromFile(rlx_land::RLXJsonLoader::LANDS_JSON_PATH);

    auto it = std::find_if(lands.begin(), lands.end(), [&data](const LandData& l) { return l.id == data.id; });

    if (it == lands.end()) {
        throw LandNotFoundException("Land not found with ID: " + std::to_string(data.id));
    }

    lands.erase(it);

    rlx_land::RLXJsonLoader::saveLandsToFile(rlx_land::RLXJsonLoader::LANDS_JSON_PATH, lands);

    for (LONG64 xi = data.x; xi <= data.dx; xi++)
        for (LONG64 zi = data.z; zi <= data.dz; zi++) {
            LandMap::getInstance()->set(nullptr, xi, zi, data.d);
        }
}

void DataManager::modifyLandPerm(LandInformation* li, int perm) {
    if (li == nullptr) throw LandNotFoundException("Land not found");

    li->ld.perm = perm;

    std::vector<LandData> lands = rlx_land::RLXJsonLoader::loadLandsFromFile(rlx_land::RLXJsonLoader::LANDS_JSON_PATH);

    auto it = std::find_if(lands.begin(), lands.end(), [&li](const LandData& l) { return l.id == li->ld.id; });

    if (it != lands.end()) {
        it->perm = perm;
        rlx_land::RLXJsonLoader::saveLandsToFile(rlx_land::RLXJsonLoader::LANDS_JSON_PATH, lands);
    }
}

void DataManager::addLandMember(LandInformation* li, const std::string& playerName) {
    if (li == nullptr) throw LandNotFoundException("Land not found");

    std::string xuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
    if (xuid.empty()) throw LandMemberException("Player not found: " + playerName);

    // 检查玩家是否已经是成员
    if (find(li->ld.memberXuids.begin(), li->ld.memberXuids.end(), xuid) != li->ld.memberXuids.end()) {
        throw LandMemberException("Player is already a member: " + playerName);
    }

    // 添加新成员
    li->ld.memberXuids.push_back(xuid);

    std::vector<LandData> lands = rlx_land::RLXJsonLoader::loadLandsFromFile(rlx_land::RLXJsonLoader::LANDS_JSON_PATH);

    auto it = std::find_if(lands.begin(), lands.end(), [&li](const LandData& l) { return l.id == li->ld.id; });

    if (it != lands.end()) {
        it->memberXuids = li->ld.memberXuids;
        rlx_land::RLXJsonLoader::saveLandsToFile(rlx_land::RLXJsonLoader::LANDS_JSON_PATH, lands);
    }
}

void DataManager::removeLandMember(LandInformation* li, const std::string& playerName) {
    if (li == nullptr) throw LandNotFoundException("Land not found");

    std::string xuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
    if (xuid.empty()) throw LandMemberException("Player not found: " + playerName);

    // 查找并移除成员
    auto it = find(li->ld.memberXuids.begin(), li->ld.memberXuids.end(), xuid);
    if (it == li->ld.memberXuids.end()) {
        throw LandMemberException("Player is not a member: " + playerName);
    }

    li->ld.memberXuids.erase(it);

    std::vector<LandData> lands = rlx_land::RLXJsonLoader::loadLandsFromFile(rlx_land::RLXJsonLoader::LANDS_JSON_PATH);

    auto it2 = std::find_if(lands.begin(), lands.end(), [&li](const LandData& l) { return l.id == li->ld.id; });

    if (it2 != lands.end()) {
        it2->memberXuids = li->ld.memberXuids;
        rlx_land::RLXJsonLoader::saveLandsToFile(rlx_land::RLXJsonLoader::LANDS_JSON_PATH, lands);
    }
}

} // namespace rlx_land