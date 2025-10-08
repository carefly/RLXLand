#pragma once
#include "common/JsonLoader.h"
#include "data/land/LandCore.h"
#include "data/town/TownCore.h"
#include <string>
#include <vector>


namespace rlx_land {
// 前向声明避免循环依赖
class LandDataManager;
class TownDataManager;
} // namespace rlx_land


namespace rlx_land {

// 主模板 - 强制使用特化
template <typename T>
struct DataLoaderTraits;

// LandData 特化
template <>
struct DataLoaderTraits<LandData> {
    using DataType    = LandData;
    using InfoType    = LandInformation;
    using ManagerType = LandDataManager;

    static std::vector<DataType> loadFromFile(const std::string& filePath) {
        return JsonLoader::loadLandsFromFile(filePath);
    }

    static void saveToFile(const std::string& filePath, const std::vector<DataType>& data) {
        JsonLoader::saveLandsToFile(filePath, data);
    }

    static const std::string& getDefaultFilePath() { return JsonLoader::LANDS_JSON_PATH; }
    static const char*        getTypeName() { return "land"; }
};

// TownData 特化
template <>
struct DataLoaderTraits<TownData> {
    using DataType    = TownData;
    using InfoType    = TownInformation;
    using ManagerType = TownDataManager;

    static std::vector<DataType> loadFromFile(const std::string& filePath) {
        return JsonLoader::loadTownsFromFile(filePath);
    }

    static void saveToFile(const std::string& filePath, const std::vector<DataType>& data) {
        JsonLoader::saveTownsToFile(filePath, data);
    }

    static const std::string& getDefaultFilePath() { return JsonLoader::TOWNS_JSON_PATH; }
    static const char*        getTypeName() { return "town"; }
};


} // namespace rlx_land