#pragma once
#include "LandCore.h"
#include "TownCore.h"
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

    void loadTowns();
    void createTown(TownData data);
    void deleteTown(TownData data);
    void modifyTownPerm(TownInformation* ti, int perm);
    void addTownMember(TownInformation* ti, const std::string& playerName);
    void removeTownMember(TownInformation* ti, const std::string& playerName);
    void transferTownMayor(TownInformation* ti, const std::string& playerName);

    static LONG64 getTownMaxId();

    TownInformation* findTownByName(const std::string& name);

    std::vector<TownInformation*> getAllTowns();

private:
    std::unordered_map<std::string, std::string> getLandMember(std::vector<std::string> xuids);

    std::vector<LandInformation*> landInformationList;
    std::vector<TownInformation*> townInformationList;
};

} // namespace rlx_land