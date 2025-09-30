#include "DataManager.h"
#include "LandCore.h"
#include "SpatialMap.h" // 为LandMap和TownMap类型别名提供定义
#include "TownCore.h"
#include "common/JsonLoader.h"
#include "common/LeviLaminaAPI.h"
#include "common/exceptions/LandExceptions.h"
#include "mod/RLXLand.h"
#include <algorithm>
#include <memory>

#define LAND_RANGE 1000000


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

    std::vector<LandData> lands = JsonLoader::loadLandsFromFile(JsonLoader::LANDS_JSON_PATH);

    RLXLand::getInstance().getSelf().getLogger().info(std::format("load {} lands from JSON", lands.size()));

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

    std::vector<LandData> lands = rlx_land::JsonLoader::loadLandsFromFile(rlx_land::JsonLoader::LANDS_JSON_PATH);

    auto it = std::find_if(lands.begin(), lands.end(), [&data](const LandData& l) { return l.id == data.id; });

    if (it != lands.end()) {
        throw DuplicateException("Land duplicate with ID: " + std::to_string(data.id));
    }

    lands.push_back(data);

    rlx_land::JsonLoader::saveLandsToFile(rlx_land::JsonLoader::LANDS_JSON_PATH, lands);

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

    std::vector<LandData> lands = rlx_land::JsonLoader::loadLandsFromFile(rlx_land::JsonLoader::LANDS_JSON_PATH);

    auto it = std::find_if(lands.begin(), lands.end(), [&data](const LandData& l) { return l.id == data.id; });

    if (it == lands.end()) {
        throw LandNotFoundException("Land not found with ID: " + std::to_string(data.id));
    }

    lands.erase(it);

    rlx_land::JsonLoader::saveLandsToFile(rlx_land::JsonLoader::LANDS_JSON_PATH, lands);

    for (LONG64 xi = data.x; xi <= data.dx; xi++)
        for (LONG64 zi = data.z; zi <= data.dz; zi++) {
            LandMap::getInstance()->set(nullptr, xi, zi, data.d);
        }
}

void DataManager::modifyLandPerm(LandInformation* li, int perm) {
    if (li == nullptr) throw LandNotFoundException("Land not found");

    li->ld.perm = perm;

    std::vector<LandData> lands = rlx_land::JsonLoader::loadLandsFromFile(rlx_land::JsonLoader::LANDS_JSON_PATH);

    auto it = std::find_if(lands.begin(), lands.end(), [&li](const LandData& l) { return l.id == li->ld.id; });

    if (it != lands.end()) {
        it->perm = perm;
        rlx_land::JsonLoader::saveLandsToFile(rlx_land::JsonLoader::LANDS_JSON_PATH, lands);
    }
}

void DataManager::addLandMember(LandInformation* li, const std::string& playerName) {
    if (li == nullptr) throw LandNotFoundException("Land not found");

    std::string xuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
    if (xuid.empty()) throw PlayerNotFoundException("Player not found: " + playerName);

    // 检查玩家是否已经是成员
    if (find(li->ld.memberXuids.begin(), li->ld.memberXuids.end(), xuid) != li->ld.memberXuids.end()) {
        throw DuplicateException("Player is already a member: " + playerName);
    }

    // 添加新成员
    li->ld.memberXuids.push_back(xuid);

    std::vector<LandData> lands = rlx_land::JsonLoader::loadLandsFromFile(rlx_land::JsonLoader::LANDS_JSON_PATH);

    auto it = std::find_if(lands.begin(), lands.end(), [&li](const LandData& l) { return l.id == li->ld.id; });

    if (it != lands.end()) {
        it->memberXuids = li->ld.memberXuids;
        rlx_land::JsonLoader::saveLandsToFile(rlx_land::JsonLoader::LANDS_JSON_PATH, lands);
    }
}

void DataManager::removeLandMember(LandInformation* li, const std::string& playerName) {
    if (li == nullptr) throw LandNotFoundException("Land not found");

    std::string xuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
    if (xuid.empty()) throw PlayerNotFoundException("Player not found: " + playerName);

    // 查找并移除成员
    auto it = find(li->ld.memberXuids.begin(), li->ld.memberXuids.end(), xuid);
    if (it == li->ld.memberXuids.end()) {
        throw NotMemberException("Player is not a member: " + playerName);
    }

    li->ld.memberXuids.erase(it);

    std::vector<LandData> lands = JsonLoader::loadLandsFromFile(rlx_land::JsonLoader::LANDS_JSON_PATH);

    auto it2 = std::find_if(lands.begin(), lands.end(), [&li](const LandData& l) { return l.id == li->ld.id; });

    if (it2 != lands.end()) {
        it2->memberXuids = li->ld.memberXuids;
        rlx_land::JsonLoader::saveLandsToFile(rlx_land::JsonLoader::LANDS_JSON_PATH, lands);
    }
}

void DataManager::loadTowns() {
    // 从JSON文件加载Town数据
    std::vector<TownData> towns = JsonLoader::loadTownsFromFile(JsonLoader::TOWNS_JSON_PATH);

    RLXLand::getInstance().getSelf().getLogger().info(std::format("load {} towns from JSON", towns.size()));


    for (const auto& town : towns) {
        auto ti = new TownInformation(town);

        ti->mayorName = LeviLaminaAPI::getPlayerNameByXuid(town.mayorXuid);

        this->townInformationList.push_back(ti);

        // 将Town添加到TownMap中
        TownMap::getInstance()->set(ti, town.x, town.z, town.d);
        for (int x = town.x; x <= town.dx; x++) {
            for (int z = town.z; z <= town.dz; z++) {
                TownMap::getInstance()->set(ti, x, z, town.d);
            }
        }
    }
}

void DataManager::createTown(TownData data) {
    // 在JSON中创建Town
    // 先从文件加载现有数据
    std::vector<TownData> towns = JsonLoader::loadTownsFromFile(JsonLoader::TOWNS_JSON_PATH);

    // 检查ID是否已存在
    auto it = std::find_if(towns.begin(), towns.end(), [&data](const TownData& t) { return t.id == data.id; });

    if (it != towns.end()) {
        throw DuplicateException("Town duplicate with ID: " + std::to_string(data.id));
    }

    // 添加新town到列表
    towns.push_back(data);

    // 保存回文件
    JsonLoader::saveTownsToFile(JsonLoader::TOWNS_JSON_PATH, towns);

    // 添加到内存中
    auto ti = new TownInformation(data);

    ti->mayorName = LeviLaminaAPI::getPlayerNameByXuid(data.mayorXuid);
    this->townInformationList.push_back(ti);

    // 添加到TownMap中
    for (LONG64 x = data.x; x <= data.dx; x++) {
        for (LONG64 z = data.z; z <= data.dz; z++) {
            TownMap::getInstance()->set(ti, x, z, data.d);
        }
    }
}

void DataManager::deleteTown(TownData data) {
    // 从JSON中删除Town
    // 先从文件加载现有数据
    std::vector<TownData> towns = JsonLoader::loadTownsFromFile(JsonLoader::TOWNS_JSON_PATH);

    // 查找并删除指定town
    auto it = std::find_if(towns.begin(), towns.end(), [&data](const TownData& t) { return t.id == data.id; });

    if (it != towns.end()) {
        towns.erase(it);

        // 保存回文件
        JsonLoader::saveTownsToFile(JsonLoader::TOWNS_JSON_PATH, towns);

        // 从内存中删除
        auto memIt = std::find_if(
            this->townInformationList.begin(),
            this->townInformationList.end(),
            [&data](TownInformation* ti) { return ti->td.id == static_cast<long long>(data.id); }
        );

        if (memIt != this->townInformationList.end()) {
            TownInformation* ti = *memIt;
            this->townInformationList.erase(memIt);
            delete ti;
        }
    } else {
        throw LandNotFoundException("Town not found to delete");
    }
}

void DataManager::modifyTownPerm(TownInformation* ti, int perm) {
    // 在JSON中修改Town权限
    // 先从文件加载现有数据
    std::vector<TownData> towns = JsonLoader::loadTownsFromFile(JsonLoader::TOWNS_JSON_PATH);

    // 查找并更新指定town
    auto it = std::find_if(towns.begin(), towns.end(), [ti](const TownData& t) {
        return t.id == static_cast<long long>(ti->td.id);
    });

    if (it != towns.end()) {
        it->perm = perm;

        // 保存回文件
        JsonLoader::saveTownsToFile(JsonLoader::TOWNS_JSON_PATH, towns);
    } else {
        throw LandNotFoundException("Town not found to modify permission");
    }

    // 更新内存中的权限
    ti->td.perm = perm;
}

void DataManager::addTownMember(TownInformation* ti, const std::string& playerName) {
    if (ti == nullptr) throw LandNotFoundException("Town not found");

    std::string xuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
    if (xuid.empty()) throw PlayerNotFoundException("Player not found: " + playerName);

    // 检查玩家是否已经是成员
    if (find(ti->td.memberXuids.begin(), ti->td.memberXuids.end(), xuid) != ti->td.memberXuids.end()) {
        throw DuplicateException("Player is already a member: " + playerName);
    }

    // 添加新成员
    ti->td.memberXuids.push_back(xuid);

    std::vector<TownData> towns = JsonLoader::loadTownsFromFile(JsonLoader::TOWNS_JSON_PATH);

    auto it = std::find_if(towns.begin(), towns.end(), [&ti](const TownData& t) { return t.id == ti->td.id; });

    if (it != towns.end()) {
        it->memberXuids = ti->td.memberXuids;
        JsonLoader::saveTownsToFile(JsonLoader::TOWNS_JSON_PATH, towns);
    }
}

void DataManager::removeTownMember(TownInformation* ti, const std::string& playerName) {
    if (ti == nullptr) throw LandNotFoundException("Town not found");

    std::string xuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
    if (xuid.empty()) throw PlayerNotFoundException("Player not found: " + playerName);

    // 查找并移除成员
    auto it = find(ti->td.memberXuids.begin(), ti->td.memberXuids.end(), xuid);
    if (it == ti->td.memberXuids.end()) {
        throw NotMemberException("Player is not a member: " + playerName);
    }

    ti->td.memberXuids.erase(it);

    std::vector<TownData> towns = JsonLoader::loadTownsFromFile(JsonLoader::TOWNS_JSON_PATH);

    auto it2 = std::find_if(towns.begin(), towns.end(), [&ti](const TownData& t) { return t.id == ti->td.id; });

    if (it2 != towns.end()) {
        it2->memberXuids = ti->td.memberXuids;
        JsonLoader::saveTownsToFile(JsonLoader::TOWNS_JSON_PATH, towns);
    }
}

void DataManager::transferTownMayor(TownInformation* ti, const std::string& playerName) {
    if (ti == nullptr) throw LandNotFoundException("Town not found");

    std::string xuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
    if (xuid.empty()) throw PlayerNotFoundException("Player not found: " + playerName);


    // 转让镇长职位
    std::string oldMayorXuid = ti->td.mayorXuid;
    ti->td.mayorXuid         = xuid;

    // 将原镇长添加为成员（如果还不是成员）
    if (find(ti->td.memberXuids.begin(), ti->td.memberXuids.end(), oldMayorXuid) == ti->td.memberXuids.end()) {
        ti->td.memberXuids.push_back(oldMayorXuid);
    }

    // 从成员列表中移除新镇长
    auto it = find(ti->td.memberXuids.begin(), ti->td.memberXuids.end(), xuid);
    if (it != ti->td.memberXuids.end()) {
        ti->td.memberXuids.erase(it);
    }

    std::vector<TownData> towns = JsonLoader::loadTownsFromFile(JsonLoader::TOWNS_JSON_PATH);

    auto townIt = std::find_if(towns.begin(), towns.end(), [&ti](const TownData& t) { return t.id == ti->td.id; });

    if (townIt != towns.end()) {
        townIt->mayorXuid   = ti->td.mayorXuid;
        townIt->memberXuids = ti->td.memberXuids;
        JsonLoader::saveTownsToFile(JsonLoader::TOWNS_JSON_PATH, towns);
    }

    // 更新内存中的镇长名
    ti->mayorName = LeviLaminaAPI::getPlayerNameByXuid(xuid);
}

LONG64 DataManager::getTownMaxId() {
    LONG64 max = 0;
    for (const auto& townInfo : getInstance()->townInformationList) {
        if (max < townInfo->td.id) max = townInfo->td.id;
    }

    return max;
}

TownInformation* DataManager::findTownByName(const std::string& name) {
    for (const auto& townInfo : getInstance()->townInformationList) {
        if (townInfo->td.name == name) return townInfo;
    }
    return nullptr;
}

std::vector<TownInformation*> DataManager::getAllTowns() { return getInstance()->townInformationList; }

} // namespace rlx_land