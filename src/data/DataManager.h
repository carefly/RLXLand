#pragma once
#include <memory>

#include "LandCore.h"

using namespace std;

class DataManager {
public:
    static shared_ptr<DataManager> getInstance();

    void loadLands();
    void createLand(LandData data);
    void deleteLand(LandData data);
    void modifyLandPerm(LandInformation* li, int perm);
    void addLandMember(LandInformation* li, const string& playerName);
    void removeLandMember(LandInformation* li, const string& playerName);

    LONG64 getLandMaxId();

private:
    DataManager();

    unordered_map<string, string> getLandMember(string line);

    std::vector<LandInformation*> landInformationList;
};