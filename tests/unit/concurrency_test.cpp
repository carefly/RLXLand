#include "data/land/LandCore.h"
#include "utils/TestDataLoader.h"
#include "utils/TestEnvironment.h"
#include "utils/TestHelper.h"
#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <set>
#include <vector>


namespace rlx_land::test {

TEST_CASE("Concurrency Tests - Thread Safe Land Creation", "[concurrency][creation]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Concurrent Land Creation") {
        const int NUM_THREADS      = 10;
        const int LANDS_PER_THREAD = 100;

        std::vector<std::unique_ptr<LandInformation>> allLands;
        std::mutex                                    landsMutex;
        std::atomic<int>                              totalCreated(0);

        TestHelper::runConcurrentTest("Concurrent land creation", NUM_THREADS, [&](int threadId) {
            std::vector<std::unique_ptr<LandInformation>> threadLands;

            for (int i = 0; i < LANDS_PER_THREAD; ++i) {
                auto land = dataLoader.createTestLand(
                    "thread_" + std::to_string(threadId) + "_land_" + std::to_string(i),
                    threadId * 1000 + i,
                    threadId * 1000 + i
                );

                REQUIRE(land != nullptr);
                threadLands.push_back(std::move(land));
                totalCreated++;
            }

            // 线程安全地添加到全局列表
            std::lock_guard<std::mutex> lock(landsMutex);
            for (auto& land : threadLands) {
                allLands.push_back(std::move(land));
            }
        });

        REQUIRE(totalCreated.load() == NUM_THREADS * LANDS_PER_THREAD);
        REQUIRE(allLands.size() == NUM_THREADS * LANDS_PER_THREAD);

        // 验证所有土地都有唯一的ID
        std::set<LONG64> uniqueIds;
        for (const auto& land : allLands) {
            uniqueIds.insert(land->data.id);
        }
        REQUIRE(uniqueIds.size() == allLands.size());
    }

    SECTION("High Frequency Concurrent Creation") {
        const int NUM_THREADS           = 20;
        const int OPERATIONS_PER_THREAD = 1000;

        std::atomic<int> successCount(0);
        std::atomic<int> errorCount(0);

        TestHelper::runConcurrentTest("High frequency concurrent creation", NUM_THREADS, [&](int threadId) {
            for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
                try {
                    auto land = dataLoader.createTestLand(
                        "high_freq_" + std::to_string(threadId) + "_" + std::to_string(i),
                        threadId * 10000 + i,
                        threadId * 10000 + i
                    );

                    if (land != nullptr) {
                        successCount++;
                    } else {
                        errorCount++;
                    }
                } catch (...) {
                    errorCount++;
                }
            }
        });

        REQUIRE(successCount.load() == NUM_THREADS * OPERATIONS_PER_THREAD);
        REQUIRE(errorCount.load() == 0);
    }
}

TEST_CASE("Concurrency Tests - Thread Safe Permission Checking", "[concurrency][permission]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Concurrent Permission Checks") {
        auto land = dataLoader.createTestLand("concurrent_permission_test", 1000, 1000);

        // 添加大量成员
        const int MEMBER_COUNT = 1000;
        for (int i = 0; i < MEMBER_COUNT; ++i) {
            land->data.memberXuids.push_back("member_" + std::to_string(i));
        }

        const int NUM_THREADS       = 10;
        const int CHECKS_PER_THREAD = 1000;

        std::atomic<int> totalChecks(0);
        std::atomic<int> successfulChecks(0);

        TestHelper::runConcurrentTest("Concurrent permission checks", NUM_THREADS, [&](int) {
            std::random_device              rd;
            std::mt19937                    gen(rd());
            std::uniform_int_distribution<> memberDist(0, MEMBER_COUNT - 1);

            for (int i = 0; i < CHECKS_PER_THREAD; ++i) {
                // 检查随机成员的权限
                std::string memberXuid    = "member_" + std::to_string(memberDist(gen));
                bool        hasPermission = land->hasBasicPermission(memberXuid);

                totalChecks++;
                if (hasPermission) {
                    successfulChecks++;
                }

                // 检查所有者权限
                REQUIRE(land->hasBasicPermission(land->ld.ownerXuid));
                REQUIRE(land->isOwner(land->ld.ownerXuid));
            }
        });

        REQUIRE(totalChecks.load() == NUM_THREADS * CHECKS_PER_THREAD);
        // 所有成员检查都应该成功
        REQUIRE(successfulChecks.load() == NUM_THREADS * CHECKS_PER_THREAD);
    }

    SECTION("Concurrent Member Modification") {
        auto land = dataLoader.createTestLand("concurrent_member_test", 1000, 1000);

        const int NUM_THREADS        = 5;
        const int MEMBERS_PER_THREAD = 200;

        std::atomic<int> totalAdded(0);
        std::mutex       memberMutex;

        TestHelper::runConcurrentTest("Concurrent member modification", NUM_THREADS, [&](int threadId) {
            for (int i = 0; i < MEMBERS_PER_THREAD; ++i) {
                std::string memberXuid = "thread_" + std::to_string(threadId) + "_member_" + std::to_string(i);

                // 线程安全地添加成员
                {
                    std::lock_guard<std::mutex> lock(memberMutex);
                    land->data.memberXuids.push_back(memberXuid);
                    totalAdded++;
                }

                // 验证成员已添加
                REQUIRE(land->hasBasicPermission(memberXuid));
            }
        });

        REQUIRE(totalAdded.load() == NUM_THREADS * MEMBERS_PER_THREAD);
        REQUIRE(land->data.memberXuids.size() == NUM_THREADS * MEMBERS_PER_THREAD);
    }
}

TEST_CASE("Concurrency Tests - Thread Safe Data Loading", "[concurrency][loading]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Concurrent Data Loading") {
        const int NUM_THREADS      = 10;
        const int LOADS_PER_THREAD = 50;

        std::atomic<int> totalLoads(0);
        std::atomic<int> successfulLoads(0);

        TestHelper::runConcurrentTest("Concurrent data loading", NUM_THREADS, [&](int) {
            for (int i = 0; i < LOADS_PER_THREAD; ++i) {
                try {
                    auto lands = dataLoader.loadLandTestData();
                    if (!lands.empty()) {
                        successfulLoads++;
                    }
                    totalLoads++;
                } catch (...) {
                    totalLoads++;
                }
            }
        });

        REQUIRE(totalLoads.load() == NUM_THREADS * LOADS_PER_THREAD);
        REQUIRE(successfulLoads.load() == NUM_THREADS * LOADS_PER_THREAD);
    }

    SECTION("Concurrent Performance Data Loading") {
        const int NUM_THREADS = 5;
        const int DATA_SIZE   = 500;

        std::atomic<int>                                           totalLoaded(0);
        std::vector<std::vector<std::unique_ptr<LandInformation>>> allData;
        std::mutex                                                 dataMutex;

        TestHelper::runConcurrentTest("Concurrent performance data loading", NUM_THREADS, [&](int) {
            auto lands = dataLoader.loadPerformanceTestData(DATA_SIZE);

            if (lands.size() == DATA_SIZE) {
                totalLoaded++;

                std::lock_guard<std::mutex> lock(dataMutex);
                allData.push_back(std::move(lands));
            }
        });

        REQUIRE(totalLoaded.load() == NUM_THREADS);
        REQUIRE(allData.size() == NUM_THREADS);

        // 验证数据的完整性
        for (const auto& landSet : allData) {
            REQUIRE(landSet.size() == DATA_SIZE);
            for (const auto& land : landSet) {
                TestHelper::assertLandDataValid(*land);
            }
        }
    }
}

TEST_CASE("Concurrency Tests - Race Condition Detection", "[concurrency][race]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Shared Resource Access") {
        auto sharedLand = dataLoader.createTestLand("shared_resource_test", 1000, 1000);

        const int NUM_THREADS           = 10;
        const int OPERATIONS_PER_THREAD = 1000;

        std::atomic<int> readOperations(0);
        std::atomic<int> writeOperations(0);
        std::atomic<int> errors(0);

        TestHelper::runConcurrentTest("Shared resource access", NUM_THREADS, [&](int threadId) {
            for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
                try {
                    if (i % 2 == 0) {
                        // 读操作
                        bool hasPermission = sharedLand->hasBasicPermission(sharedLand->ld.ownerXuid);
                        bool isOwner       = sharedLand->isOwner(sharedLand->ld.ownerXuid);
                        readOperations++;

                        // 验证数据一致性
                        REQUIRE(hasPermission);
                        REQUIRE(isOwner);
                    } else {
                        // 写操作（添加成员）
                        std::string memberXuid = "thread_" + std::to_string(threadId) + "_member_" + std::to_string(i);
                        sharedLand->data.memberXuids.push_back(memberXuid);
                        writeOperations++;
                    }
                } catch (...) {
                    errors++;
                }
            }
        });

        REQUIRE(readOperations.load() > 0);
        REQUIRE(writeOperations.load() > 0);
        REQUIRE(errors.load() == 0);
    }

    SECTION("ID Generation Race Condition") {
        const int NUM_THREADS          = 20;
        const int CREATIONS_PER_THREAD = 100;

        std::vector<std::unique_ptr<LandInformation>> allLands;
        std::mutex                                    landsMutex;
        std::atomic<int>                              totalCreated(0);
        std::atomic<int>                              duplicateIds(0);

        TestHelper::runConcurrentTest("ID generation race condition", NUM_THREADS, [&](int threadId) {
            std::vector<std::unique_ptr<LandInformation>> threadLands;

            for (int i = 0; i < CREATIONS_PER_THREAD; ++i) {
                auto land = dataLoader.createTestLand(
                    "race_test_" + std::to_string(threadId) + "_" + std::to_string(i),
                    threadId * 1000 + i,
                    threadId * 1000 + i
                );

                if (land != nullptr) {
                    threadLands.push_back(std::move(land));
                    totalCreated++;
                }
            }

            // 检查重复ID
            std::set<LONG64> threadIds;
            for (const auto& land : threadLands) {
                if (threadIds.count(land->data.id) > 0) {
                    duplicateIds++;
                }
                threadIds.insert(land->data.id);
            }

            // 添加到全局列表
            std::lock_guard<std::mutex> lock(landsMutex);
            for (auto& land : threadLands) {
                allLands.push_back(std::move(land));
            }
        });

        REQUIRE(totalCreated.load() == NUM_THREADS * CREATIONS_PER_THREAD);
        REQUIRE(duplicateIds.load() == 0);

        // 全局重复ID检查
        std::set<LONG64> globalIds;
        for (const auto& land : allLands) {
            REQUIRE(globalIds.count(land->data.id) == 0);
            globalIds.insert(land->data.id);
        }
    }
}

TEST_CASE("Concurrency Tests - Deadlock Prevention", "[concurrency][deadlock]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Multiple Resource Access") {
        auto land1 = dataLoader.createTestLand("deadlock_test_1", 1000, 1000);
        auto land2 = dataLoader.createTestLand("deadlock_test_2", 2000, 2000);

        const int NUM_THREADS           = 10;
        const int OPERATIONS_PER_THREAD = 100;

        std::atomic<int> completedOperations(0);
        std::atomic<int> timeouts(0);

        TestHelper::runConcurrentTest("Multiple resource access", NUM_THREADS, [&](int threadId) {
            for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
                try {
                    // 使用超时机制防止死锁
                    auto start = std::chrono::steady_clock::now();

                    // 交替访问两个资源
                    if (threadId % 2 == 0) {
                        land1->hasBasicPermission(land1->ld.ownerXuid);
                        land2->hasBasicPermission(land2->ld.ownerXuid);
                    } else {
                        land2->hasBasicPermission(land2->ld.ownerXuid);
                        land1->hasBasicPermission(land1->ld.ownerXuid);
                    }

                    auto end      = std::chrono::steady_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

                    // 如果操作时间过长，可能发生了死锁
                    if (duration.count() > 1000) { // 1秒超时
                        timeouts++;
                    } else {
                        completedOperations++;
                    }
                } catch (...) {
                    // 异常也可能表明死锁或其他并发问题
                }
            }
        });

        REQUIRE(completedOperations.load() > 0);
        REQUIRE(timeouts.load() == 0); // 不应该有超时
    }

    SECTION("Nested Resource Access") {
        const int                                     NUM_LANDS = 5;
        std::vector<std::unique_ptr<LandInformation>> lands;

        for (int i = 0; i < NUM_LANDS; ++i) {
            lands.push_back(dataLoader.createTestLand("nested_test_" + std::to_string(i), i * 1000, i * 1000));
        }

        const int        NUM_THREADS = 5;
        std::atomic<int> successfulAccesses(0);

        TestHelper::runConcurrentTest("Nested resource access", NUM_THREADS, [&](int threadId) {
            for (int i = 0; i < 100; ++i) {
                try {
                    // 按固定顺序访问资源以避免死锁
                    for (int j = 0; j < NUM_LANDS; ++j) {
                        bool hasPermission = lands[j]->hasBasicPermission(lands[j]->ld.ownerXuid);
                        REQUIRE(hasPermission);
                    }
                    successfulAccesses++;
                } catch (...) {
                    // 记录异常但继续测试
                }
            }
        });

        REQUIRE(successfulAccesses.load() > 0);
    }
}

TEST_CASE("Concurrency Tests - Performance Under Load", "[concurrency][performance]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("High Concurrency Performance") {
        const int NUM_THREADS           = 50;
        const int OPERATIONS_PER_THREAD = 200;

        std::atomic<int> totalOperations(0);
        auto             startTime = std::chrono::high_resolution_clock::now();

        TestHelper::runConcurrentTest("High concurrency performance", NUM_THREADS, [&](int threadId) {
            for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
                auto land = dataLoader.createTestLand(
                    "perf_test_" + std::to_string(threadId) + "_" + std::to_string(i),
                    threadId * 10000 + i,
                    threadId * 10000 + i
                );

                if (land != nullptr) {
                    totalOperations++;

                    // 执行一些基本操作
                    land->hasBasicPermission(land->ld.ownerXuid);
                    land->isOwner(land->ld.ownerXuid);
                }
            }
        });

        auto endTime  = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        REQUIRE(totalOperations.load() == NUM_THREADS * OPERATIONS_PER_THREAD);

        // 计算吞吐量
        double throughput = (double)totalOperations.load() / duration.count() * 1000; // operations per second
        std::cout << "[CONCURRENCY] Throughput: " << throughput << " ops/sec" << std::endl;

        // 性能应该合理（这里设置一个基本阈值）
        REQUIRE(throughput > 100); // 至少100 ops/sec
    }

    SECTION("Memory Usage Under Concurrency") {
        TestHelper::trackMemoryUsage();

        const int NUM_THREADS        = 20;
        const int OBJECTS_PER_THREAD = 500;

        std::vector<std::vector<std::unique_ptr<LandInformation>>> threadData(NUM_THREADS);
        std::atomic<int>                                           totalCreated(0);

        TestHelper::runConcurrentTest("Memory usage under concurrency", NUM_THREADS, [&](int threadId) {
            for (int i = 0; i < OBJECTS_PER_THREAD; ++i) {
                auto land = dataLoader.createTestLand(
                    "memory_test_" + std::to_string(threadId) + "_" + std::to_string(i),
                    threadId * 10000 + i,
                    threadId * 10000 + i
                );

                if (land != nullptr) {
                    threadData[threadId].push_back(std::move(land));
                    totalCreated++;
                }
            }
        });

        TestHelper::reportMemoryUsage();
        REQUIRE(totalCreated.load() == NUM_THREADS * OBJECTS_PER_THREAD);

        // 验证所有对象都有效
        for (const auto& threadObjects : threadData) {
            for (const auto& land : threadObjects) {
                REQUIRE(land != nullptr);
                TestHelper::assertLandDataValid(*land);
            }
        }
    }
}

} // namespace rlx_land::test