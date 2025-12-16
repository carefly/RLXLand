#pragma once
#include "data/core/BaseInformation.h"
#include <cassert>
#include <memory>

namespace rlx_land {

// 存储模式枚举
enum class BlockStorageMode { Empty = 0, FullCover = 1, PointArray = 2 };

// BlockEntry: 优化的块级存储结构，使用明确的模式枚举
// 现在每个 BlockEntry 覆盖 10×10 坐标
template <typename InfoType>
struct BlockEntry {
private:
    union {
        InfoType*  fullCoverInfo; // 整块填充时的信息指针
        InfoType** pointArray;    // 点数组模式时的数组指针
    } data;

    BlockStorageMode mode;

public:
    static constexpr int SIZE = 10; // 10×10 坐标

    BlockEntry() : mode(BlockStorageMode::Empty) { data.fullCoverInfo = nullptr; }

    ~BlockEntry() { clear(); }

    // 禁止拷贝构造和拷贝赋值（避免双重释放）
    BlockEntry(const BlockEntry&)            = delete;
    BlockEntry& operator=(const BlockEntry&) = delete;

    // 允许移动构造和移动赋值
    BlockEntry(BlockEntry&& other) noexcept : mode(other.mode), data(other.data) {
        other.mode               = BlockStorageMode::Empty;
        other.data.fullCoverInfo = nullptr;
    }

    BlockEntry& operator=(BlockEntry&& other) noexcept {
        if (this != &other) {
            clear();
            mode                     = other.mode;
            data                     = other.data;
            other.mode               = BlockStorageMode::Empty;
            other.data.fullCoverInfo = nullptr;
        }
        return *this;
    }

    // 判断存储模式
    bool isEmpty() const { return mode == BlockStorageMode::Empty; }
    bool isFullCover() const { return mode == BlockStorageMode::FullCover; }
    bool isPointArray() const { return mode == BlockStorageMode::PointArray; }

    // 获取整块填充的指针
    InfoType* getFullCover() const { return (mode == BlockStorageMode::FullCover) ? data.fullCoverInfo : nullptr; }

    // 获取点数组指针
    InfoType** getPointArray() const { return (mode == BlockStorageMode::PointArray) ? data.pointArray : nullptr; }

    // 设置整块填充（安全：会先清理现有数据）
    void setFullCover(InfoType* info) {
        clear();
        mode               = BlockStorageMode::FullCover;
        data.fullCoverInfo = info;
    }

    // 设置点数组（安全：会先清理现有数据）
    void setPointArray(InfoType** arr) {
        clear();
        mode            = BlockStorageMode::PointArray;
        data.pointArray = arr;
    }

    // 清理数据（安全的内存释放）
    void clear() {
        if (mode == BlockStorageMode::PointArray && data.pointArray != nullptr) {
            delete[] data.pointArray;
        }
        mode               = BlockStorageMode::Empty;
        data.fullCoverInfo = nullptr;
    }

    // 检查指针有效性（调试用）
    bool isValid() const {
        switch (mode) {
        case BlockStorageMode::Empty:
            return data.fullCoverInfo == nullptr;
        case BlockStorageMode::FullCover:
            return data.fullCoverInfo != nullptr;
        case BlockStorageMode::PointArray:
            return data.pointArray != nullptr;
        default:
            return false;
        }
    }
};

// 前向声明
template <typename InfoType>
class SmallMap;
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
    friend class SmallMap<InfoType>;
    friend class MiddleMap<InfoType>;
    friend class BigMap<InfoType>;

    static std::shared_ptr<SpatialMap> getInstance() {
        static std::shared_ptr<SpatialMap> spatialMapInstance = std::make_shared<SpatialMap>();
        return spatialMapInstance;
    }

    InfoType* find(LONG64 coordx, LONG64 coordz, int d);
    void      set(InfoType* info, LONG64 xi, LONG64 zi, int d);
    void      setRange(InfoType* info, LONG64 x1, LONG64 z1, LONG64 x2, LONG64 z2, int d); // 范围设置（优化）
    void      clearAll(); // 清理所有空间地图数据

    BigMap<InfoType>* map[20][20][3] = {};
};

// SmallMap: 持有 10×10 个 BlockEntry（固定分配）
// 每个 SmallMap 覆盖 100×100 坐标
template <typename InfoType>
class SmallMap {
public:
    SmallMap();
    ~SmallMap();

    // 查询接口：ix, iz 是 [0, 99] 的坐标
    InfoType* getInfo(int ix, int iz);

    // 设置接口
    void setInfo(int ix, int iz, InfoType* info);
    void setBlockFullCover(int bx, int bz, InfoType* info); // 设置整个 BlockEntry 填充

    // 范围设置（优化用）：x1, z1, x2, z2 是 [0, 99] 的坐标
    void setRange(int x1, int z1, int x2, int z2, InfoType* info);

private:
    static constexpr int GRID_SIZE  = 10;  // 10×10 个 BlockEntry
    static constexpr int BLOCK_SIZE = 10;  // 每个 BlockEntry 10×10 坐标
    static constexpr int TOTAL_SIZE = 100; // SmallMap 总共覆盖 100×100 坐标

    BlockEntry<InfoType> blocks[GRID_SIZE * GRID_SIZE]; // 固定分配 100 个 BlockEntry
};

// MiddleMap: 持有 100×100 个 SmallMap 指针（按需分配）
// 每个 MiddleMap 覆盖 10,000×10,000 坐标
template <typename InfoType>
class MiddleMap {
public:
    MiddleMap();
    ~MiddleMap();

    // 查询接口：ix, iz 是 [0, 9999] 的坐标
    InfoType* getInfo(int ix, int iz);

    // 设置接口
    void setInfo(int ix, int iz, InfoType* info);

    // 范围设置（优化用）：x1, z1, x2, z2 是 [0, 9999] 的坐标
    void setRange(int x1, int z1, int x2, int z2, InfoType* info);

private:
    static constexpr int SIZE       = 100;   // 100×100 个 SmallMap
    static constexpr int SMALL_SIZE = 100;   // 每个 SmallMap 100×100 坐标
    static constexpr int TOTAL_SIZE = 10000; // MiddleMap 总共覆盖 10,000×10,000 坐标

    SmallMap<InfoType>** smallMaps; // SmallMap*[100×100]，按需分配
};

template <typename InfoType>
class BigMap {
public:
    BigMap(int x, int z, int d);
    void                 setMap(int mx, int mz, MiddleMap<InfoType>* m);
    MiddleMap<InfoType>* getMap(int mx, int mz);
    ~BigMap();

    int x, z, d;
    int size = 10; // 10×10 个 MiddleMap

private:
    MiddleMap<InfoType>** map; // MiddleMap*[10×10]
};

// ================ SmallMap 实现 ================

template <typename InfoType>
SmallMap<InfoType>::SmallMap() {
    // BlockEntry 数组已固定分配，默认构造函数会初始化为 Empty 模式
}

template <typename InfoType>
SmallMap<InfoType>::~SmallMap() {
    // BlockEntry 数组的析构函数会自动调用每个元素的析构函数
}

template <typename InfoType>
InfoType* SmallMap<InfoType>::getInfo(int ix, int iz) {
    // 边界检查：ix, iz 应该在 [0, 99] 范围内
    if (ix < 0 || ix >= TOTAL_SIZE || iz < 0 || iz >= TOTAL_SIZE) return nullptr;

    // 计算所属的 BlockEntry
    int bx  = ix / BLOCK_SIZE;
    int bz  = iz / BLOCK_SIZE;
    int idx = bx + bz * GRID_SIZE;

    BlockEntry<InfoType>& block = blocks[idx];

    // 调试断言：确保 BlockEntry 处于有效状态
    assert(block.isValid());

    // 如果是整块填充，直接返回
    if (block.isFullCover()) {
        return block.getFullCover();
    }

    // 否则从点数组获取
    InfoType** arr = block.getPointArray();
    if (arr == nullptr) {
        return nullptr;
    }

    // 计算 Block 内的局部坐标
    int localX = ix % BLOCK_SIZE;
    int localZ = iz % BLOCK_SIZE;

    return arr[localX + localZ * BLOCK_SIZE];
}

template <typename InfoType>
void SmallMap<InfoType>::setBlockFullCover(int bx, int bz, InfoType* info) {
    // 边界检查：bx, bz 应该在 [0, 9] 范围内
    if (bx < 0 || bx >= GRID_SIZE || bz < 0 || bz >= GRID_SIZE) return;

    int                   idx   = bx + bz * GRID_SIZE;
    BlockEntry<InfoType>& block = blocks[idx];

    // 使用 BlockEntry 的安全设置方法，会自动清理现有数据
    block.setFullCover(info);
}

template <typename InfoType>
void SmallMap<InfoType>::setInfo(int ix, int iz, InfoType* info) {
    // 边界检查：ix, iz 应该在 [0, 99] 范围内
    if (ix < 0 || ix >= TOTAL_SIZE || iz < 0 || iz >= TOTAL_SIZE) return;

    // 计算所属的 BlockEntry
    int bx  = ix / BLOCK_SIZE;
    int bz  = iz / BLOCK_SIZE;
    int idx = bx + bz * GRID_SIZE;

    BlockEntry<InfoType>& block = blocks[idx];

    // 如果当前是整块填充，需要展开为点数组
    if (block.isFullCover()) {
        InfoType*  oldInfo = block.getFullCover();
        InfoType** arr     = new InfoType*[BLOCK_SIZE * BLOCK_SIZE];

        // 用原来的值填充所有点
        for (int i = 0; i < BLOCK_SIZE * BLOCK_SIZE; i++) {
            arr[i] = oldInfo;
        }

        block.setPointArray(arr);
    }

    // 确保点数组存在
    if (block.isEmpty()) {
        InfoType** arr = new InfoType*[BLOCK_SIZE * BLOCK_SIZE];
        for (int i = 0; i < BLOCK_SIZE * BLOCK_SIZE; i++) {
            arr[i] = nullptr;
        }
        block.setPointArray(arr);
    }

    // 设置指定位置的值
    InfoType** arr = block.getPointArray();
    if (arr != nullptr) {
        int localX                        = ix % BLOCK_SIZE;
        int localZ                        = iz % BLOCK_SIZE;
        arr[localX + localZ * BLOCK_SIZE] = info;
    }
}

template <typename InfoType>
void SmallMap<InfoType>::setRange(int x1, int z1, int x2, int z2, InfoType* info) {
    // 边界检查
    if (x1 < 0) x1 = 0;
    if (z1 < 0) z1 = 0;
    if (x2 >= TOTAL_SIZE) x2 = TOTAL_SIZE - 1;
    if (z2 >= TOTAL_SIZE) z2 = TOTAL_SIZE - 1;

    // 计算 BlockEntry 范围
    int bx1 = x1 / BLOCK_SIZE;
    int bz1 = z1 / BLOCK_SIZE;
    int bx2 = x2 / BLOCK_SIZE;
    int bz2 = z2 / BLOCK_SIZE;

    // 遍历所有涉及的 BlockEntry
    for (int bx = bx1; bx <= bx2; bx++) {
        for (int bz = bz1; bz <= bz2; bz++) {
            // 计算当前 Block 内的坐标范围
            int localX1 = (bx == bx1) ? (x1 % BLOCK_SIZE) : 0;
            int localZ1 = (bz == bz1) ? (z1 % BLOCK_SIZE) : 0;
            int localX2 = (bx == bx2) ? (x2 % BLOCK_SIZE) : (BLOCK_SIZE - 1);
            int localZ2 = (bz == bz2) ? (z2 % BLOCK_SIZE) : (BLOCK_SIZE - 1);

            // 判断是否整块覆盖
            bool isFullBlock = (localX1 == 0 && localZ1 == 0 && localX2 == BLOCK_SIZE - 1 && localZ2 == BLOCK_SIZE - 1);

            if (isFullBlock) {
                // 整块覆盖，使用整块填充模式（O(1) 操作）
                setBlockFullCover(bx, bz, info);
            } else {
                // 部分覆盖，需要逐点设置
                for (int x = localX1; x <= localX2; x++) {
                    for (int z = localZ1; z <= localZ2; z++) {
                        int globalX = bx * BLOCK_SIZE + x;
                        int globalZ = bz * BLOCK_SIZE + z;
                        setInfo(globalX, globalZ, info);
                    }
                }
            }
        }
    }
}

// ================ MiddleMap 实现 ================

template <typename InfoType>
MiddleMap<InfoType>::MiddleMap() {
    smallMaps = new SmallMap<InfoType>*[SIZE * SIZE];
    for (int i = 0; i < SIZE * SIZE; i++) {
        smallMaps[i] = nullptr;
    }
}

template <typename InfoType>
MiddleMap<InfoType>::~MiddleMap() {
    for (int i = 0; i < SIZE * SIZE; i++) {
        if (smallMaps[i] != nullptr) {
            delete smallMaps[i];
            smallMaps[i] = nullptr;
        }
    }
    delete[] smallMaps;
}

template <typename InfoType>
InfoType* MiddleMap<InfoType>::getInfo(int ix, int iz) {
    // 边界检查：ix, iz 应该在 [0, 9999] 范围内
    if (ix < 0 || ix >= TOTAL_SIZE || iz < 0 || iz >= TOTAL_SIZE) return nullptr;

    // 计算所属的 SmallMap
    int sx  = ix / SMALL_SIZE;
    int sz  = iz / SMALL_SIZE;
    int idx = sx + sz * SIZE;

    SmallMap<InfoType>* smallMap = smallMaps[idx];
    if (smallMap == nullptr) {
        return nullptr;
    }

    // 计算 SmallMap 内的局部坐标
    int localX = ix % SMALL_SIZE;
    int localZ = iz % SMALL_SIZE;

    return smallMap->getInfo(localX, localZ);
}

template <typename InfoType>
void MiddleMap<InfoType>::setInfo(int ix, int iz, InfoType* info) {
    // 边界检查：ix, iz 应该在 [0, 9999] 范围内
    if (ix < 0 || ix >= TOTAL_SIZE || iz < 0 || iz >= TOTAL_SIZE) return;

    // 计算所属的 SmallMap
    int sx  = ix / SMALL_SIZE;
    int sz  = iz / SMALL_SIZE;
    int idx = sx + sz * SIZE;

    SmallMap<InfoType>* smallMap = smallMaps[idx];
    if (smallMap == nullptr) {
        smallMap       = new SmallMap<InfoType>();
        smallMaps[idx] = smallMap;
    }

    // 计算 SmallMap 内的局部坐标
    int localX = ix % SMALL_SIZE;
    int localZ = iz % SMALL_SIZE;

    smallMap->setInfo(localX, localZ, info);
}

template <typename InfoType>
void MiddleMap<InfoType>::setRange(int x1, int z1, int x2, int z2, InfoType* info) {
    // 边界检查
    if (x1 < 0) x1 = 0;
    if (z1 < 0) z1 = 0;
    if (x2 >= TOTAL_SIZE) x2 = TOTAL_SIZE - 1;
    if (z2 >= TOTAL_SIZE) z2 = TOTAL_SIZE - 1;

    // 计算 SmallMap 范围
    int sx1 = x1 / SMALL_SIZE;
    int sz1 = z1 / SMALL_SIZE;
    int sx2 = x2 / SMALL_SIZE;
    int sz2 = z2 / SMALL_SIZE;

    // 遍历所有涉及的 SmallMap
    for (int sx = sx1; sx <= sx2; sx++) {
        for (int sz = sz1; sz <= sz2; sz++) {
            int idx = sx + sz * SIZE;

            // 按需创建 SmallMap
            SmallMap<InfoType>* smallMap = smallMaps[idx];
            if (smallMap == nullptr) {
                smallMap       = new SmallMap<InfoType>();
                smallMaps[idx] = smallMap;
            }

            // 计算当前 SmallMap 内的坐标范围
            int localX1 = (sx == sx1) ? (x1 % SMALL_SIZE) : 0;
            int localZ1 = (sz == sz1) ? (z1 % SMALL_SIZE) : 0;
            int localX2 = (sx == sx2) ? (x2 % SMALL_SIZE) : (SMALL_SIZE - 1);
            int localZ2 = (sz == sz2) ? (z2 % SMALL_SIZE) : (SMALL_SIZE - 1);

            // 调用 SmallMap 的范围设置
            smallMap->setRange(localX1, localZ1, localX2, localZ2, info);
        }
    }
}

// ================ BigMap 实现 ================

template <typename InfoType>
BigMap<InfoType>::BigMap(int x, int z, int d) : x(x),
                                                z(z),
                                                d(d) {
    map = new MiddleMap<InfoType>*[size * size];
    for (int i = 0; i < size * size; i++) map[i] = nullptr;
}

template <typename InfoType>
void BigMap<InfoType>::setMap(int mx, int mz, MiddleMap<InfoType>* m) {
    // 边界检查
    if (mx < 0 || mx >= size || mz < 0 || mz >= size) return;
    map[mx + mz * size] = m;
}

template <typename InfoType>
MiddleMap<InfoType>* BigMap<InfoType>::getMap(int mx, int mz) {
    // 边界检查
    if (mx < 0 || mx >= size || mz < 0 || mz >= size) return nullptr;
    return map[mx + mz * size];
}

template <typename InfoType>
BigMap<InfoType>::~BigMap() {
    for (int i = 0; i < size * size; i++) {
        if (map[i] != nullptr) {
            delete map[i];
            map[i] = nullptr;
        }
    }
    delete[] map;
}

// ================ SpatialMap 实现 ================

template <typename InfoType>
InfoType* SpatialMap<InfoType>::find(LONG64 coordx, LONG64 coordz, int d) {
    // 添加维度边界检查
    if (d < 0 || d >= 3) return nullptr;

    // 坐标范围 [-1000000, 999999] 映射到 [0, 19] 的数组索引
    int bigx = (int)((coordx + 100000 * 10) / 100000);
    int bigz = (int)((coordz + 100000 * 10) / 100000);

    if (bigx >= 20 || bigx < 0 || bigz >= 20 || bigz < 0) return nullptr;

    BigMap<InfoType>* bigMap = this->map[bigx][bigz][d];
    if (bigMap == nullptr) return nullptr;

    // 计算当前 BigMap 的基础坐标
    LONG64 bigBaseX = (LONG64)(bigx - 10) * 100000;
    LONG64 bigBaseZ = (LONG64)(bigz - 10) * 100000;

    // 计算 BigMap 内的局部坐标
    LONG64 x = coordx - bigBaseX;
    LONG64 z = coordz - bigBaseZ;

    // 计算所属的 MiddleMap (10×10)
    int middlex = (int)(x / 10000);
    int middlez = (int)(z / 10000);

    if (middlex >= 10 || middlex < 0 || middlez >= 10 || middlez < 0) return nullptr;

    MiddleMap<InfoType>* middleMap = bigMap->getMap(middlex, middlez);
    if (middleMap == nullptr) return nullptr;

    // 计算 MiddleMap 内的局部坐标
    int localX = (int)(x - middlex * 10000);
    int localZ = (int)(z - middlez * 10000);

    if (localX >= 10000 || localX < 0 || localZ >= 10000 || localZ < 0) return nullptr;

    // 直接从 MiddleMap 获取 InfoType
    return middleMap->getInfo(localX, localZ);
}

template <typename InfoType>
void SpatialMap<InfoType>::set(InfoType* info, LONG64 xi, LONG64 zi, int d) {
    // 添加维度边界检查
    if (d < 0 || d >= 3) return;

    // 坐标范围 [-1000000, 999999] 映射到 [0, 19] 的数组索引
    int bigx = (int)((xi + 100000 * 10) / 100000);
    int bigz = (int)((zi + 100000 * 10) / 100000);

    if (bigx >= 20 || bigx < 0 || bigz >= 20 || bigz < 0) return;

    BigMap<InfoType>* bigMap = this->map[bigx][bigz][d];
    if (bigMap == nullptr) {
        bigMap                   = new BigMap<InfoType>(bigx, bigz, d);
        this->map[bigx][bigz][d] = bigMap;
    }

    // 计算当前 BigMap 的基础坐标
    LONG64 bigBaseX = (LONG64)(bigx - 10) * 100000;
    LONG64 bigBaseZ = (LONG64)(bigz - 10) * 100000;

    // 计算 BigMap 内的局部坐标
    LONG64 x = xi - bigBaseX;
    LONG64 z = zi - bigBaseZ;

    // 计算所属的 MiddleMap (10×10)
    int middlex = (int)(x / 10000);
    int middlez = (int)(z / 10000);

    if (middlex >= 10 || middlex < 0 || middlez >= 10 || middlez < 0) return;

    MiddleMap<InfoType>* middleMap = bigMap->getMap(middlex, middlez);
    if (middleMap == nullptr) {
        middleMap = new MiddleMap<InfoType>();
        bigMap->setMap(middlex, middlez, middleMap);
    }

    // 计算 MiddleMap 内的局部坐标
    int localX = (int)(x - middlex * 10000);
    int localZ = (int)(z - middlez * 10000);

    if (localX >= 10000 || localX < 0 || localZ >= 10000 || localZ < 0) return;

    // 直接在 MiddleMap 中设置 InfoType
    middleMap->setInfo(localX, localZ, info);
}

template <typename InfoType>
void SpatialMap<InfoType>::setRange(InfoType* info, LONG64 x1, LONG64 z1, LONG64 x2, LONG64 z2, int d) {
    // 维度边界检查
    if (d < 0 || d >= 3) return;

    // 计算涉及的 BigMap 范围
    int bigx1 = (int)((x1 + 100000 * 10) / 100000);
    int bigz1 = (int)((z1 + 100000 * 10) / 100000);
    int bigx2 = (int)((x2 + 100000 * 10) / 100000);
    int bigz2 = (int)((z2 + 100000 * 10) / 100000);

    for (int bigx = bigx1; bigx <= bigx2; bigx++) {
        for (int bigz = bigz1; bigz <= bigz2; bigz++) {
            if (bigx >= 20 || bigx < 0 || bigz >= 20 || bigz < 0) continue;

            BigMap<InfoType>* bigMap = this->map[bigx][bigz][d];
            if (bigMap == nullptr) {
                bigMap                   = new BigMap<InfoType>(bigx, bigz, d);
                this->map[bigx][bigz][d] = bigMap;
            }

            // 计算当前 BigMap 的基础坐标
            LONG64 bigBaseX = (LONG64)(bigx - 10) * 100000;
            LONG64 bigBaseZ = (LONG64)(bigz - 10) * 100000;

            // 计算当前 BigMap 内的坐标范围
            LONG64 localX1 = std::max(x1, bigBaseX) - bigBaseX;
            LONG64 localZ1 = std::max(z1, bigBaseZ) - bigBaseZ;
            LONG64 localX2 = std::min(x2, bigBaseX + 100000 - 1) - bigBaseX;
            LONG64 localZ2 = std::min(z2, bigBaseZ + 100000 - 1) - bigBaseZ;

            // 计算涉及的 MiddleMap 范围
            int middlex1 = (int)(localX1 / 10000);
            int middlez1 = (int)(localZ1 / 10000);
            int middlex2 = (int)(localX2 / 10000);
            int middlez2 = (int)(localZ2 / 10000);

            for (int middlex = middlex1; middlex <= middlex2; middlex++) {
                for (int middlez = middlez1; middlez <= middlez2; middlez++) {
                    MiddleMap<InfoType>* middleMap = bigMap->getMap(middlex, middlez);
                    if (middleMap == nullptr) {
                        middleMap = new MiddleMap<InfoType>();
                        bigMap->setMap(middlex, middlez, middleMap);
                    }

                    // 计算当前 MiddleMap 的基础坐标
                    LONG64 middleBaseX = (LONG64)middlex * 10000;
                    LONG64 middleBaseZ = (LONG64)middlez * 10000;

                    // 计算当前 MiddleMap 内的坐标范围
                    int rangeX1 = (int)(std::max(localX1, middleBaseX) - middleBaseX);
                    int rangeZ1 = (int)(std::max(localZ1, middleBaseZ) - middleBaseZ);
                    int rangeX2 = (int)(std::min(localX2, middleBaseX + 10000 - 1) - middleBaseX);
                    int rangeZ2 = (int)(std::min(localZ2, middleBaseZ + 10000 - 1) - middleBaseZ);

                    // 调用 MiddleMap 的范围设置
                    middleMap->setRange(rangeX1, rangeZ1, rangeX2, rangeZ2, info);
                }
            }
        }
    }
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
