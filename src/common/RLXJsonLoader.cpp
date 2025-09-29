#include "common/RLXJsonLoader.h"
#include "data/LandCore.h"
#include "data/TownCore.h"
#include "mod/RLXLand.h"
#include <fstream>
#include <ll/api/io/Logger.h>


namespace rlx_land {

const std::string RLXJsonLoader::TOWNS_JSON_PATH = "data/towns.json";
const std::string RLXJsonLoader::LANDS_JSON_PATH = "data/lands.json";


std::vector<LandData> RLXJsonLoader::loadLandsFromFile(const std::string& filePath) {
    std::vector<LandData> lands;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        rlx_land::RLXLand::getInstance().getSelf().getLogger().error("Can't open lands file {}", filePath);
        return lands;
    }

    nlohmann::json json;
    file >> json;
    file.close();

    if (json.is_array()) {
        for (const auto& item : json) {
            LandData land;
            land.x           = item.value("x", 0);
            land.z           = item.value("z", 0);
            land.dx          = item.value("dx", 0);
            land.dz          = item.value("dz", 0);
            land.d           = item.value("d", 0);
            land.perm        = item.value("perm", 0);
            land.ownerXuid   = item.value("ownerXuid", "");
            land.memberXuids = item.value("memberXuids", std::vector<std::string>());
            land.description = item.value("description", "");
            land.id          = item.value("id", 0LL);

            lands.push_back(land);
        }
    }

    return lands;
}

std::vector<TownData> RLXJsonLoader::loadTownsFromFile(const std::string& filePath) {
    std::vector<TownData> towns;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        rlx_land::RLXLand::getInstance().getSelf().getLogger().error("Can't open towns file {}", filePath);
        return towns;
    }

    nlohmann::json json;
    file >> json;
    file.close();

    if (json.is_array()) {
        for (const auto& item : json) {
            TownData town;
            town.id          = item.value("id", 0LL);
            town.name        = item.value("name", "");
            town.mayorXuid   = item.value("mayorXuid", "");
            town.memberXuids = item.value("memberXuids", std::vector<std::string>());
            town.perm        = item.value("perm", 0);
            town.x           = item.value("x", 0);
            town.z           = item.value("z", 0);
            town.dx          = item.value("dx", 0);
            town.dz          = item.value("dz", 0);
            town.d           = item.value("d", 0);
            town.description = item.value("description", "");

            towns.push_back(town);
        }
    }

    return towns;
}

void RLXJsonLoader::saveLandsToFile(const std::string& filePath, const std::vector<LandData>& lands) {
    nlohmann::json json;

    for (const auto& land : lands) {
        nlohmann::json item;
        item["x"]           = land.x;
        item["z"]           = land.z;
        item["dx"]          = land.dx;
        item["dz"]          = land.dz;
        item["d"]           = land.d;
        item["perm"]        = land.perm;
        item["ownerXuid"]   = land.ownerXuid;
        item["memberXuids"] = land.memberXuids;
        item["description"] = land.description;
        item["id"]          = land.id;

        json.push_back(item);
    }

    std::ofstream file(filePath);
    if (file.is_open()) {
        file << json.dump(4);
        file.close();
    }
}

void RLXJsonLoader::saveTownsToFile(const std::string& filePath, const std::vector<TownData>& towns) {
    nlohmann::json json;

    for (const auto& town : towns) {
        nlohmann::json item;
        item["id"]          = town.id;
        item["name"]        = town.name;
        item["mayorXuid"]   = town.mayorXuid;
        item["memberXuids"] = town.memberXuids;
        item["perm"]        = town.perm;
        item["x"]           = town.x;
        item["z"]           = town.z;
        item["dx"]          = town.dx;
        item["dz"]          = town.dz;
        item["d"]           = town.d;
        item["description"] = town.description;

        json.push_back(item);
    }

    std::ofstream file(filePath);
    if (file.is_open()) {
        file << json.dump(4);
        file.close();
    }
}

} // namespace rlx_land