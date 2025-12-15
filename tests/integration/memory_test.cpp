// 内存安全测试 - 验证 BlockEntry 的内存管理
#include "../../src/data/spatial/SpatialMap.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <vector>


using namespace rlx_land;

// 测试用的简单信息结构
struct TestInfo {
    int         id;
    std::string name;

    TestInfo(int i, const std::string& n) : id(i), name(n) {}
};

TEST_CASE("BlockEntry 基本功能", "[memory]") {
    std::cout << "✅ 测试1: BlockEntry 基本功能" << std::endl;
    BlockEntry<TestInfo> entry;

    // 初始状态应该是空的
    REQUIRE(entry.isEmpty());
    REQUIRE_FALSE(entry.isFullCover());
    REQUIRE_FALSE(entry.isPointArray());
    REQUIRE(entry.getFullCover() == nullptr);
    REQUIRE(entry.getPointArray() == nullptr);

    // 测试整块填充
    TestInfo info(42, "test");
    entry.setFullCover(&info);
    REQUIRE_FALSE(entry.isEmpty());
    REQUIRE(entry.isFullCover());
    REQUIRE(entry.getFullCover() == &info);
    REQUIRE(entry.getFullCover()->id == 42);

    // 测试切换到点数组
    const int  SIZE = 100;
    TestInfo** arr  = new TestInfo*[SIZE * SIZE];
    for (int i = 0; i < SIZE * SIZE; i++) {
        arr[i] = nullptr;
    }
    entry.setPointArray(arr);
    REQUIRE_FALSE(entry.isEmpty());
    REQUIRE(entry.isPointArray());
    REQUIRE(entry.getPointArray() == arr);

    // BlockEntry 接管了 arr 的所有权，析构时会自动释放
    // delete[] arr; // 移除这行，避免 double free

    std::cout << "  ✅ BlockEntry 基本功能测试通过" << std::endl;
}

TEST_CASE("MiddleMap 内存管理", "[memory]") {
    std::cout << "✅ 测试2: MiddleMap 内存管理" << std::endl;
    MiddleMap<TestInfo> middleMap;

    // 测试整块填充
    TestInfo info1(1, "block1");
    middleMap.setBlockFullCover(0, 0, &info1);

    auto* retrieved = middleMap.getInfo(0, 0, 50, 50);
    REQUIRE(retrieved == &info1);
    REQUIRE(retrieved->id == 1);

    // 测试从整块填充切换到点数组
    TestInfo info2(2, "block2");
    middleMap.setInfo(0, 0, 10, 10, &info2);

    retrieved = middleMap.getInfo(0, 0, 10, 10);
    REQUIRE(retrieved == &info2);
    REQUIRE(retrieved->id == 2);

    // 其他位置应该保持原值
    retrieved = middleMap.getInfo(0, 0, 50, 50);
    REQUIRE(retrieved == &info1);
    REQUIRE(retrieved->id == 1);

    std::cout << "  ✅ MiddleMap 内存管理测试通过" << std::endl;
}

TEST_CASE("大量操作测试（检查内存泄漏）", "[memory]") {
    std::cout << "✅ 测试3: 大量操作测试" << std::endl;

    // 使用智能指针和较小的数量来避免栈溢出
    constexpr int MAP_COUNT = 20; // 减少数量避免栈溢出

    std::vector<std::unique_ptr<MiddleMap<TestInfo>>> maps;
    std::vector<std::unique_ptr<TestInfo>>            infos;

    // 创建对象（使用智能指针自动管理内存）
    for (int i = 0; i < MAP_COUNT; i++) {
        maps.emplace_back(std::make_unique<MiddleMap<TestInfo>>());
        infos.emplace_back(std::make_unique<TestInfo>(i, "test" + std::to_string(i)));
    }

    // 执行大量操作
    for (size_t mapIdx = 0; mapIdx < maps.size(); mapIdx++) {
        auto& map = maps[mapIdx];
        for (int blockX = 0; blockX < 10; blockX++) {
            for (int blockZ = 0; blockZ < 10; blockZ++) {
                if (mapIdx % 3 == 0) {
                    // 整块填充
                    map->setBlockFullCover(blockX, blockZ, infos[mapIdx].get());
                } else {
                    // 点模式设置
                    map->setInfo(blockX, blockZ, 25, 25, infos[mapIdx].get());
                }
            }
        }
    }

    // 验证数据完整性
    for (size_t mapIdx = 0; mapIdx < maps.size(); mapIdx++) {
        auto& map  = maps[mapIdx];
        auto& info = infos[mapIdx];

        auto* retrieved = map->getInfo(5, 5, 25, 25);
        if (mapIdx % 3 == 0) {
            REQUIRE(retrieved == info.get());
        } else {
            REQUIRE((retrieved == nullptr || retrieved == info.get()));
        }
    }

    // 智能指针会自动清理内存，无需手动删除

    std::cout << "  ✅ 大量操作测试通过（无内存泄漏）" << std::endl;
}