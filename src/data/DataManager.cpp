#include "DataManager.h"
#include "LandCore.h"
#include "SpatialMap.h"
#include "TownCore.h"
#include "common/JsonLoader.h"
#include "common/LeviLaminaAPI.h"
#include "mod/RLXLand.h"
#include <memory>

#define LAND_RANGE 1000000


namespace rlx_land {

DataManager::DataManager()
: landManager(std::make_unique<LandDataManager>()),
  townManager(std::make_unique<TownDataManager>()) {}

std::shared_ptr<DataManager> DataManager::getInstance() {
    static std::shared_ptr<DataManager> instance(new DataManager());
    return instance;
}

LONG64 DataManager::getLandMaxId() { return getInstance()->landManager->getMaxId(); }

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

        this->landManager->getAllItems().push_back(li);

        for (LONG64 xi = x1; xi <= x2; xi++)
            for (LONG64 zi = z1; zi <= z2; zi++) {
                LandMap::getInstance()->set(li, xi, zi, d);
            }
    }
}

void DataManager::createLand(LandData data) {
    landManager->create(std::move(data));

    // 更新地图
    for (LONG64 x = data.x; x <= data.dx; x++) {
        for (LONG64 z = data.z; z <= data.dz; z++) {
            LandMap::getInstance()->set(static_cast<LandInformation*>(landManager->getAllItems().back()), x, z, data.d);
        }
    }
}

void DataManager::deleteLand(LandData data) {
    landManager->remove(std::move(data));

    for (LONG64 xi = data.x; xi <= data.dx; xi++)
        for (LONG64 zi = data.z; zi <= data.dz; zi++) {
            LandMap::getInstance()->set(nullptr, xi, zi, data.d);
        }
}

void DataManager::modifyLandPerm(LandInformation* li, int perm) { landManager->modifyPerm(li, perm); }

void DataManager::addLandMember(LandInformation* li, const std::string& playerName) {
    landManager->addMember(li, playerName);
}

void DataManager::removeLandMember(LandInformation* li, const std::string& playerName) {
    landManager->removeMember(li, playerName);
}

void DataManager::loadTowns() {
    // 从JSON文件加载Town数据
    std::vector<TownData> towns = JsonLoader::loadTownsFromFile(JsonLoader::TOWNS_JSON_PATH);

    RLXLand::getInstance().getSelf().getLogger().info(std::format("load {} towns from JSON", towns.size()));


    for (const auto& town : towns) {
        auto ti = new TownInformation(town);

        ti->mayorName = LeviLaminaAPI::getPlayerNameByXuid(town.mayorXuid);

        this->townManager->getAllItems().push_back(ti);

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
    townManager->create(std::move(data));

    // 更新地图
    auto towns = townManager->getAllItems();
    if (!towns.empty()) {
        TownInformation* ti = towns.back();
        for (LONG64 x = data.x; x <= data.dx; x++) {
            for (LONG64 z = data.z; z <= data.dz; z++) {
                TownMap::getInstance()->set(ti, x, z, data.d);
            }
        }
    }
}

void DataManager::deleteTown(TownData data) {
    townManager->remove(std::move(data));

    for (LONG64 xi = data.x; xi <= data.dx; xi++)
        for (LONG64 zi = data.z; zi <= data.dz; zi++) {
            TownMap::getInstance()->set(nullptr, xi, zi, data.d);
        }
}

void DataManager::modifyTownPerm(TownInformation* ti, int perm) { townManager->modifyPerm(ti, perm); }

void DataManager::addTownMember(TownInformation* ti, const std::string& playerName) {
    townManager->addMember(ti, playerName);
}

void DataManager::removeTownMember(TownInformation* ti, const std::string& playerName) {
    townManager->removeMember(ti, playerName);
}

void DataManager::transferTownMayor(TownInformation* ti, const std::string& playerName) {
    townManager->transferMayor(ti, playerName);
}

LONG64 DataManager::getTownMaxId() { return getInstance()->townManager->getMaxId(); }

TownInformation* DataManager::findTownByName(const std::string& name) {
    auto towns = getInstance()->townManager->getAllItems();
    for (const auto& townInfo : towns) {
        if (townInfo->td.name == name) return townInfo;
    }
    return nullptr;
}

std::vector<TownInformation*> DataManager::getAllTowns() { return getInstance()->townManager->getAllItems(); }

} // namespace rlx_land