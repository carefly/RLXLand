#pragma once

#include <basetsd.h>
#include <mc/deps/core/math/Vec3.h>
#include <mc/world/actor/player/Player.h>
#include <string>
#include <unordered_map>

using namespace std;

#define LAND_BIG_SIZE    100000
#define LAND_MIDDLE_SIZE 10000
#define LAND_SMALL_SIZE  100
#define LAND_RANGE       1000000

class BigLandMap;
class MiddleLandMap;
class SmallLandMap;

class LandData {
public:
    int    x, z, dx, dz, d, perm;
    LONG64 id;
    string ownerXuid, memberXuids, description;
};

class LandInformation {
public:
    explicit LandInformation(LandData ld);
    LandData ld;

    string                        ownerName;
    unordered_map<string, string> members;

    bool   hasPerm(Player* p);
    bool   isOwner(string xuid);
    string getMembers();
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
    static shared_ptr<LandMap> getInstance();

    BigLandMap* map[20][20][3];

    LandInformation* find(LONG64 coordx, LONG64 coordy, int d);

    void set(LandInformation* li, LONG64 xi, LONG64 zi, int d);
};