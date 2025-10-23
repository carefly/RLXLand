#pragma once
#include "data/economy/EconomyData.h"
#include <vector>
#include <string>


namespace rlx_land {

class EconomyDataManager {
public:
    static std::vector<EconomyData> loadFromFile();
    static void saveToFile(const std::vector<EconomyData>& data);
    
private:
    static std::string getEconomyDataFile();
    static std::string getEconomyDataDir();
    static void ensureDirectoryExists(const std::string& dirPath);
};

} // namespace rlx_land