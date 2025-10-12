#include "data/land/LandCore.h"
#include "mocks/MockPlayer.h"
#include "utils/TestDataLoader.h"
#include "utils/TestEnvironment.h"
#include "utils/TestHelper.h"
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <memory>
#include <vector>


namespace rlx_land::test {

TEST_CASE("Performance Tests - Land Creation", "[performance][creation]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Single Land Creation Performance") {
        TestHelper::measureExecutionTime("Single land creation", [&]() {
            for (int i = 0; i < 1000; ++i) {
                auto land = dataLoader.createTestLand("perf_test_" + std::to_string(i), i, i);
                REQUIRE(land != nullptr);
            }
        });
    }

    SECTION("Batch Land Creation Performance") {
        std::vector<std::unique_ptr<LandInformation>> lands;

        TestHelper::measureExecutionTime("Batch land creation (1000 lands)", [&]() {
            lands = dataLoader.loadPerformanceTestData(1000);
        });

        REQUIRE(lands.size() == 1000);

        // 验证所有土地的数据完整性
        TestHelper::measureExecutionTime("Data integrity verification", [&]() {
            for (const auto& land : lands) {
                TestHelper::assertLandDataValid(*land);
            }
        });
    }

    SECTION("Large Scale Land Creation") {
        const int LARGE_SCALE = 10000;

        TestHelper::performStressTest("Large scale land creation", LARGE_SCALE, [&]() {
            auto land = dataLoader.createTestLand("stress_test", rand() % 10000, rand() % 10000);
            REQUIRE(land != nullptr);
        });
    }

    SECTION("Memory Usage During Creation") {
        TestHelper::trackMemoryUsage();

        std::vector<std::unique_ptr<LandInformation>> lands;
        TestHelper::measureMemoryUsage("Creating 5000 lands", [&]() {
            for (int i = 0; i < 5000; ++i) {
                auto land = dataLoader.createTestLand("memory_test_" + std::to_string(i), i, i);
                lands.push_back(std::move(land));
            }
        });

        TestHelper::reportMemoryUsage();
        REQUIRE(lands.size() == 5000);
    }
}

TEST_CASE("Performance Tests - Permission Checking", "[performance][permission]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Permission Check Performance") {
        auto land = dataLoader.createTestLand("permission_test", 1000, 1000);

        // 添加大量成员
        const int MEMBER_COUNT = 1000;
        for (int i = 0; i < MEMBER_COUNT; ++i) {
            land->data.memberXuids.push_back("member_" + std::to_string(i));
        }

        // 测试权限检查性能
        TestHelper::measureExecutionTime("Permission checks for " + std::to_string(MEMBER_COUNT) + " members", [&]() {
            for (int i = 0; i < MEMBER_COUNT; ++i) {
                REQUIRE(land->hasBasicPermission("member_" + std::to_string(i)));
            }
        });

        // 测试所有者权限检查性能
        TestHelper::measureExecutionTime("Owner permission checks (10000 iterations)", [&]() {
            for (int i = 0; i < 10000; ++i) {
                REQUIRE(land->hasBasicPermission(land->ld.ownerXuid));
                REQUIRE(land->isOwner(land->ld.ownerXuid));
            }
        });
    }

    SECTION("Large Member List Performance") {
        auto land = dataLoader.createTestLand("large_member_test", 1000, 1000);

        // 添加大量成员
        const int LARGE_MEMBER_COUNT = 10000;
        for (int i = 0; i < LARGE_MEMBER_COUNT; ++i) {
            land->data.memberXuids.push_back("large_member_" + std::to_string(i));
        }

        // 测试大数据量下的权限检查
        TestHelper::performStressTest("Permission checks with large member list", 100, [&]() {
            for (int i = 0; i < LARGE_MEMBER_COUNT; ++i) {
                land->hasBasicPermission("large_member_" + std::to_string(i));
            }
        });
    }

    SECTION("Non-member Permission Check Performance") {
        auto land = dataLoader.createTestLand("non_member_test", 1000, 1000);

        // 测试检查非成员权限的性能
        TestHelper::measureExecutionTime("Non-member permission checks (10000 iterations)", [&]() {
            for (int i = 0; i < 10000; ++i) {
                REQUIRE_FALSE(land->hasBasicPermission("non_existent_member_" + std::to_string(i)));
            }
        });
    }
}

TEST_CASE("Performance Tests - Data Loading", "[performance][loading]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Test Data Loading Performance") {
        TestHelper::measureExecutionTime("Loading basic test data", [&]() {
            auto lands = dataLoader.loadLandTestData();
            REQUIRE(!lands.empty());
        });

        TestHelper::measureExecutionTime("Loading boundary test data", [&]() {
            auto lands = dataLoader.loadBoundaryTestData();
            REQUIRE(!lands.empty());
        });

        TestHelper::measureExecutionTime("Loading invalid test data", [&]() {
            auto lands = dataLoader.loadInvalidTestData();
            REQUIRE(!lands.empty());
        });
    }

    SECTION("Performance Data Loading") {
        const std::vector<int> testSizes = {100, 500, 1000, 5000};

        for (int size : testSizes) {
            TestHelper::measureExecutionTime("Loading " + std::to_string(size) + " performance test lands", [&]() {
                auto lands = dataLoader.loadPerformanceTestData(size);
                REQUIRE(lands.size() == size);
            });
        }
    }

    SECTION("Repeated Loading Performance") {
        const int ITERATIONS = 100;

        TestHelper::performStressTest("Repeated data loading", ITERATIONS, [&]() {
            auto lands = dataLoader.loadLandTestData();
            REQUIRE(!lands.empty());
            dataLoader.cleanupTestData();
        });
    }
}

TEST_CASE("Performance Tests - Memory Management", "[performance][memory]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Memory Allocation and Deallocation") {
        TestHelper::trackMemoryUsage();

        // 大量创建和销毁土地对象
        TestHelper::performStressTest("Memory allocation/deallocation", 1000, [&]() {
            {
                auto land = dataLoader.createTestLand("memory_cycle_test", rand() % 1000, rand() % 1000);
                REQUIRE(land != nullptr);
                // land会在作用域结束时自动销毁
            }
        });

        TestHelper::reportMemoryUsage();
    }

    SECTION("Large Data Structure Memory Usage") {
        TestHelper::trackMemoryUsage();

        std::vector<std::unique_ptr<LandInformation>> largeLandSet;

        // 创建大量土地对象
        TestHelper::measureMemoryUsage("Creating large land set", [&]() {
            for (int i = 0; i < 10000; ++i) {
                auto land = dataLoader.createTestLand("large_set_test_" + std::to_string(i), i, i);

                // 为每个土地添加大量成员
                for (int j = 0; j < 100; ++j) {
                    land->data.memberXuids.push_back("member_" + std::to_string(i) + "_" + std::to_string(j));
                }

                largeLandSet.push_back(std::move(land));
            }
        });

        TestHelper::reportMemoryUsage();
        REQUIRE(largeLandSet.size() == 10000);

        // 清理内存
        TestHelper::measureMemoryUsage("Cleaning up large land set", [&]() { largeLandSet.clear(); });

        TestHelper::reportMemoryUsage();
    }

    SECTION("Memory Leak Detection") {
        TestHelper::trackMemoryUsage();

        // 模拟可能导致内存泄漏的操作
        for (int cycle = 0; cycle < 10; ++cycle) {
            std::vector<std::unique_ptr<LandInformation>> lands;

            // 创建大量土地
            for (int i = 0; i < 1000; ++i) {
                auto land =
                    dataLoader.createTestLand("leak_test_" + std::to_string(cycle) + "_" + std::to_string(i), i, i);
                lands.push_back(std::move(land));
            }

            // lands在作用域结束时应该自动清理
        }

        TestHelper::reportMemoryUsage();
    }
}

TEST_CASE("Performance Tests - String Operations", "[performance][strings]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Member Name Generation Performance") {
        auto land = dataLoader.createTestLand("string_test", 1000, 1000);

        // 添加大量成员
        const int MEMBER_COUNT = 5000;
        for (int i = 0; i < MEMBER_COUNT; ++i) {
            land->data.memberXuids.push_back("string_member_" + std::to_string(i));
        }

        // 测试成员名称字符串生成性能
        TestHelper::measureExecutionTime(
            "Member name generation for " + std::to_string(MEMBER_COUNT) + " members",
            [&]() {
                std::string memberNames = land->getMembers();
                REQUIRE(!memberNames.empty());
            }
        );
    }

    SECTION("Long String Handling Performance") {
        auto land = dataLoader.createTestLand("long_string_test", 1000, 1000);

        // 创建长描述
        std::string longDescription = TestHelper::generateLongString(10000);
        land->data.description      = longDescription;

        // 测试长字符串操作性能
        TestHelper::measureExecutionTime("Long string operations", [&]() {
            for (int i = 0; i < 1000; ++i) {
                std::string desc = land->data.description;
                REQUIRE(desc.length() == 10000);
            }
        });
    }

    SECTION("String Comparison Performance") {
        auto land = dataLoader.createTestLand("string_compare_test", 1000, 1000);

        // 添加大量成员
        const int MEMBER_COUNT = 1000;
        for (int i = 0; i < MEMBER_COUNT; ++i) {
            land->data.memberXuids.push_back("compare_member_" + std::to_string(i));
        }

        // 测试字符串比较性能
        TestHelper::measureExecutionTime("String comparisons for " + std::to_string(MEMBER_COUNT) + " members", [&]() {
            for (int i = 0; i < MEMBER_COUNT; ++i) {
                land->hasBasicPermission("compare_member_" + std::to_string(i));
            }
        });
    }
}

TEST_CASE("Performance Tests - Boundary Performance", "[performance][boundary]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Boundary Value Performance") {
        auto boundaryValues = TestHelper::getBoundaryValues();

        // 测试边界值操作的性能
        TestHelper::measureExecutionTime("Boundary value operations", [&]() {
            for (int i = 0; i < 10000; ++i) {
                REQUIRE(TestHelper::isValidCoordinate(boundaryValues.minCoordinate));
                REQUIRE(TestHelper::isValidCoordinate(boundaryValues.maxCoordinate));
                REQUIRE(TestHelper::isValidSize(boundaryValues.minSize));
                REQUIRE(TestHelper::isValidSize(boundaryValues.maxSize));
                REQUIRE(TestHelper::isValidPermission(boundaryValues.minPermission));
                REQUIRE(TestHelper::isValidPermission(boundaryValues.maxPermission));
            }
        });
    }

    SECTION("Boundary Land Creation Performance") {
        TestHelper::measureExecutionTime("Boundary land creation", [&]() {
            for (int i = 0; i < 1000; ++i) {
                auto land = dataLoader.createBoundaryTestLand(
                    "boundary_perf_" + std::to_string(i),
                    -30000000 + i,
                    -30000000 + i,
                    1,
                    1
                );
                REQUIRE(land != nullptr);
            }
        });
    }
}

TEST_CASE("Performance Tests - stress tests for DataLoader", "[performance][dataloader]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("DataLoader Stress Test") {
        TestHelper::performStressTest("DataLoader operations", 1000, [&]() {
            auto land =
                dataLoader.createTestLand("dataloader_stress_" + std::to_string(rand()), rand() % 1000, rand() % 1000);
            REQUIRE(land != nullptr);
        });
    }

    SECTION("Maximum ID Generation Performance") {
        TestHelper::measureExecutionTime("Maximum ID generation (1000 iterations)", [&]() {
            for (int i = 0; i < 1000; ++i) {
                LONG64 maxId = dataLoader.getMaxId();
                REQUIRE(maxId >= 0);
            }
        });
    }

    SECTION("Data Validation Performance") {
        auto validLands = dataLoader.loadPerformanceTestData(1000);

        TestHelper::measureExecutionTime("Data validation for 1000 lands", [&]() {
            bool isValid = dataLoader.validateTestData(validLands);
            REQUIRE(isValid);
        });
    }
}

} // namespace rlx_land::test