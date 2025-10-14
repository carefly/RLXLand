#pragma once

#include <map>
#include <string>


// Mock Player类，模拟LeviLamina的Player类
class Player {
private:
    std::string mXuid;
    std::string mName;
    bool        mIsOperator;

public:
    Player(const std::string& xuid, const std::string& name, bool isOp = false)
    : mXuid(xuid),
      mName(name),
      mIsOperator(isOp) {}

    const std::string& getXuid() const { return mXuid; }
    const std::string& getName() const { return mName; }
    bool               isOperator() const { return mIsOperator; }
    void               setOperator(bool isOp) { mIsOperator = isOp; }
};

namespace rlx_land {

class LeviLaminaAPI {
private:
    static std::map<std::string, Player*>     mockPlayers;
    static std::map<std::string, std::string> xuidToName;
    static std::map<std::string, std::string> nameToXuid;

public:
    static Player*     getPlayerByXuid(const std::string& xuid);
    static Player*     getPlayerByName(const std::string& name);
    static std::string getPlayerNameByXuid(const std::string& xuid);
    static std::string getXuidByPlayerName(const std::string& name);

    // 测试辅助方法
    static void addMockPlayer(const std::string& xuid, const std::string& name, bool isOp = false);
    static void removeMockPlayer(const std::string& xuid);
    static void clearMockPlayers();
};

} // namespace rlx_land