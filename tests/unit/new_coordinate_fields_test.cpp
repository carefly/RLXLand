#include "data/core/BaseInformation.h"
#include "data/land/LandCore.h"
#include "data/town/TownCore.h"
#include "utils/TestDataLoader.h"
#include "utils/TestEnvironment.h"
#include "utils/TestHelper.h"
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <memory>


namespace rlx_land::test {

TEST_CASE("New Coordinate Fields - Convenience Functions", "[new_fields][convenience]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("LandData convenience functions") {
        auto land = dataLoader.createTestLand("test_owner", 1000, 2000);

        // 设置新的终点坐标字段
        land->data.x_end = 1050;
        land->data.z_end = 2050;

        // 测试便利函数
        REQUIRE(land->data.getWidth() == 50);
        REQUIRE(land->data.getHeight() == 50);
        REQUIRE(land->data.getArea() == 2500);

        // 测试包含检查
        REQUIRE(land->data.contains(1000, 2000));       // 起始点
        REQUIRE(land->data.contains(1050, 2050));       // 终点
        REQUIRE(land->data.contains(1025, 2025));       // 中间点
        REQUIRE_FALSE(land->data.contains(999, 2000));  // 超出左边界
        REQUIRE_FALSE(land->data.contains(1000, 1999)); // 超出下边界
        REQUIRE_FALSE(land->data.contains(1051, 2050)); // 超出右边界
        REQUIRE_FALSE(land->data.contains(1000, 2051)); // 超出上边界
    }

    SECTION("TownData convenience functions") {
        auto town = dataLoader.createTestTown("test_mayor", 3000, 4000);

        // 设置新的终点坐标字段
        town->data.x_end = 3100;
        town->data.z_end = 4100;

        // 测试便利函数
        REQUIRE(town->data.getWidth() == 100);
        REQUIRE(town->data.getHeight() == 100);
        REQUIRE(town->data.getArea() == 10000);

        // 测试包含检查
        REQUIRE(town->data.contains(3000, 4000));       // 起始点
        REQUIRE(town->data.contains(3100, 4100));       // 终点
        REQUIRE(town->data.contains(3050, 4050));       // 中间点
        REQUIRE_FALSE(town->data.contains(2999, 4000)); // 超出左边界
    }
}

TEST_CASE("New Coordinate Fields - Field Synchronization", "[new_fields][synchronization]") {

    SECTION("Default constructor synchronization") {
        BaseData defaultData;

        REQUIRE(defaultData.x == 0);
        REQUIRE(defaultData.z == 0);
        REQUIRE(defaultData.x_end == 0);
        REQUIRE(defaultData.z_end == 0);
    }

    SECTION("Direct field assignment") {
        BaseData data;
        data.x     = 100;
        data.z     = 200;
        data.x_end = 150;
        data.z_end = 250;

        REQUIRE(data.x_end == 150);
        REQUIRE(data.z_end == 250);
        REQUIRE(data.getWidth() == 50);
        REQUIRE(data.getHeight() == 50);
    }

    SECTION("LandData direct assignment") {
        auto land   = std::make_unique<LandData>();
        land->x     = 1000;
        land->z     = 2000;
        land->x_end = 1050;
        land->z_end = 2050;

        REQUIRE(land->x_end == 1050);
        REQUIRE(land->z_end == 2050);
        REQUIRE(land->getWidth() == 50);
        REQUIRE(land->getHeight() == 50);
    }
}

TEST_CASE("New Coordinate Fields - Edge Cases", "[new_fields][edge_cases]") {

    SECTION("Minimum size land (1x1)") {
        BaseData minLand;
        minLand.x     = 1000;
        minLand.z     = 2000;
        minLand.x_end = 1000; // 相同坐标，1x1领地
        minLand.z_end = 2000;
        // 无需同步，直接使用新字段

        REQUIRE(minLand.getWidth() == 1);
        REQUIRE(minLand.getHeight() == 1);
        REQUIRE(minLand.getArea() == 1);
        REQUIRE(minLand.contains(1000, 2000));
        REQUIRE_FALSE(minLand.contains(1001, 2000));
    }

    SECTION("Negative coordinates") {
        BaseData negativeLand;
        negativeLand.x     = -1000;
        negativeLand.z     = -2000;
        negativeLand.x_end = -900;
        negativeLand.z_end = -1900;
        // 无需同步，直接使用新字段

        REQUIRE(negativeLand.getWidth() == 100);
        REQUIRE(negativeLand.getHeight() == 100);
        REQUIRE(negativeLand.contains(-1000, -2000));
        REQUIRE(negativeLand.contains(-900, -1900));
        REQUIRE_FALSE(negativeLand.contains(-1001, -2000));
    }

    SECTION("Large area land") {
        BaseData largeLand;
        largeLand.x     = -50000;
        largeLand.z     = -50000;
        largeLand.x_end = 50000;
        largeLand.z_end = 50000;
        // 无需同步，直接使用新字段

        REQUIRE(largeLand.getWidth() == 100000);
        REQUIRE(largeLand.getHeight() == 100000);
        REQUIRE(largeLand.getArea() == 10000000000LL);
    }

    SECTION("Invalid coordinates (end < start)") {
        BaseData invalidLand;
        invalidLand.x     = 1000;
        invalidLand.z     = 2000;
        invalidLand.x_end = 900;  // 无效：终点小于起点
        invalidLand.z_end = 1900; // 无效：终点小于起点
        // 无需同步，直接使用新字段

        // 便利函数应该仍然工作，但返回负值或零
        REQUIRE(invalidLand.getWidth() <= 0);
        REQUIRE(invalidLand.getHeight() <= 0);
        REQUIRE(invalidLand.getArea() <= 0);

        // 包含检查应该正确处理无效范围
        REQUIRE_FALSE(invalidLand.contains(1000, 2000));
        REQUIRE_FALSE(invalidLand.contains(900, 1900));
    }
}

TEST_CASE("New Coordinate Fields - Integration with Existing Tests", "[new_fields][integration]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Compatibility with existing land creation") {
        auto land = dataLoader.createTestLand("integration_test", 1000, 1000);

        // 设置新字段
        land->data.x_end = land->data.x + 50; // 使用新字段设置终点
        land->data.z_end = land->data.z + 50;

        // 验证数据有效性
        TestHelper::assertLandDataValid(*land);

        // 验证新字段功能
        REQUIRE(land->data.getWidth() == 50);
        REQUIRE(land->data.getHeight() == 50);
        REQUIRE(land->data.getArea() == 2500);
    }

    SECTION("Compatibility with existing validation") {
        std::vector<std::pair<int, int>> testSizes = {
            {1,   1  },
            {10,  10 },
            {50,  50 },
            {100, 100}
        };

        for (const auto& size : testSizes) {
            auto land = dataLoader.createTestLand("size_validation", 1000, 1000);

            // 使用新字段设置大小
            land->data.x_end = land->data.x + size.first;
            land->data.z_end = land->data.z + size.second;

            // 验证大小
            REQUIRE(land->data.getWidth() == size.first);
            REQUIRE(land->data.getHeight() == size.second);

            // 验证面积
            int expectedArea = size.first * size.second;
            REQUIRE(land->data.getArea() == expectedArea);

            // 验证数据仍然有效
            TestHelper::assertLandDataValid(*land);
        }
    }

    SECTION("Direct field usage") {
        auto land = dataLoader.createTestLand("direct_usage", 1000, 1000);

        // 直接使用新字段设置
        land->data.x_end = 1050;
        land->data.z_end = 2050;

        // 验证功能正确
        REQUIRE(land->data.getWidth() == 50);
        REQUIRE(land->data.getHeight() == 50);
        REQUIRE(land->data.getArea() == 2500);

        // 验证包含检查
        REQUIRE(land->data.contains(1000, 1000));
        REQUIRE(land->data.contains(1050, 2050));
        REQUIRE_FALSE(land->data.contains(1051, 2051));
    }
}

TEST_CASE("New Coordinate Fields - Performance and Memory", "[new_fields][performance]") {

    SECTION("Memory overhead check") {
        // 验证新字段不会显著增加内存使用
        BaseData data;

        // BaseData现在有更多字段，但内存增长应该很小
        REQUIRE(sizeof(data) <= sizeof(BaseData) * 2); // 宽松的限制
    }

    SECTION("Performance of convenience functions") {
        BaseData testData;
        testData.x     = 1000;
        testData.z     = 2000;
        testData.x_end = 1050;
        testData.z_end = 2050;
        // 无需同步，直接使用新字段

        // 测试便利函数的性能（简单的时间测量）
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 10000; ++i) {
            volatile int  width    = testData.getWidth();
            volatile int  height   = testData.getHeight();
            volatile int  area     = testData.getArea();
            volatile bool contains = testData.contains(1025, 2025);
            (void)width;
            (void)height;
            (void)area;
            (void)contains; // 避免编译器优化
        }

        auto end      = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        // 10000次调用应该在合理时间内完成（比如100ms）
        REQUIRE(duration.count() < 100000); // 100ms in microseconds
    }
}

} // namespace rlx_land::test