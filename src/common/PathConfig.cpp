#include "PathConfig.h"
#include "mod/RLXLand.h"
#include <filesystem>


namespace rlx_land {

const std::string PathConfig::BASE_DATA_DIR     = "../RLXModeResources/data";
const std::string PathConfig::LANDS_BASE_DIR    = "../RLXModeResources/data/lands";
const std::string PathConfig::TOWNS_BASE_DIR    = "../RLXModeResources/data/towns";
const std::string PathConfig::TOWNS_JSON_FILE   = "../RLXModeResources/data/towns/towns.json";
const std::string PathConfig::ECONOMY_BASE_DIR  = "../RLXModeResources/data/economy";
const std::string PathConfig::ECONOMY_JSON_FILE = "../RLXModeResources/data/economy/economy.json";

void PathConfig::ensureDataDirectoriesExist() {
    try {
        // 确保基础数据目录存在
        std::filesystem::create_directories(BASE_DATA_DIR);

        // 确保各子目录存在
        std::filesystem::create_directories(LANDS_BASE_DIR);
        std::filesystem::create_directories(TOWNS_BASE_DIR);
        std::filesystem::create_directories(ECONOMY_BASE_DIR);
    } catch (const std::exception& e) {
        rlx_land::RLXLand::getInstance().getSelf().getLogger().error("Failed to create data directories: {}", e.what());
        throw;
    }
}

} // namespace rlx_land