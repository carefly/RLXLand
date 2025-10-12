#include "data/land/LandCore.h"
#include "mocks/MockPlayer.h"
#include "utils/TestDataLoader.h"
#include "utils/TestEnvironment.h"
#include "utils/TestHelper.h"
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <vector>


namespace rlx_land::test {

TEST_CASE("Boundary Value Tests - Land Coordinates", "[boundary][land][coordinates]") {
    TestDataLoader& dataLoader     = TestDataLoader::getInstance();
    auto            boundaryValues = TestHelper::getBoundaryValues();

    SECTION("Minimum Coordinate Boundaries") {
        auto land = dataLoader.createBoundaryTestLand(
            "min_coord_test",
            boundaryValues.minCoordinate,
            boundaryValues.minCoordinate,
            50,
            50
        );

        REQUIRE(land != nullptr);
        REQUIRE(land->data.x == boundaryValues.minCoordinate);
        REQUIRE(land->data.z == boundaryValues.minCoordinate);
        TestHelper::assertLandDataValid(*land);
    }

    SECTION("Maximum Coordinate Boundaries") {
        auto land = dataLoader.createBoundaryTestLand(
            "max_coord_test",
            boundaryValues.maxCoordinate,
            boundaryValues.maxCoordinate,
            50,
            50
        );

        REQUIRE(land != nullptr);
        REQUIRE(land->data.x == boundaryValues.maxCoordinate);
        REQUIRE(land->data.z == boundaryValues.maxCoordinate);
        TestHelper::assertLandDataValid(*land);
    }

    SECTION("Out of Bounds Coordinates") {
        // 超出最大边界
        auto land1 = dataLoader.createInvalidTestLand("out_of_bounds_1", boundaryValues.maxCoordinate + 1, 0, 50, 50);
        TestHelper::assertLandDataInvalid(*land1);

        // 超出最小边界
        auto land2 = dataLoader.createInvalidTestLand("out_of_bounds_2", boundaryValues.minCoordinate - 1, 0, 50, 50);
        TestHelper::assertLandDataInvalid(*land2);
    }
}

TEST_CASE("Boundary Value Tests - Land Size", "[boundary][land][size]") {
    TestDataLoader& dataLoader     = TestDataLoader::getInstance();
    auto            boundaryValues = TestHelper::getBoundaryValues();

    SECTION("Minimum Size Boundaries") {
        auto land =
            dataLoader
                .createBoundaryTestLand("min_size_test", 1000, 1000, boundaryValues.minSize, boundaryValues.minSize);

        REQUIRE(land != nullptr);
        REQUIRE(land->data.getWidth() == boundaryValues.minSize);
        REQUIRE(land->data.getHeight() == boundaryValues.minSize);
        TestHelper::assertLandDataValid(*land);
    }

    SECTION("Maximum Size Boundaries") {
        auto land =
            dataLoader
                .createBoundaryTestLand("max_size_test", 1000, 1000, boundaryValues.maxSize, boundaryValues.maxSize);

        REQUIRE(land != nullptr);
        REQUIRE(land->data.getWidth() == boundaryValues.maxSize);
        REQUIRE(land->data.getHeight() == boundaryValues.maxSize);
        TestHelper::assertLandDataValid(*land);
    }

    SECTION("Invalid Size Values") {
        // 零大小
        auto land1 = dataLoader.createInvalidTestLand("zero_size", 1000, 1000, 0, 50);
        TestHelper::assertLandDataInvalid(*land1);

        // 负数大小
        auto land2 = dataLoader.createInvalidTestLand("negative_size", 1000, 1000, -1, 50);
        TestHelper::assertLandDataInvalid(*land2);
    }
}

TEST_CASE("Boundary Value Tests - Permission Levels", "[boundary][land][permission]") {
    TestDataLoader& dataLoader     = TestDataLoader::getInstance();
    auto            boundaryValues = TestHelper::getBoundaryValues();

    SECTION("Minimum Permission Boundary") {
        auto land       = dataLoader.createBoundaryTestLand("min_perm_test", 1000, 1000, 50, 50);
        land->data.perm = boundaryValues.minPermission;

        REQUIRE(land != nullptr);
        REQUIRE(land->data.perm == boundaryValues.minPermission);
    }

    SECTION("Maximum Permission Boundary") {
        auto land       = dataLoader.createBoundaryTestLand("max_perm_test", 1000, 1000, 50, 50);
        land->data.perm = boundaryValues.maxPermission;

        REQUIRE(land != nullptr);
        REQUIRE(land->data.perm == boundaryValues.maxPermission);
    }

    SECTION("Invalid Permission Values") {
        // 负数权限
        auto land1       = dataLoader.createInvalidTestLand("negative_perm", 1000, 1000, 50, 50);
        land1->data.perm = -1;
        REQUIRE_FALSE(TestHelper::isValidPermission(land1->data.perm));

        // 超出最大权限
        auto land2       = dataLoader.createInvalidTestLand("oversize_perm", 1000, 1000, 50, 50);
        land2->data.perm = boundaryValues.maxPermission + 1;
        REQUIRE_FALSE(TestHelper::isValidPermission(land2->data.perm));
    }
}

TEST_CASE("Boundary Value Tests - Member Lists", "[boundary][land][members]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Empty Member List") {
        auto land = dataLoader.createTestLand("empty_members_test", 1000, 1000);
        REQUIRE(land != nullptr);
        REQUIRE(land->data.memberXuids.empty());

        // 测试权限检查 - 空成员列表应该只有所有者有权限
        REQUIRE(land->hasBasicPermission(land->ld.ownerXuid));
        REQUIRE_FALSE(land->hasBasicPermission("non_existent_member"));
    }

    SECTION("Single Member List") {
        auto land = dataLoader.createTestLand("single_member_test", 1000, 1000);
        land->data.memberXuids.push_back("single_member");

        REQUIRE(land->data.memberXuids.size() == 1);
        REQUIRE(land->hasBasicPermission("single_member"));
    }

    SECTION("Maximum Member List") {
        auto land = dataLoader.createTestLand("max_members_test", 1000, 1000);

        // 添加大量成员
        const int MAX_MEMBERS = 100;
        for (int i = 0; i < MAX_MEMBERS; ++i) {
            land->data.memberXuids.push_back("member_" + std::to_string(i));
        }

        REQUIRE(land->data.memberXuids.size() == MAX_MEMBERS);
        REQUIRE(land->hasBasicPermission("member_0"));
        REQUIRE(land->hasBasicPermission("member_" + std::to_string(MAX_MEMBERS - 1)));
    }
}

} // namespace rlx_land::test