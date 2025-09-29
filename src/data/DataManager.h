#pragma once
#include "LandCore.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


namespace rlx_land {

class DataManager {
public:
    static std::shared_ptr<DataManager> getInstance();


    void loadLands();
    void createLand(LandData data);
    void deleteLand(LandData data);
    void modifyLandPerm(LandInformation* li, int perm);
    void addLandMember(LandInformation* li, const std::string& playerName);
    void removeLandMember(LandInformation* li, const std::string& playerName);

    static LONG64 getLandMaxId();

private:
    std::unordered_map<std::string, std::string> getLandMember(std::vector<std::string> xuids);

    std::vector<LandInformation*> landInformationList;
};

} // namespace rlx_land