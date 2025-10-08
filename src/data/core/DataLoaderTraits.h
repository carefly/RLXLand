#pragma once
#include "common/JsonLoader.h"
#include "data/land/LandCore.h"
#include "data/town/TownCore.h"
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

    static std::vector<DataType> loadFromFile() {
        // 直接使用新的按玩家加载方法
        return JsonLoader::loadLandsFromFile();
    }

    static void saveToFile(const std::vector<DataType>& data) {
        // 直接使用新的按玩家保存方法
        JsonLoader::saveLandsToFile(data);
    }

    static const char* getTypeName() { return "land"; }
};

// TownData 特化
template <>
struct DataLoaderTraits<TownData> {
    using DataType    = TownData;
    using InfoType    = TownInformation;
    using ManagerType = TownDataManager;

    static std::vector<DataType> loadFromFile() { return JsonLoader::loadTownsFromFile(); }

    static void saveToFile(const std::vector<DataType>& data) { JsonLoader::saveTownsToFile(data); }

    static const char* getTypeName() { return "town"; }
};


} // namespace rlx_land