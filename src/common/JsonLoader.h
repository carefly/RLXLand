#pragma once
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>


namespace rlx_town {
class TownData;
}

namespace rlx_land {

class LandData;

class JsonLoader {
public:
    // Town相关方法
    static std::vector<rlx_town::TownData> loadTownsFromFile();
    static void                            saveTownsToFile(const std::vector<rlx_town::TownData>& towns);

    // Lands使用新的按玩家存储方式
    static std::vector<LandData> loadLandsFromFile();
    static void                  saveLandsToFile(const std::vector<LandData>& lands);

    // 从文件名获取玩家名称（用于初始化时的 fallback）
    static std::string getPlayerNameFromFileName(const std::string& xuid);

    // 检查并更新玩家文件名（在玩家登录时调用）
    static void checkAndUpdatePlayerFileName(const std::string& xuid, const std::string& currentPlayerName);

    // 统一的玩家名称获取方法（带 fallback 机制）
    static std::string getPlayerNameWithFallback(const std::string& xuid);

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

public:
    // 玩家文件管理辅助方法（用于测试）
    static bool hasPlayerFile(const std::string& xuid);
    static void createEmptyPlayerFile(const std::string& xuid, const std::string& playerName);

private:
};

} // namespace rlx_land