#pragma once
#include "common/JsonLoader.h"
#include "data/LandCore.h"
#include "data/TownCore.h"
#include <string>
#include <vector>


namespace rlx_land {

// 主模板 - 强制使用特化
template <typename T>
struct DataLoaderTraits;

// LandData 特化
template <>
struct DataLoaderTraits<LandData> {
    using DataType = LandData;

    static std::vector<DataType> loadFromFile(const std::string& filePath) {
        return JsonLoader::loadLandsFromFile(filePath);
    }

    static void saveToFile(const std::string& filePath, const std::vector<DataType>& data) {
        JsonLoader::saveLandsToFile(filePath, data);
    }

    static const std::string& getDefaultFilePath() { return JsonLoader::LANDS_JSON_PATH; }
};

// TownData 特化
template <>
struct DataLoaderTraits<TownData> {
    using DataType = TownData;

    static std::vector<DataType> loadFromFile(const std::string& filePath) {
        return JsonLoader::loadTownsFromFile(filePath);
    }

    static void saveToFile(const std::string& filePath, const std::vector<DataType>& data) {
        JsonLoader::saveTownsToFile(filePath, data);
    }

    static const std::string& getDefaultFilePath() { return JsonLoader::TOWNS_JSON_PATH; }
};


} // namespace rlx_land