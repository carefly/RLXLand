#pragma once
#include "data/core/BaseInformation.h"
#include <memory>
#include <cassert>

namespace rlx_land {

// 存储模式枚举
enum class BlockStorageMode {
    Empty = 0,
    FullCover = 1,
    PointArray = 2
};

// BlockEntry: 优化的块级存储结构，使用明确的模式枚举
template <typename InfoType>
struct BlockEntry {
private:
    union {
        InfoType* fullCoverInfo;     // 整块填充时的信息指针
        InfoType** pointArray;       // 点数组模式时的数组指针
    } data;

    BlockStorageMode mode;

public:
    BlockEntry() : mode(BlockStorageMode::Empty) {
        data.fullCoverInfo = nullptr;
    }

    ~BlockEntry() {
        clear();
    }

    // 禁止拷贝构造和拷贝赋值（避免双重释放）
    BlockEntry(const BlockEntry&) = delete;
    BlockEntry& operator=(const BlockEntry&) = delete;

    // 允许移动构造和移动赋值
    BlockEntry(BlockEntry&& other) noexcept : mode(other.mode), data(other.data) {
        other.mode = BlockStorageMode::Empty;
        other.data.fullCoverInfo = nullptr;
    }

    BlockEntry& operator=(BlockEntry&& other) noexcept {
        if (this != &other) {
            clear();
            mode = other.mode;
            data = other.data;
            other.mode = BlockStorageMode::Empty;
            other.data.fullCoverInfo = nullptr;
        }
        return *this;
    }

    // 判断存储模式
    bool isEmpty() const { return mode == BlockStorageMode::Empty; }
    bool isFullCover() const { return mode == BlockStorageMode::FullCover; }
    bool isPointArray() const { return mode == BlockStorageMode::PointArray; }

    // 获取整块填充的指针
    InfoType* getFullCover() const {
        return (mode == BlockStorageMode::FullCover) ? data.fullCoverInfo : nullptr;
    }

    // 获取点数组指针
    InfoType** getPointArray() const {
        return (mode == BlockStorageMode::PointArray) ? data.pointArray : nullptr;
    }

    // 设置整块填充（安全：会先清理现有数据）
    void setFullCover(InfoType* info) {
        clear();
        mode = BlockStorageMode::FullCover;
        data.fullCoverInfo = info;
    }

    // 设置点数组（安全：会先清理现有数据）
    void setPointArray(InfoType** arr) {
        clear();
        mode = BlockStorageMode::PointArray;
        data.pointArray = arr;
    }

    // 清理数据（安全的内存释放）
    void clear() {
        if (mode == BlockStorageMode::PointArray && data.pointArray != nullptr) {
            delete[] data.pointArray;
        }
        mode = BlockStorageMode::Empty;
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
    void      setRange(InfoType* info, LONG64 x1, LONG64 z1, LONG64 x2, LONG64 z2, int d); // 范围设置（优化）
    void      clearAll(); // 清理所有空间地图数据

    BigMap<InfoType>* map[20][20][3] = {};
};

// MiddleMap 使用 BlockEntry 实现块级填充存储
// 每个 Block (100×100 坐标) 支持整块填充模式或点数组模式
template <typename InfoType>
class MiddleMap {
public:
    MiddleMap();
    ~MiddleMap();

    // 查询接口
    InfoType* getInfo(int sx, int sz, int ix, int iz);

    // 设置接口
    void setInfo(int sx, int sz, int ix, int iz, InfoType* info);
    void setBlockFullCover(int sx, int sz, InfoType* info); // 设置整块填充

    // 范围设置（优化用）
    void setRange(int x1, int z1, int x2, int z2, InfoType* info);

private:
    static constexpr int SIZE       = 100; // 100×100 个 Block
    static constexpr int BLOCK_SIZE = 100; // 每个 Block 100×100 坐标

    BlockEntry<InfoType>* blocks; // BlockEntry[100×100]
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
    blocks = new BlockEntry<InfoType>[SIZE * SIZE];
    // BlockEntry 默认构造函数会将 data 初始化为 0
}

template <typename InfoType>
MiddleMap<InfoType>::~MiddleMap() {
    // BlockEntry 数组的析构函数会自动调用每个元素的析构函数
    // 这确保了所有点数组都能被安全释放
    delete[] blocks;
}

template <typename InfoType>
InfoType* MiddleMap<InfoType>::getInfo(int sx, int sz, int ix, int iz) {
    // 边界检查
    if (sx < 0 || sx >= SIZE || sz < 0 || sz >= SIZE) return nullptr;
    if (ix < 0 || ix >= BLOCK_SIZE || iz < 0 || iz >= BLOCK_SIZE) return nullptr;

    int                   idx   = sx + sz * SIZE;
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

    return arr[ix + iz * BLOCK_SIZE];
}

template <typename InfoType>
void MiddleMap<InfoType>::setBlockFullCover(int sx, int sz, InfoType* info) {
    // 边界检查
    if (sx < 0 || sx >= SIZE || sz < 0 || sz >= SIZE) return;

    int                   idx   = sx + sz * SIZE;
    BlockEntry<InfoType>& block = blocks[idx];

    // 使用 BlockEntry 的安全设置方法，会自动清理现有数据
    block.setFullCover(info);
}

template <typename InfoType>
void MiddleMap<InfoType>::setInfo(int sx, int sz, int ix, int iz, InfoType* info) {
    // 边界检查
    if (sx < 0 || sx >= SIZE || sz < 0 || sz >= SIZE) return;
    if (ix < 0 || ix >= BLOCK_SIZE || iz < 0 || iz >= BLOCK_SIZE) return;

    int                   idx   = sx + sz * SIZE;
    BlockEntry<InfoType>& block = blocks[idx];

    // 如果当前是整块填充，需要展开为点数组
    if (block.isFullCover()) {
        InfoType*  oldInfo = block.getFullCover();
        InfoType** arr     = new InfoType*[BLOCK_SIZE * BLOCK_SIZE];

        // 安全地初始化点数组：用原来的值填充所有点
        // 注意：这里存储的是相同的指针，但这是安全的，因为：
        // 1. InfoType 对象的生命周期由外部管理
        // 2. 我们不负责释放 InfoType 对象，只释放数组本身
        for (int i = 0; i < BLOCK_SIZE * BLOCK_SIZE; i++) {
            arr[i] = oldInfo;
        }

        // 使用安全的方法设置点数组
        block.setPointArray(arr);
    }

    // 确保点数组存在
    if (block.isEmpty() || block.isFullCover()) {
        // 这种情况理论上不应该发生，但为了安全起见
        InfoType** arr = new InfoType*[BLOCK_SIZE * BLOCK_SIZE];
        for (int i = 0; i < BLOCK_SIZE * BLOCK_SIZE; i++) {
            arr[i] = nullptr;
        }
        block.setPointArray(arr);
    }

    // 设置指定位置的值
    InfoType** arr = block.getPointArray();
    if (arr != nullptr) {
        arr[ix + iz * BLOCK_SIZE] = info;
    }
}

template <typename InfoType>
void MiddleMap<InfoType>::setRange(int x1, int z1, int x2, int z2, InfoType* info) {
    // 边界检查
    if (x1 < 0) x1 = 0;
    if (z1 < 0) z1 = 0;
    if (x2 >= SIZE * BLOCK_SIZE) x2 = SIZE * BLOCK_SIZE - 1;
    if (z2 >= SIZE * BLOCK_SIZE) z2 = SIZE * BLOCK_SIZE - 1;

    // 计算 Block 范围
    int bx1 = x1 / BLOCK_SIZE;
    int bz1 = z1 / BLOCK_SIZE;
    int bx2 = x2 / BLOCK_SIZE;
    int bz2 = z2 / BLOCK_SIZE;

    // 遍历所有涉及的 Block
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
                        setInfo(bx, bz, x, z, info);
                    }
                }
            }
        }
    }
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

    // 坐标范围 [-1000000, 999999] 映射到 [0, 19] 的数组索引
    // bigx = floor((coordx + 1000000) / 100000)
    int bigx = (int)((coordx + 100000 * 10) / 100000);
    int bigz = (int)((coordz + 100000 * 10) / 100000);

    if (bigx >= 20 || bigx < 0 || bigz >= 20 || bigz < 0) return nullptr;

    // 计算当前 BigMap 的基础坐标（与 setRange 保持一致）
    LONG64 bigBaseX = (LONG64)(bigx - 10) * 100000;
    LONG64 bigBaseZ = (LONG64)(bigz - 10) * 100000;

    // 计算 BigMap 内的局部坐标
    LONG64 x = coordx - bigBaseX;
    LONG64 z = coordz - bigBaseZ;

    BigMap<InfoType>* bigMap = this->map[bigx][bigz][d];
    if (bigMap == nullptr) return nullptr;

    int middlex = (int)(x / 10000);
    int middlez = (int)(z / 10000);

    // 边界检查：BigMap 的 size 是 10，所以 middlex 和 middlez 应该在 [0, 9] 范围内
    if (middlex >= 10 || middlex < 0 || middlez >= 10 || middlez < 0) return nullptr;

    x = x - middlex * 10000;
    z = z - middlez * 10000;

    MiddleMap<InfoType>* middleMap = bigMap->getMap(middlex, middlez);
    if (middleMap == nullptr) return nullptr;

    int smallx = (int)(x / 100);
    int smallz = (int)(z / 100);

    // 边界检查：MiddleMap 的 SIZE 是 100，所以 smallx 和 smallz 应该在 [0, 99] 范围内
    if (smallx >= 100 || smallx < 0 || smallz >= 100 || smallz < 0) return nullptr;

    x = x - smallx * 100;
    z = z - smallz * 100;

    // 边界检查：BLOCK_SIZE 是 100，所以 ix 和 iz 应该在 [0, 99] 范围内
    int ix = (int)x;
    int iz = (int)z;
    if (ix >= 100 || ix < 0 || iz >= 100 || iz < 0) return nullptr;

    // 直接从 MiddleMap 获取 InfoType
    return middleMap->getInfo(smallx, smallz, ix, iz);
}

template <typename InfoType>
void SpatialMap<InfoType>::set(InfoType* info, LONG64 xi, LONG64 zi, int d) {
    // 添加维度边界检查
    if (d < 0 || d >= 3) return;

    // 坐标范围 [-1000000, 999999] 映射到 [0, 19] 的数组索引
    // bigx = floor((xi + 1000000) / 100000)
    int bigx = (int)((xi + 100000 * 10) / 100000);
    int bigz = (int)((zi + 100000 * 10) / 100000);

    if (bigx >= 20 || bigx < 0 || bigz >= 20 || bigz < 0) return;

    // 计算当前 BigMap 的基础坐标（与 setRange 保持一致）
    LONG64 bigBaseX = (LONG64)(bigx - 10) * 100000;
    LONG64 bigBaseZ = (LONG64)(bigz - 10) * 100000;

    // 计算 BigMap 内的局部坐标
    LONG64 x = xi - bigBaseX;
    LONG64 z = zi - bigBaseZ;

    BigMap<InfoType>* bigMap = this->map[bigx][bigz][d];
    if (bigMap == nullptr) {
        bigMap                   = new BigMap<InfoType>(bigx, bigz, d);
        this->map[bigx][bigz][d] = bigMap;
    }

    int middlex = (int)(x / 10000);
    int middlez = (int)(z / 10000);

    // 边界检查：BigMap 的 size 是 10，所以 middlex 和 middlez 应该在 [0, 9] 范围内
    if (middlex >= 10 || middlex < 0 || middlez >= 10 || middlez < 0) return;

    x = x - middlex * 10000;
    z = z - middlez * 10000;

    MiddleMap<InfoType>* middleMap = bigMap->getMap(middlex, middlez);
    if (middleMap == nullptr) {
        middleMap = new MiddleMap<InfoType>();
        bigMap->setMap(middlex, middlez, middleMap);
    }

    int smallx = (int)(x / 100);
    int smallz = (int)(z / 100);

    // 边界检查：MiddleMap 的 SIZE 是 100，所以 smallx 和 smallz 应该在 [0, 99] 范围内
    if (smallx >= 100 || smallx < 0 || smallz >= 100 || smallz < 0) return;

    x = x - smallx * 100;
    z = z - smallz * 100;

    // 边界检查：BLOCK_SIZE 是 100，所以 ix 和 iz 应该在 [0, 99] 范围内
    int ix = (int)x;
    int iz = (int)z;
    if (ix >= 100 || ix < 0 || iz >= 100 || iz < 0) return;

    // 直接在 MiddleMap 中设置 InfoType
    middleMap->setInfo(smallx, smallz, ix, iz, info);
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
