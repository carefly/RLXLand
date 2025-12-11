#include "common/JsonLoader.h"
#include "common/LeviLaminaAPI.h"
#include "common/PathConfig.h"
#include "common/Utf8Utils.h"
#include "data/land/LandCore.h"
#include "data/town/TownCore.h"
#include "mod/RLXLand.h"
#include <filesystem>
#include <fstream>
#include <ranges>


namespace rlx_land {


// 路径管理方法

std::string JsonLoader::getLandsBaseDir() { return PathConfig::LANDS_BASE_DIR; }

std::string JsonLoader::getTownsBaseDir() { return PathConfig::TOWNS_BASE_DIR; }

std::string JsonLoader::getTownsJsonFile() { return PathConfig::TOWNS_JSON_FILE; }


std::string JsonLoader::generatePlayerFileName(const std::string& xuid, const std::string& playerName) {
    return std::format("{}-{}.json", xuid, playerName);
}

std::pair<std::string, std::string> JsonLoader::parseFileName(const std::string& fileName) {
    // 从 "xuid-playerName.json" 解析出 xuid 和 playerName
    size_t dashPos = fileName.find('-');
    size_t dotPos  = fileName.find(".json");

    if (dashPos != std::string::npos && dotPos != std::string::npos && dotPos > dashPos) {
        std::string xuid       = fileName.substr(0, dashPos);
        std::string playerName = fileName.substr(dashPos + 1, dotPos - dashPos - 1);
        return {xuid, playerName};
    }
    return {"", ""};
}

// 文件操作方法
void JsonLoader::ensureDirectoryExists(const std::string& dirPath) {
    try {
        std::filesystem::create_directories(dirPath);
    } catch (const std::exception& e) {
        rlx_land::RLXLand::getInstance().getSelf().getLogger().error(
            "Failed to create directory {}: {}",
            dirPath,
            e.what()
        );
        throw;
    }
}

std::vector<std::string> JsonLoader::scanPlayerFiles() {
    std::vector<std::string> playerFiles;
    std::string              landsDir = getLandsBaseDir();

    try {
        if (std::filesystem::exists(landsDir)) {
            for (const auto& entry : std::filesystem::directory_iterator(landsDir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    playerFiles.push_back(entry.path().filename().string());
                }
            }
        }
    } catch (const std::exception& e) {
        rlx_land::RLXLand::getInstance().getSelf().getLogger().error("Failed to scan player files: {}", e.what());
    }

    return playerFiles;
}

void JsonLoader::deleteOldPlayerFile(const std::string& xuid) {
    std::string landsDir    = getLandsBaseDir();
    auto        playerFiles = scanPlayerFiles();

    auto it = std::ranges::find_if(playerFiles, [&xuid](const std::string& file) {
        auto [parsedXuid, oldName] = parseFileName(file);
        return parsedXuid == xuid;
    });

    if (it != playerFiles.end()) {
        const auto&           file    = *it;
        std::filesystem::path oldPath = std::filesystem::path(landsDir) / file;
        try {
            std::filesystem::remove(oldPath);
        } catch (const std::exception& e) {
            rlx_land::RLXLand::getInstance().getSelf().getLogger().error(
                "Failed to delete old player file {}: {}",
                file,
                e.what()
            );
        }
    }
}

void JsonLoader::deletePlayerFile(const std::string& xuid) { deleteOldPlayerFile(xuid); }

// 数据处理方法
std::map<std::string, std::vector<LandData>> JsonLoader::groupLandsByOwner(const std::vector<LandData>& lands) {
    std::map<std::string, std::vector<LandData>> grouped;

    for (const auto& land : lands) {
        grouped[land.ownerXuid].push_back(land);
    }

    return grouped;
}

std::string JsonLoader::getPlayerNameByXuid(const std::string& xuid) {
    std::string playerName = LeviLaminaAPI::getPlayerNameByXuid(xuid);
    if (playerName.empty()) {
        playerName = "Unknown";
    }
    return playerName;
}

std::vector<LandData> JsonLoader::loadLandsFromFile() {
    // 直接使用新的按玩家文件加载方式
    std::vector<LandData> allLands;
    std::string           landsDir = getLandsBaseDir();

    // 确保目录存在
    ensureDirectoryExists(landsDir);

    // 扫描所有玩家文件并加载
    auto playerFiles = scanPlayerFiles();

    for (const auto& file : playerFiles) {
        std::filesystem::path fullPath = std::filesystem::path(landsDir) / file;
        try {
            std::ifstream fileStream = Utf8Utils::createUtf8InputStream(fullPath.string());
            if (!fileStream.is_open()) {
                rlx_land::RLXLand::getInstance().getSelf().getLogger().error(
                    "Can't open player lands file {}",
                    fullPath
                );
                continue;
            }

            nlohmann::json json;
            fileStream >> json;
            fileStream.close();

            if (json.is_array()) {
                for (const auto& item : json) {
                    LandData land;
                    land.x           = item.value("x", 0);
                    land.z           = item.value("z", 0);
                    land.x_end       = item.value("x_end", 0); // 直接使用新字段名
                    land.z_end       = item.value("z_end", 0); // 直接使用新字段名
                    land.d           = item.value("d", 0);
                    land.perm        = item.value("perm", 0);
                    land.ownerXuid   = item.value("ownerXuid", "");
                    land.memberXuids = item.value("memberXuids", std::vector<std::string>());
                    land.description = item.value("description", "");
                    land.id          = item.value("id", 0LL);

                    allLands.push_back(land);
                }
            }
        } catch (const std::exception& e) {
            // 记录错误但继续处理其他文件
            rlx_land::RLXLand::getInstance().getSelf().getLogger().error(
                "Failed to load player file {}: {}",
                file,
                e.what()
            );
        }
    }

    return allLands;
}

std::vector<rlx_town::TownData> JsonLoader::loadTownsFromFile() {
    using rlx_town::TownData;

    std::vector<TownData> towns;
    std::string           townsPath = getTownsJsonFile();

    // 确保towns目录存在
    std::string townsDir = getTownsBaseDir();
    ensureDirectoryExists(townsDir);

    // 如果文件不存在则创建一个空文件，避免启动时报错
    try {
        if (!std::filesystem::exists(townsPath)) {
            nlohmann::json emptyJson = nlohmann::json::array();
            std::ofstream  initFile  = Utf8Utils::createUtf8OutputStream(townsPath);
            if (initFile.is_open()) {
                initFile << emptyJson.dump(4);
                initFile.close();
                rlx_land::RLXLand::getInstance().getSelf().getLogger().info(
                    "Towns file not found, created default file at {}",
                    townsPath
                );
            } else {
                rlx_land::RLXLand::getInstance().getSelf().getLogger().error(
                    "Failed to create towns file {}",
                    townsPath
                );
                return towns;
            }
        }
    } catch (const std::exception& e) {
        rlx_land::RLXLand::getInstance().getSelf().getLogger().error(
            "Failed to initialize towns file {}: {}",
            townsPath,
            e.what()
        );
        return towns;
    }

    std::ifstream file = Utf8Utils::createUtf8InputStream(townsPath);
    if (!file.is_open()) {
        rlx_land::RLXLand::getInstance().getSelf().getLogger().error("Can't open towns file {}", townsPath);
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
            town.x_end       = item.value("x_end", 0); // 直接使用新字段名
            town.z_end       = item.value("z_end", 0); // 直接使用新字段名
            town.d           = item.value("d", 0);
            town.description = item.value("description", "");

            towns.push_back(town);
        }
    }

    return towns;
}

void JsonLoader::saveLandsToFile(const std::vector<LandData>& lands) {
    // 直接使用新的按玩家文件保存方式
    std::string landsDir = getLandsBaseDir();
    ensureDirectoryExists(landsDir);

    // 1. 按owner分组新数据
    auto newGroupedLands = groupLandsByOwner(lands);

    // 2. 加载现有数据用于对比
    auto existingGroupedLands = loadExistingPlayerData();

    // 3. 处理每个玩家的数据
    for (const auto& [ownerXuid, newPlayerLands] : newGroupedLands) {
        std::string playerName = getPlayerNameByXuid(ownerXuid);

        // 检查是否需要重命名文件
        bool needsRename = needsFileRename(ownerXuid, playerName);

        // 检查数据是否有变化
        bool dataChanged = true;
        auto existingIt  = existingGroupedLands.find(ownerXuid);
        if (existingIt != existingGroupedLands.end()) {
            dataChanged = compareLandData(existingIt->second, newPlayerLands);
        }

        // 只有在数据变化或需要重命名时才保存
        if (dataChanged || needsRename) {
            // 如果需要重命名，先删除旧文件
            if (needsRename) {
                deleteOldPlayerFile(ownerXuid);
            }

            std::string           fileName = generatePlayerFileName(ownerXuid, playerName);
            std::filesystem::path fullPath = std::filesystem::path(landsDir) / fileName;

            // 保存单个玩家的数据到文件
            nlohmann::json json;
            for (const auto& land : newPlayerLands) {
                nlohmann::json item;
                item["x"]           = land.x;
                item["z"]           = land.z;
                item["x_end"]       = land.x_end; // 只保存新字段名
                item["z_end"]       = land.z_end; // 只保存新字段名
                item["d"]           = land.d;
                item["perm"]        = land.perm;
                item["ownerXuid"]   = land.ownerXuid;
                item["memberXuids"] = land.memberXuids;
                item["description"] = land.description;
                item["id"]          = land.id;
                json.push_back(item);
            }

            std::ofstream file = Utf8Utils::createUtf8OutputStream(fullPath.string());
            if (file.is_open()) {
                file << json.dump(4);
                file.close();
            }
        }
    }

    // 4. 处理需要删除的玩家文件（玩家不再有任何lands）
    for (const auto& [ownerXuid, oldPlayerLands] : existingGroupedLands) {
        if (newGroupedLands.find(ownerXuid) == newGroupedLands.end()) {
            deletePlayerFile(ownerXuid);
        }
    }
}

void JsonLoader::saveTownsToFile(const std::vector<rlx_town::TownData>& towns) {
    using rlx_town::TownData;
    nlohmann::json json;
    std::string    townsPath = getTownsJsonFile();
    std::string    townsDir  = getTownsBaseDir();

    // 确保towns目录存在
    ensureDirectoryExists(townsDir);

    for (const auto& town : towns) {
        nlohmann::json item;
        item["id"]          = town.id;
        item["name"]        = town.name;
        item["mayorXuid"]   = town.mayorXuid;
        item["memberXuids"] = town.memberXuids;
        item["perm"]        = town.perm;
        item["x"]           = town.x;
        item["z"]           = town.z;
        item["x_end"]       = town.x_end; // 只保存新字段名
        item["z_end"]       = town.z_end; // 只保存新字段名
        item["d"]           = town.d;
        item["description"] = town.description;

        json.push_back(item);
    }

    // 保存到统一文件
    std::ofstream file = Utf8Utils::createUtf8OutputStream(townsPath);
    if (file.is_open()) {
        file << json.dump(4);
        file.close();
    }
}


// 差异检测相关方法
std::map<std::string, std::vector<LandData>> JsonLoader::loadExistingPlayerData() {
    std::map<std::string, std::vector<LandData>> existingData;
    std::string                                  landsDir = getLandsBaseDir();

    auto playerFiles = scanPlayerFiles();

    for (const auto& file : playerFiles) {
        auto [xuid, playerName] = parseFileName(file);
        if (!xuid.empty()) {
            std::filesystem::path fullPath = std::filesystem::path(landsDir) / file;
            try {
                // 直接从文件加载单个玩家的数据
                std::vector<LandData> playerLands;
                std::ifstream         fileStream = Utf8Utils::createUtf8InputStream(fullPath.string());
                if (fileStream.is_open()) {
                    nlohmann::json json;
                    fileStream >> json;
                    fileStream.close();

                    if (json.is_array()) {
                        for (const auto& item : json) {
                            LandData land;
                            land.x           = item.value("x", 0);
                            land.z           = item.value("z", 0);
                            land.x_end       = item.value("x_end", 0); // 直接使用新字段名
                            land.z_end       = item.value("z_end", 0); // 直接使用新字段名
                            land.d           = item.value("d", 0);
                            land.perm        = item.value("perm", 0);
                            land.ownerXuid   = item.value("ownerXuid", "");
                            land.memberXuids = item.value("memberXuids", std::vector<std::string>());
                            land.description = item.value("description", "");
                            land.id          = item.value("id", 0LL);
                            playerLands.push_back(land);
                        }
                    }
                }
                existingData[xuid] = playerLands;
            } catch (const std::exception& e) {
                rlx_land::RLXLand::getInstance().getSelf().getLogger().error(
                    "Failed to load existing player file {}: {}",
                    file,
                    e.what()
                );
            }
        }
    }

    return existingData;
}

bool JsonLoader::compareLandData(const std::vector<LandData>& oldData, const std::vector<LandData>& newData) {
    // 1. 快速检查数量
    if (oldData.size() != newData.size()) {
        return true; // 有变化
    }

    // 2. 创建ID到数据的映射
    std::map<LONG64, LandData> oldMap, newMap;
    for (const auto& land : oldData) {
        oldMap[land.id] = land;
    }
    for (const auto& land : newData) {
        newMap[land.id] = land;
    }

    // 3. 比较每个land的详细信息
    for (const auto& [id, newLand] : newMap) {
        auto oldIt = oldMap.find(id);
        if (oldIt == oldMap.end()) {
            return true; // 新增的land
        }

        const auto& oldLand = oldIt->second;

        // 比较关键字段
        if (oldLand.x != newLand.x || oldLand.z != newLand.z || oldLand.x_end != newLand.x_end
            || oldLand.z_end != newLand.z_end || oldLand.d != newLand.d || oldLand.perm != newLand.perm
            || oldLand.ownerXuid != newLand.ownerXuid || oldLand.memberXuids != newLand.memberXuids
            || oldLand.description != newLand.description) {
            return true; // 数据有变化
        }
    }

    return false; // 没有变化
}

bool JsonLoader::needsFileRename(const std::string& xuid, const std::string& currentName) {
    std::string landsDir    = getLandsBaseDir();
    auto        playerFiles = scanPlayerFiles();

    return std::ranges::any_of(playerFiles, [&xuid, &currentName](const std::string& file) {
        auto [parsedXuid, oldName] = parseFileName(file);
        return parsedXuid == xuid && oldName != currentName;
    });
}

void JsonLoader::renamePlayerFileIfNeeded(const std::string& xuid, const std::string& newName) {
    if (!needsFileRename(xuid, newName)) {
        return; // 不需要重命名
    }

    std::string landsDir    = getLandsBaseDir();
    auto        playerFiles = scanPlayerFiles();

    auto it = std::ranges::find_if(playerFiles, [&xuid, &newName](const std::string& file) {
        auto [parsedXuid, oldName] = parseFileName(file);
        return parsedXuid == xuid && oldName != newName;
    });

    if (it != playerFiles.end()) {
        const auto& file              = *it;
        auto [parsedXuid, oldName]    = parseFileName(file);
        std::filesystem::path oldPath = std::filesystem::path(landsDir) / file;
        std::filesystem::path newPath = std::filesystem::path(landsDir) / generatePlayerFileName(xuid, newName);

        try {
            std::filesystem::rename(oldPath, newPath);
            rlx_land::RLXLand::getInstance().getSelf().getLogger().info(
                "Renamed player file from {} to {}",
                file,
                generatePlayerFileName(xuid, newName)
            );
        } catch (const std::exception& e) {
            rlx_land::RLXLand::getInstance().getSelf().getLogger().error(
                "Failed to rename player file from {} to {}: {}",
                oldPath,
                newPath,
                e.what()
            );
        }
    }
}


} // namespace rlx_land