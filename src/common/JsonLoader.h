#pragma once
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>


namespace rlx_land {

class LandData;
class TownData;

class JsonLoader {
public:
    // Town相关方法
    static std::vector<TownData> loadTownsFromFile();
    static void                  saveTownsToFile(const std::vector<TownData>& towns);

    // Lands使用新的按玩家存储方式
    static std::vector<LandData> loadLandsFromFile();
    static void                  saveLandsToFile(const std::vector<LandData>& lands);

private:
    // 路径管理
    static std::string getLandsBaseDir();
    static std::string getTownsBaseDir();
    static std::string getTownsJsonFile();

    static std::string generatePlayerFileName(const std::string& xuid, const std::string& playerName);
    static std::pair<std::string, std::string> parseFileName(const std::string& fileName);

    // 文件操作
    static void                     ensureDirectoryExists(const std::string& dirPath);
    static void                     renamePlayerFileIfNeeded(const std::string& xuid, const std::string& newName);
    static void                     deleteOldPlayerFile(const std::string& xuid);
    static void                     deletePlayerFile(const std::string& xuid);
    static std::vector<std::string> scanPlayerFiles();

    // 数据处理
    static std::map<std::string, std::vector<LandData>> groupLandsByOwner(const std::vector<LandData>& lands);
    static std::string                                  getPlayerNameByXuid(const std::string& xuid);

    // 差异检测相关方法
    static std::map<std::string, std::vector<LandData>> loadExistingPlayerData();
    static bool compareLandData(const std::vector<LandData>& oldData, const std::vector<LandData>& newData);
    static bool needsFileRename(const std::string& xuid, const std::string& currentName);
};

} // namespace rlx_land