#pragma once

#include <basetsd.h>
#include <memory>
#include <string>
#include <vector>

namespace rlx_land {

#define LAND_BIG_SIZE    100000
#define LAND_MIDDLE_SIZE 10000
#define LAND_SMALL_SIZE  100
#define LAND_RANGE       1000000

class BigLandMap;
class MiddleLandMap;
class SmallLandMap;

class LandData {
public:
    int                      x, z, dx, dz, d, perm;
    LONG64                   id;
    std::string              ownerXuid, description;
    std::vector<std::string> memberXuids;
};

class LandInformation {
public:
    explicit LandInformation(LandData ld);
    LandData ld;

    std::string ownerName;

    [[nodiscard]] bool hasBasicPermission(const std::string& xuid) const;
    [[nodiscard]] bool isOwner(const std::string& xuid) const;

    std::string getMembers();
};

class SmallLandMap {
public:
    int x, z;
    int d;
    int size;

    BigLandMap*    bigMap;
    MiddleLandMap* middleMap;

    SmallLandMap(int x, int z, int d);

    void             setLand(int x, int z, LandInformation* li);
    LandInformation* getLand(int x, int z);

private:
    LandInformation** map;
};

class MiddleLandMap {
public:
    int x, z;
    int d;
    int size;

    // SmallLandMap *map[100][100];

    BigLandMap* bigMap;

    MiddleLandMap(int x, int z, int d);
    ~MiddleLandMap();

    void          setMap(int x, int z, SmallLandMap* m);
    SmallLandMap* getMap(int x, int z);

private:
    SmallLandMap** map;
};

class BigLandMap {
public:
    int x;
    int z;
    int d;
    int size;

    // MiddleLandMap *map[10][10];

    BigLandMap(int x, int z, int d);
    ~BigLandMap();

    void           setMap(int x, int z, MiddleLandMap* m);
    MiddleLandMap* getMap(int x, int z);

private:
    MiddleLandMap** map;
};

class LandMap {
public:
    static std::shared_ptr<LandMap> getInstance();

    BigLandMap* map[20][20][3];

    LandInformation* find(LONG64 coordx, LONG64 coordy, int d);

    void set(LandInformation* li, LONG64 xi, LONG64 zi, int d);
};

} // namespace rlx_land