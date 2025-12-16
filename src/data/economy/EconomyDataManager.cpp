#include "EconomyDataManager.h"
#include "common/Utf8Utils.h"
#include "mod/RLXLand.h"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>


namespace rlx_land {

std::string EconomyDataManager::getEconomyDataDir() { return "plugins/RLXModeResources/data/economy"; }

std::string EconomyDataManager::getEconomyDataFile() { return "plugins/RLXModeResources/data/economy/economy.json"; }

void EconomyDataManager::ensureDirectoryExists(const std::string& dirPath) {
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

std::vector<EconomyData> EconomyDataManager::loadFromFile() {
    std::vector<EconomyData> economyData;
    std::string              dataPath = getEconomyDataFile();

    // 确保目录存在
    std::string dirPath = getEconomyDataDir();
    ensureDirectoryExists(dirPath);

    std::ifstream file = Utf8Utils::createUtf8InputStream(dataPath);
    if (!file.is_open()) {
        // 文件不存在，返回空数据
        return economyData;
    }

    try {
        nlohmann::json json;
        file >> json;
        file.close();

        if (json.is_array()) {
            for (const auto& item : json) {
                EconomyData data = EconomyData::fromJson(item);
                economyData.push_back(data);
            }
        }
    } catch (const std::exception& e) {
        rlx_land::RLXLand::getInstance().getSelf().getLogger().error(
            "Failed to load economy data from {}: {}",
            dataPath,
            e.what()
        );

        // 返回已成功加载的数据
        return economyData;
    }

    return economyData;
}

void EconomyDataManager::saveToFile(const std::vector<EconomyData>& data) {
    std::string dataPath = getEconomyDataFile();

    // 确保目录存在
    std::string dirPath = getEconomyDataDir();
    ensureDirectoryExists(dirPath);

    nlohmann::json json;
    for (const auto& item : data) {
        json.push_back(item.toJson());
    }

    std::ofstream file = Utf8Utils::createUtf8OutputStream(dataPath);
    if (file.is_open()) {
        file << json.dump(4);
        file.close();
    } else {
        rlx_land::RLXLand::getInstance().getSelf().getLogger().error("Failed to save economy data to {}", dataPath);
    }
}

} // namespace rlx_land