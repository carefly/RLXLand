#include "TownCore.h"
#include "common/LeviLaminaAPI.h"
#include <algorithm>
#include <string>

namespace rlx_land {

TownInformation::TownInformation(TownData td) { this->td = std::move(td); }

bool TownInformation::isMayor(const std::string& xuid) const { return this->td.mayorXuid == xuid; }

bool TownInformation::hasBasicPermission(const std::string& xuid) const {
    // 检查玩家是否为镇长
    if (isMayor(xuid)) return true;

    // 检查玩家是否为成员
    return std::ranges::any_of(this->td.memberXuids, [&xuid](const std::string& member) { return member == xuid; });
}

// 获取成员名称,用逗号分隔
std::string TownInformation::getMembers() const {
    std::string memberNames;

    for (size_t i = 0; i < this->td.memberXuids.size(); ++i) {
        if (i > 0) {
            memberNames += ",";
        }
        memberNames += LeviLaminaAPI::getPlayerNameByXuid(this->td.memberXuids[i]);
    }

    return memberNames;
}

SmallTownMap::SmallTownMap(int x, int z, int d) {
    this->x         = x;
    this->z         = z;
    this->d         = d;
    this->size      = 100;
    this->middleMap = nullptr;
    this->bigMap    = nullptr;

    // 为100x100的区域创建存储空间，每个格子存储一个Town
    this->map = new TownInformation*[this->size * this->size];
    for (int i = 0; i < this->size * this->size; i++) map[i] = NULL;
}

void SmallTownMap::setTown(int sx, int sz, TownInformation* ti) {
    // 将Town添加到指定位置
    this->map[sx + sz * this->size] = ti;
}

TownInformation* SmallTownMap::getTown(int sx, int sz) {
    // 返回指定位置的Town
    return this->map[sx + sz * this->size];
}

MiddleTownMap::MiddleTownMap(int x, int z, int d) {
    this->x      = x;
    this->z      = z;
    this->d      = d;
    this->size   = 100;
    this->bigMap = nullptr;

    this->map = new SmallTownMap*[this->size * this->size];
    for (int i = 0; i < this->size * this->size; i++) map[i] = NULL;
}

MiddleTownMap::~MiddleTownMap() {
    // 清理所有SmallTownMap
    for (int i = 0; i < this->size * this->size; i++) {
        if (map[i] != NULL) {
            delete map[i];
        }
    }
    delete[] this->map;
}

void MiddleTownMap::setMap(int sx, int sz, SmallTownMap* sm) { this->map[sx + sz * this->size] = sm; }

SmallTownMap* MiddleTownMap::getMap(int sx, int sz) { return this->map[sx + sz * this->size]; }

BigTownMap::BigTownMap(int x, int z, int d) {
    this->x    = x;
    this->z    = z;
    this->d    = d;
    this->size = 10;

    this->map = new MiddleTownMap*[this->size * this->size];
    for (int i = 0; i < this->size * this->size; i++) map[i] = NULL;
}

BigTownMap::~BigTownMap() {
    // 清理所有MiddleTownMap
    for (int i = 0; i < this->size * this->size; i++) {
        if (map[i] != nullptr) {
            delete map[i];
        }
    }
    delete[] this->map;
}

void BigTownMap::setMap(int mx, int mz, MiddleTownMap* m) { this->map[mx + mz * this->size] = m; }

MiddleTownMap* BigTownMap::getMap(int mx, int mz) { return this->map[mx + mz * this->size]; }


std::shared_ptr<TownMap> TownMap::getInstance() {

    static std::shared_ptr<TownMap> instance = std::make_shared<TownMap>();
    return instance;
}

TownInformation* TownMap::find(LONG64 coordx, LONG64 coordz, int d) {
    int    bigx, bigz, middlex, middlez, smallx, smallz;
    LONG64 x = coordx;
    LONG64 z = coordz;

    // 计算在BigTownMap中的索引
    bigx = (int)((coordx + TOWN_BIG_SIZE * 10) / TOWN_BIG_SIZE);
    bigz = (int)((coordz + TOWN_BIG_SIZE * 10) / TOWN_BIG_SIZE);

    x = abs(x) - abs(coordx / TOWN_BIG_SIZE * TOWN_BIG_SIZE);
    z = abs(z) - abs(coordz / TOWN_BIG_SIZE * TOWN_BIG_SIZE);

    // 检查索引是否在有效范围内
    if (bigx >= 20 || bigx < 0 || bigz >= 20 || bigz < 0) return nullptr; // 返回空指针

    // 获取BigTownMap
    BigTownMap* bigMap = TownMap::map[bigx][bigz][d];
    if (bigMap == nullptr) return nullptr; // 返回空指针

    // 计算在MiddleTownMap中的索引
    middlex = (int)(x / TOWN_MIDDLE_SIZE);
    middlez = (int)(z / TOWN_MIDDLE_SIZE);

    x = x - middlex * TOWN_MIDDLE_SIZE;
    z = z - middlez * TOWN_MIDDLE_SIZE;

    // 获取MiddleTownMap
    MiddleTownMap* middleMap = bigMap->getMap(middlex, middlez);
    if (middleMap == nullptr) return nullptr; // 返回空指针

    // 计算在SmallTownMap中的索引
    smallx = (int)(x / TOWN_SMALL_SIZE);
    smallz = (int)(z / TOWN_SMALL_SIZE);

    x = x - smallx * TOWN_SMALL_SIZE;
    z = z - smallz * TOWN_SMALL_SIZE;

    // 获取SmallTownMap
    SmallTownMap* smallMap = middleMap->getMap(smallx, smallz);
    if (smallMap == nullptr) return nullptr; // 返回空指针

    // 返回指定位置的Town
    return smallMap->getTown((int)x, (int)z);
}


void TownMap::set(TownInformation* ti, LONG64 xi, LONG64 zi, int d) {
    int    bigx, bigz, middlex, middlez, smallx, smallz;
    LONG64 x = xi, z = zi;

    // 计算在BigTownMap中的索引
    bigx = (int)((xi + TOWN_BIG_SIZE * 10) / TOWN_BIG_SIZE);
    bigz = (int)((zi + TOWN_BIG_SIZE * 10) / TOWN_BIG_SIZE);

    x = abs(x) - abs(xi / TOWN_BIG_SIZE * TOWN_BIG_SIZE);
    z = abs(z) - abs(zi / TOWN_BIG_SIZE * TOWN_BIG_SIZE);

    // 检查索引是否在有效范围内
    if (bigx >= 20 || bigx < 0 || bigz >= 20 || bigz < 0) return;

    // 获取或创建BigTownMap
    BigTownMap* bigMap = TownMap::map[bigx][bigz][d];
    if (bigMap == nullptr) {
        bigMap = new BigTownMap(bigx, bigz, d);

        TownMap::map[bigx][bigz][d] = bigMap;
    }

    // 计算在MiddleTownMap中的索引
    middlex = (int)(x / TOWN_MIDDLE_SIZE);
    middlez = (int)(z / TOWN_MIDDLE_SIZE);

    x = x - middlex * TOWN_MIDDLE_SIZE;
    z = z - middlez * TOWN_MIDDLE_SIZE;

    // 获取或创建MiddleTownMap
    MiddleTownMap* middleMap = bigMap->getMap(middlex, middlez);
    if (middleMap == nullptr) {
        middleMap = new MiddleTownMap(middlex, middlez, d);

        middleMap->bigMap = bigMap;
        bigMap->setMap(middlex, middlez, middleMap);
    }

    // 计算在SmallTownMap中的索引
    smallx = (int)(x / TOWN_SMALL_SIZE);
    smallz = (int)(z / TOWN_SMALL_SIZE);

    x = x - smallx * TOWN_SMALL_SIZE;
    z = z - smallz * TOWN_SMALL_SIZE;

    // 获取或创建SmallTownMap
    SmallTownMap* smallMap = middleMap->getMap(smallx, smallz);
    if (smallMap == NULL) {
        smallMap            = new SmallTownMap(smallx, smallz, d);
        smallMap->bigMap    = bigMap;
        smallMap->middleMap = middleMap;
        middleMap->setMap(smallx, smallz, smallMap);
    }

    // 将Town信息添加到指定位置
    smallMap->setTown((int)x, (int)z, ti);
}

} // namespace rlx_land