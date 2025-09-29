#pragma once

#include <basetsd.h>
#include <memory>
#include <string>
#include <vector>

namespace rlx_land {

class BigTownMap;
class MiddleTownMap;
class SmallTownMap;

class TownData {
public:
    long long                id;
    std::string              name;
    std::string              mayorXuid;
    std::vector<std::string> memberXuids;
    int                      perm;
    int                      x, z, dx, dz, d;
    std::string              description;
};

class TownInformation {
public:
    explicit TownInformation(TownData td);
    TownData td;

    std::string mayorName;

    [[nodiscard]] bool hasBasicPermission(const std::string& xuid) const;
    [[nodiscard]] bool isMayor(const std::string& xuid) const;

    [[nodiscard]] std::string getMembers() const;
};

#define TOWN_BIG_SIZE    100000
#define TOWN_MIDDLE_SIZE 10000
#define TOWN_SMALL_SIZE  100

class SmallTownMap {
public:
    int x, z;
    int d;
    int size;

    BigTownMap*    bigMap;
    MiddleTownMap* middleMap;

    SmallTownMap(int x, int z, int d);

    void             setTown(int x, int z, TownInformation* ti);
    TownInformation* getTown(int x, int z);

private:
    TownInformation** map;
};

class MiddleTownMap {
public:
    int x, z;
    int d;
    int size;

    BigTownMap* bigMap;

    MiddleTownMap(int x, int z, int d);
    ~MiddleTownMap();

    void          setMap(int x, int z, SmallTownMap* m);
    SmallTownMap* getMap(int x, int z);

private:
    SmallTownMap** map;
};

class BigTownMap {
public:
    int x;
    int z;
    int d;
    int size;

    BigTownMap(int x, int z, int d);
    ~BigTownMap();

    void           setMap(int x, int z, MiddleTownMap* m);
    MiddleTownMap* getMap(int x, int z);

private:
    MiddleTownMap** map;
};

class TownMap {
public:
    static std::shared_ptr<TownMap> getInstance();

    BigTownMap* map[20][20][3];

    TownInformation* find(LONG64 coordx, LONG64 coordy, int d);
    void             set(TownInformation* ti, LONG64 xi, LONG64 zi, int d);
};

} // namespace rlx_land