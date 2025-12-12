#pragma once
#include "data/core/BaseInformation.h"
#include <memory>

namespace rlx_land {

// 前向声明
template <typename InfoType>
class MiddleMap;
template <typename InfoType>
class BigMap;
class DataService;

template <typename InfoType>
class SpatialMap {
private:
    // 只有 DataService 可以访问 SpatialMap 的公共接口
    friend class DataService;
    friend class MiddleMap<InfoType>;
    friend class BigMap<InfoType>;

    static std::shared_ptr<SpatialMap> getInstance() {
        static std::shared_ptr<SpatialMap> spatialMapInstance = std::make_shared<SpatialMap>();
        return spatialMapInstance;
    }

    InfoType* find(LONG64 coordx, LONG64 coordz, int d);
    void      set(InfoType* info, LONG64 xi, LONG64 zi, int d);
    void      clearAll(); // 清理所有空间地图数据

    BigMap<InfoType>* map[20][20][3] = {};
};

// MiddleMap 直接存储 InfoType** 数组指针（方案2：去掉SmallMap类）
template <typename InfoType>
class MiddleMap {
public:
    MiddleMap();
    ~MiddleMap();

    // 数组操作
    void       setInfoArray(int sx, int sz, InfoType** arr);
    InfoType** getInfoArray(int sx, int sz);
    InfoType** getOrCreateInfoArray(int sx, int sz);

    // 直接访问InfoType
    void      setInfo(int sx, int sz, int ix, int iz, InfoType* info);
    InfoType* getInfo(int sx, int sz, int ix, int iz);

private:
    static constexpr int SIZE      = 100; // MiddleMap内数组个数：100x100
    static constexpr int INFO_SIZE = 100; // 每个数组存储InfoType*的个数：100x100
    InfoType***          map;             // 指向 InfoType*[10000] 数组的指针数组
};

template <typename InfoType>
class BigMap {
public:
    BigMap(int x, int z, int d);
    void                 setMap(int mx, int mz, MiddleMap<InfoType>* m);
    MiddleMap<InfoType>* getMap(int mx, int mz);
    ~BigMap();

    int x, z, d;
    int size = 10;

private:
    MiddleMap<InfoType>** map;
};

// MiddleMap 实现
template <typename InfoType>
MiddleMap<InfoType>::MiddleMap() {
    map = new InfoType**[SIZE * SIZE];
    for (int i = 0; i < SIZE * SIZE; i++) map[i] = nullptr;
}

template <typename InfoType>
void MiddleMap<InfoType>::setInfoArray(int sx, int sz, InfoType** arr) {
    map[sx + sz * SIZE] = arr;
}

template <typename InfoType>
InfoType** MiddleMap<InfoType>::getInfoArray(int sx, int sz) {
    return map[sx + sz * SIZE];
}

template <typename InfoType>
InfoType** MiddleMap<InfoType>::getOrCreateInfoArray(int sx, int sz) {
    int idx = sx + sz * SIZE;
    if (map[idx] == nullptr) {
        map[idx] = new InfoType*[INFO_SIZE * INFO_SIZE];
        for (int i = 0; i < INFO_SIZE * INFO_SIZE; i++) map[idx][i] = nullptr;
    }
    return map[idx];
}

template <typename InfoType>
void MiddleMap<InfoType>::setInfo(int sx, int sz, int ix, int iz, InfoType* info) {
    InfoType** arr           = getOrCreateInfoArray(sx, sz);
    arr[ix + iz * INFO_SIZE] = info;
}

template <typename InfoType>
InfoType* MiddleMap<InfoType>::getInfo(int sx, int sz, int ix, int iz) {
    InfoType** arr = getInfoArray(sx, sz);
    if (arr == nullptr) return nullptr;
    return arr[ix + iz * INFO_SIZE];
}

template <typename InfoType>
MiddleMap<InfoType>::~MiddleMap() {
    for (int i = 0; i < SIZE * SIZE; i++) {
        if (map[i] != nullptr) {
            delete[] map[i]; // 释放 InfoType*[] 数组
            map[i] = nullptr;
        }
    }
    delete[] map;
}

// BigMap 实现
template <typename InfoType>
BigMap<InfoType>::BigMap(int x, int z, int d) : x(x),
                                                z(z),
                                                d(d) {
    map = new MiddleMap<InfoType>*[size * size];
    for (int i = 0; i < size * size; i++) map[i] = nullptr;
}

template <typename InfoType>
void BigMap<InfoType>::setMap(int mx, int mz, MiddleMap<InfoType>* m) {
    map[mx + mz * size] = m;
}

template <typename InfoType>
MiddleMap<InfoType>* BigMap<InfoType>::getMap(int mx, int mz) {
    return map[mx + mz * size];
}

template <typename InfoType>
BigMap<InfoType>::~BigMap() {
    // 注意：不删除 MiddleMap 对象中的 InfoType 对象
    // 只清理 MiddleMap 对象本身
    for (int i = 0; i < size * size; i++) {
        if (map[i] != nullptr) {
            delete map[i]; // 删除 MiddleMap 对象
            map[i] = nullptr;
        }
    }
    delete[] map;
}

// SpatialMap 实现
template <typename InfoType>
InfoType* SpatialMap<InfoType>::find(LONG64 coordx, LONG64 coordz, int d) {
    // 添加维度边界检查
    if (d < 0 || d >= 3) return nullptr;

    // 减1是为了确保 LAND_RANGE = 1000000 边界值能正确映射到索引19而不是越界到20
    // 这样修改后，坐标范围 [-1000000, 1000000] 都能正确映射到 [0, 19] 的数组索引
    int bigx = (int)((coordx + 100000 * 10 - 1) / 100000);
    int bigz = (int)((coordz + 100000 * 10 - 1) / 100000);

    LONG64 x = std::abs(coordx) - std::abs(coordx / 100000 * 100000);
    LONG64 z = std::abs(coordz) - std::abs(coordz / 100000 * 100000);

    if (bigx >= 20 || bigx < 0 || bigz >= 20 || bigz < 0) return nullptr;

    BigMap<InfoType>* bigMap = this->map[bigx][bigz][d];
    if (bigMap == nullptr) return nullptr;

    int middlex = (int)(x / 10000);
    int middlez = (int)(z / 10000);

    x = x - middlex * 10000;
    z = z - middlez * 10000;

    MiddleMap<InfoType>* middleMap = bigMap->getMap(middlex, middlez);
    if (middleMap == nullptr) return nullptr;

    int smallx = (int)(x / 100);
    int smallz = (int)(z / 100);

    x = x - smallx * 100;
    z = z - smallz * 100;

    // 直接从 MiddleMap 获取 InfoType
    return middleMap->getInfo(smallx, smallz, (int)x, (int)z);
}

template <typename InfoType>
void SpatialMap<InfoType>::set(InfoType* info, LONG64 xi, LONG64 zi, int d) {
    // 添加维度边界检查
    if (d < 0 || d >= 3) return;

    // 减1是为了确保 LAND_RANGE = 1000000 边界值能正确映射到索引19而不是越界到20
    // 这样修改后，坐标范围 [-1000000, 1000000] 都能正确映射到 [0, 19] 的数组索引
    int bigx = (int)((xi + 100000 * 10 - 1) / 100000);
    int bigz = (int)((zi + 100000 * 10 - 1) / 100000);

    LONG64 x = std::abs(xi) - std::abs(xi / 100000 * 100000);
    LONG64 z = std::abs(zi) - std::abs(zi / 100000 * 100000);

    if (bigx >= 20 || bigx < 0 || bigz >= 20 || bigz < 0) return;

    BigMap<InfoType>* bigMap = this->map[bigx][bigz][d];
    if (bigMap == nullptr) {
        bigMap                   = new BigMap<InfoType>(bigx, bigz, d);
        this->map[bigx][bigz][d] = bigMap;
    }

    int middlex = (int)(x / 10000);
    int middlez = (int)(z / 10000);

    x = x - middlex * 10000;
    z = z - middlez * 10000;

    MiddleMap<InfoType>* middleMap = bigMap->getMap(middlex, middlez);
    if (middleMap == nullptr) {
        middleMap = new MiddleMap<InfoType>();
        bigMap->setMap(middlex, middlez, middleMap);
    }

    int smallx = (int)(x / 100);
    int smallz = (int)(z / 100);

    x = x - smallx * 100;
    z = z - smallz * 100;

    // 直接在 MiddleMap 中设置 InfoType
    middleMap->setInfo(smallx, smallz, (int)x, (int)z, info);
}

template <typename InfoType>
void SpatialMap<InfoType>::clearAll() {
    for (int x = 0; x < 20; x++) {
        for (int z = 0; z < 20; z++) {
            for (int d = 0; d < 3; d++) {
                if (map[x][z][d] != nullptr) {
                    delete map[x][z][d];
                    map[x][z][d] = nullptr;
                }
            }
        }
    }
}

} // namespace rlx_land
