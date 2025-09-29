#include "LandCore.h"
#include "common/LeviLaminaAPI.h"

namespace rlx_land {

LandInformation::LandInformation(LandData ld) { this->ld = std::move(ld); }

bool LandInformation::hasBasicPermission(const std::string& xuid) const {

    // 检查是否是拥有者
    if (xuid == this->ld.ownerXuid) return true;

    // 检查是否是成员
    if (find(this->ld.memberXuids.begin(), this->ld.memberXuids.end(), xuid) != this->ld.memberXuids.end()) {
        return true;
    } else return false;
}

bool LandInformation::isOwner(const std::string& xuid) const {
    if (xuid == this->ld.ownerXuid) return true;
    else return false;
}

// 获取成员名称,用逗号分隔
std::string LandInformation::getMembers() const {

    std::string memberNames;

    for (size_t i = 0; i < this->ld.memberXuids.size(); ++i) {
        if (i > 0) {
            memberNames += ",";
        }
        memberNames += LeviLaminaAPI::getPlayerNameByXuid(this->ld.memberXuids[i]);
    }

    return memberNames;
}

BigLandMap::BigLandMap(int x, int z, int d) {
    this->x    = x;
    this->z    = z;
    this->d    = d;
    this->size = 10;
    // logger.info(format("x:{} z:{} d:{}", x, z, d));
    this->map = new MiddleLandMap*[this->size * this->size];
    for (int i = 0; i < this->size * this->size; i++) map[i] = nullptr;
}

BigLandMap::~BigLandMap() { delete map; }

void BigLandMap::setMap(int mx, int mz, MiddleLandMap* m) { this->map[mx + mz * this->size] = m; }

MiddleLandMap* BigLandMap::getMap(int mx, int mz) { return this->map[mx + mz * this->size]; }

MiddleLandMap::MiddleLandMap(int x, int z, int d) {
    this->x      = x;
    this->z      = z;
    this->d      = d;
    this->size   = 100;
    this->bigMap = nullptr;

    this->map = new SmallLandMap*[this->size * this->size];
    for (int i = 0; i < this->size * this->size; i++) map[i] = nullptr;
}

MiddleLandMap::~MiddleLandMap() { delete this->map; }

void MiddleLandMap::setMap(int sx, int sz, SmallLandMap* sm) { this->map[sx + sz * this->size] = sm; }

SmallLandMap* MiddleLandMap::getMap(int sx, int sz) { return this->map[sx + sz * this->size]; }

SmallLandMap::SmallLandMap(int x, int z, int d) {
    this->x         = x;
    this->z         = z;
    this->d         = d;
    this->size      = 100;
    this->middleMap = nullptr;
    this->bigMap    = nullptr;

    this->map = new LandInformation*[this->size * this->size];
    for (int i = 0; i < this->size * this->size; i++) map[i] = nullptr;
}

void SmallLandMap::setLand(int sx, int sz, LandInformation* li) { this->map[sx + sz * this->size] = li; }

LandInformation* SmallLandMap::getLand(int sx, int sz) { return this->map[sx + sz * this->size]; }

std::shared_ptr<LandMap> LandMap::getInstance() {

    static std::shared_ptr<LandMap> instance = std::make_shared<LandMap>();
    return instance;
}

LandInformation* LandMap::find(LONG64 coordx, LONG64 coordz, int d) {
    int    bigx, bigz, middlex, middlez, smallx, smallz;
    LONG64 x = coordx;
    LONG64 z = coordz;

    bigx = (int)((coordx + LAND_BIG_SIZE * 10) / LAND_BIG_SIZE);
    bigz = (int)((coordz + LAND_BIG_SIZE * 10) / LAND_BIG_SIZE);

    x = abs(x) - abs(coordx / LAND_BIG_SIZE * LAND_BIG_SIZE);
    z = abs(z) - abs(coordz / LAND_BIG_SIZE * LAND_BIG_SIZE);

    if (bigx >= 20 || bigx < 0 || bigz >= 20 || bigz < 0) return nullptr;

    BigLandMap* bigMap = LandMap::map[bigx][bigz][d];
    if (bigMap == nullptr) return nullptr;

    middlex = (int)(x / LAND_MIDDLE_SIZE);
    middlez = (int)(z / LAND_MIDDLE_SIZE);

    x = x - middlex * LAND_MIDDLE_SIZE;
    z = z - middlez * LAND_MIDDLE_SIZE;

    MiddleLandMap* middleMap = bigMap->getMap(middlex, middlez);
    if (middleMap == nullptr) return nullptr;

    smallx = (int)(x / LAND_SMALL_SIZE);
    smallz = (int)(z / LAND_SMALL_SIZE);

    x = x - smallx * LAND_SMALL_SIZE;
    z = z - smallz * LAND_SMALL_SIZE;

    SmallLandMap* smallMap = middleMap->getMap(smallx, smallz);
    if (smallMap == nullptr) return nullptr;

    return smallMap->getLand((int)x, (int)z);
}

void LandMap::set(LandInformation* li, LONG64 xi, LONG64 zi, int d) {
    int    bigx, bigz, middlex, middlez, smallx, smallz;
    LONG64 x = xi, z = zi;
    bigx = (int)((xi + LAND_BIG_SIZE * 10) / LAND_BIG_SIZE);
    bigz = (int)((zi + LAND_BIG_SIZE * 10) / LAND_BIG_SIZE);

    x = abs(x) - abs(xi / LAND_BIG_SIZE * LAND_BIG_SIZE);
    z = abs(z) - abs(zi / LAND_BIG_SIZE * LAND_BIG_SIZE);

    if (bigx >= 20 || bigx < 0 || bigz >= 20 || bigz < 0) return;

    BigLandMap* bigMap = LandMap::map[bigx][bigz][d];
    if (bigMap == nullptr) {
        bigMap = new BigLandMap(bigx, bigz, d);

        LandMap::map[bigx][bigz][d] = bigMap;
    }

    middlex = (int)(x / LAND_MIDDLE_SIZE);
    middlez = (int)(z / LAND_MIDDLE_SIZE);

    x                        = x - middlex * LAND_MIDDLE_SIZE;
    z                        = z - middlez * LAND_MIDDLE_SIZE;
    MiddleLandMap* middleMap = bigMap->getMap(middlex, middlez);
    if (middleMap == nullptr) {
        middleMap         = new MiddleLandMap(middlex, middlez, d);
        middleMap->bigMap = bigMap;
        bigMap->setMap(middlex, middlez, middleMap);
    }

    smallx = (int)(x / LAND_SMALL_SIZE);
    smallz = (int)(z / LAND_SMALL_SIZE);

    x = x - smallx * LAND_SMALL_SIZE;
    z = z - smallz * LAND_SMALL_SIZE;

    SmallLandMap* smallMap = middleMap->getMap(smallx, smallz);
    if (smallMap == nullptr) {
        smallMap            = new SmallLandMap(smallx, smallz, d);
        smallMap->bigMap    = bigMap;
        smallMap->middleMap = middleMap;
        middleMap->setMap(smallx, smallz, smallMap);
    }

    smallMap->setLand((int)x, (int)z, li);
}

} // namespace rlx_land