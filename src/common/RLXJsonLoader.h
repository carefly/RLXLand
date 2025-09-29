#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <vector>


namespace rlx_land {

class LandData;
class TownData;

class RLXJsonLoader {
public:
    static const std::string TOWNS_JSON_PATH;
    static const std::string LANDS_JSON_PATH;

    static std::vector<LandData> loadLandsFromFile(const std::string& filePath);
    static std::vector<TownData> loadTownsFromFile(const std::string& filePath);

    static void saveLandsToFile(const std::string& filePath, const std::vector<LandData>& lands);
    static void saveTownsToFile(const std::string& filePath, const std::vector<TownData>& towns);
};

} // namespace rlx_land