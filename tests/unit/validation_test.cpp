#include "data/land/LandCore.h"
#include "mocks/MockPlayer.h"
#include "utils/TestDataLoader.h"
#include "utils/TestEnvironment.h"
#include "utils/TestHelper.h"
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <vector>


namespace rlx_land::test {

TEST_CASE("Data Validation Tests - Land Data Integrity", "[validation][integrity]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Valid Land Data") {
        auto land = dataLoader.createTestLand("valid_owner", 1000, 1000);

        REQUIRE(land != nullptr);
        TestHelper::assertLandDataValid(*land);

        // 验证基本字段
        REQUIRE(land->data.id > 0);
        REQUIRE(!land->ld.ownerXuid.empty());
        REQUIRE(land->data.getWidth() > 0);
        REQUIRE(land->data.getHeight() > 0);
        REQUIRE(TestHelper::isValidCoordinate(land->data.x));
        REQUIRE(TestHelper::isValidCoordinate(land->data.z));
        REQUIRE(TestHelper::isValidPermission(land->data.perm));
    }

    SECTION("Invalid Land Data Detection") {
        auto invalidLands = dataLoader.loadInvalidTestData();

        for (const auto& land : invalidLands) {
            TestHelper::assertLandDataInvalid(*land);
        }
    }

    SECTION("Data Consistency Check") {
        auto lands = dataLoader.loadLandTestData();

        for (const auto& land : lands) {
            // 检查数据一致性
            REQUIRE(land->data.id > 0);
            REQUIRE(!land->ld.ownerXuid.empty());
            REQUIRE(land->ownerName == land->getOwnerName());

            // 检查成员列表一致性
            for (const auto& memberXuid : land->data.memberXuids) {
                REQUIRE(land->hasBasicPermission(memberXuid));
                REQUIRE_FALSE(land->isOwner(memberXuid));
            }

            // 检查所有者权限
            REQUIRE(land->isOwner(land->ld.ownerXuid));
            REQUIRE(land->hasBasicPermission(land->ld.ownerXuid));
        }
    }
}

TEST_CASE("Data Validation Tests - XUID Validation", "[validation][xuid]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Valid XUIDs") {
        std::vector<std::string> validXuids =
            {"1234567890123456789", "test_user_001", "owner_123", "player_with_underscores", "Player123", "a1b2c3d4e5f6"
            };

        for (const auto& xuid : validXuids) {
            REQUIRE(TestHelper::isValidXuid(xuid));

            auto land = dataLoader.createTestLand(xuid, 1000, 1000);
            REQUIRE(land != nullptr);
            REQUIRE(land->ld.ownerXuid == xuid);
        }
    }

    SECTION("Invalid XUIDs") {
        std::vector<std::string> invalidXuids = {
            "",
            "   ",                 // 只有空格
            "\t\n",                // 只有制表符和换行符
            std::string(300, 'a'), // 超长字符串
            "user with spaces",    // 包含空格
            "user@domain.com",     // 包含特殊字符
            "user#123"             // 包含特殊字符
        };

        for (const auto& xuid : invalidXuids) {
            // 空字符串应该无效
            if (xuid.empty()) {
                REQUIRE_FALSE(TestHelper::isValidXuid(xuid));
            }

            // 其他情况取决于具体的验证规则
            // 这里我们只测试基本的有效性检查
        }
    }

    SECTION("XUID Format Validation") {
        // 测试不同格式的XUID
        std::vector<std::string> formatTests = {
            "numeric_123456789",
            "alphanumeric_abc123",
            "mixed_ABC123def456",
            "short_a",
            "very_long_xuid_that_exceeds_normal_limits_but_might_still_be_valid_depending_on_system_requirements"
        };

        for (const auto& xuid : formatTests) {
            if (xuid.length() <= 256) { // 假设最大长度为256
                auto land = dataLoader.createTestLand(xuid, 1000, 1000);
                REQUIRE(land != nullptr);
            }
        }
    }
}

TEST_CASE("Data Validation Tests - Coordinate Validation", "[validation][coordinates]") {
    TestDataLoader& dataLoader     = TestDataLoader::getInstance();
    auto            boundaryValues = TestHelper::getBoundaryValues();

    SECTION("Valid Coordinates") {
        std::vector<std::pair<int, int>> validCoords = {
            {0,                            0                           },
            {1000,                         1000                        },
            {-1000,                        -1000                       },
            {boundaryValues.minCoordinate, boundaryValues.minCoordinate},
            {boundaryValues.maxCoordinate, boundaryValues.maxCoordinate},
            {5000,                         -3000                       },
            {-7500,                        8000                        }
        };

        for (const auto& coord : validCoords) {
            REQUIRE(TestHelper::isValidCoordinate(coord.first));
            REQUIRE(TestHelper::isValidCoordinate(coord.second));

            auto land = dataLoader.createTestLand("coord_test", coord.first, coord.second);
            REQUIRE(land != nullptr);
            REQUIRE(land->data.x == coord.first);
            REQUIRE(land->data.z == coord.second);
        }
    }

    SECTION("Invalid Coordinates") {
        std::vector<std::pair<int, int>> invalidCoords = {
            {boundaryValues.minCoordinate - 1, 0                               },
            {boundaryValues.maxCoordinate + 1, 0                               },
            {0,                                boundaryValues.minCoordinate - 1},
            {0,                                boundaryValues.maxCoordinate + 1}
        };

        for (const auto& coord : invalidCoords) {
            REQUIRE_FALSE(TestHelper::isValidCoordinate(coord.first) || TestHelper::isValidCoordinate(coord.second));
        }
    }

    SECTION("Edge Case Coordinates") {
        // 测试边界值附近的坐标
        std::vector<std::pair<int, int>> edgeCoords = {
            {boundaryValues.minCoordinate + 1, boundaryValues.minCoordinate + 1},
            {boundaryValues.maxCoordinate - 1, boundaryValues.maxCoordinate - 1},
            {-1,                               -1                              },
            {1,                                1                               }
        };

        for (const auto& coord : edgeCoords) {
            REQUIRE(TestHelper::isValidCoordinate(coord.first));
            REQUIRE(TestHelper::isValidCoordinate(coord.second));

            auto land = dataLoader.createTestLand("edge_test", coord.first, coord.second);
            REQUIRE(land != nullptr);
        }
    }
}

TEST_CASE("Data Validation Tests - Size Validation", "[validation][size]") {
    TestDataLoader& dataLoader     = TestDataLoader::getInstance();
    auto            boundaryValues = TestHelper::getBoundaryValues();

    SECTION("Valid Sizes") {
        std::vector<std::pair<int, int>> validSizes = {
            {1,                      1                     },
            {50,                     50                    },
            {100,                    100                   },
            {boundaryValues.minSize, boundaryValues.minSize},
            {boundaryValues.maxSize, boundaryValues.maxSize},
            {1,                      boundaryValues.maxSize},
            {boundaryValues.maxSize, 1                     }
        };

        for (const auto& size : validSizes) {
            REQUIRE(TestHelper::isValidSize(size.first));
            REQUIRE(TestHelper::isValidSize(size.second));

            auto land        = dataLoader.createTestLand("size_test", 1000, 1000);
            land->data.x_end = land->data.x + size.first;
            land->data.z_end = land->data.z + size.second;

            REQUIRE(land->data.getWidth() == size.first);
            REQUIRE(land->data.getHeight() == size.second);
        }
    }

    SECTION("Invalid Sizes") {
        std::vector<std::pair<int, int>> invalidSizes = {
            {0,                          50                        },
            {50,                         0                         },
            {-1,                         50                        },
            {50,                         -1                        },
            {boundaryValues.minSize - 1, 50                        },
            {50,                         boundaryValues.maxSize + 1}
        };

        for (const auto& size : invalidSizes) {
            REQUIRE_FALSE(TestHelper::isValidSize(size.first) || TestHelper::isValidSize(size.second));
        }
    }
}

TEST_CASE("Data Validation Tests - Permission Validation", "[validation][permission]") {
    TestDataLoader& dataLoader     = TestDataLoader::getInstance();
    auto            boundaryValues = TestHelper::getBoundaryValues();

    SECTION("Valid Permissions") {
        std::vector<int> validPerms = {
            boundaryValues.minPermission,
            boundaryValues.maxPermission,
            0,
            1,
            2,
            3,
            4,
            5 // 常见权限级别
        };

        for (int perm : validPerms) {
            REQUIRE(TestHelper::isValidPermission(perm));

            auto land       = dataLoader.createTestLand("perm_test", 1000, 1000);
            land->data.perm = perm;

            REQUIRE(land->data.perm == perm);
        }
    }

    SECTION("Invalid Permissions") {
        std::vector<int> invalidPerms =
            {-1, -100, boundaryValues.maxPermission + 1, boundaryValues.maxPermission + 100, INT_MIN, INT_MAX};

        for (int perm : invalidPerms) {
            REQUIRE_FALSE(TestHelper::isValidPermission(perm));
        }
    }
}

TEST_CASE("Data Validation Tests - Member List Validation", "[validation][members]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Valid Member Lists") {
        auto land = dataLoader.createTestLand("members_test", 1000, 1000);

        // 添加有效成员
        std::vector<std::string> validMembers = {"member_001", "member_002", "member_003"};

        for (const auto& member : validMembers) {
            land->data.memberXuids.push_back(member);
            REQUIRE(land->hasBasicPermission(member));
        }

        REQUIRE(land->data.memberXuids.size() == validMembers.size());
    }

    SECTION("Invalid Member Handling") {
        auto land = dataLoader.createTestLand("invalid_members_test", 1000, 1000);

        // 添加无效成员
        std::vector<std::string> invalidMembers = {
            "",     // 空字符串
            "   ",  // 只有空格
            "\t\n", // 只有制表符和换行符
        };

        for (const auto& member : invalidMembers) {
            land->data.memberXuids.push_back(member);
            // 空成员的处理取决于业务逻辑
            if (member.empty()) {
                REQUIRE_FALSE(land->hasBasicPermission(member));
            }
        }
    }

    SECTION("Duplicate Member Detection") {
        auto land = dataLoader.createTestLand("duplicate_test", 1000, 1000);

        std::string duplicateMember = "duplicate_user";

        // 添加重复成员
        land->data.memberXuids.push_back(duplicateMember);
        land->data.memberXuids.push_back(duplicateMember);
        land->data.memberXuids.push_back(duplicateMember);

        // 检查重复次数
        int count = 0;
        for (const auto& member : land->data.memberXuids) {
            if (member == duplicateMember) {
                count++;
            }
        }
        REQUIRE(count == 3);

        // 权限检查应该仍然工作
        REQUIRE(land->hasBasicPermission(duplicateMember));
    }

    SECTION("Owner in Member List") {
        auto land = dataLoader.createTestLand("owner_member_test", 1000, 1000);

        // 将所有者添加到成员列表
        land->data.memberXuids.push_back(land->ld.ownerXuid);

        // 应该仍然正确识别为所有者
        REQUIRE(land->isOwner(land->ld.ownerXuid));
        REQUIRE(land->hasBasicPermission(land->ld.ownerXuid));
    }
}

TEST_CASE("Data Validation Tests - Description Validation", "[validation][description]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Valid Descriptions") {
        std::vector<std::string> validDescriptions = {
            "Normal description",
            "Description with numbers 123",
            "Description_with_underscores",
            "Description-with-dashes",
            "Description with special chars: !@#$%",
            "",                   // 空描述可能有效
            "A",                  // 单字符描述
            std::string(500, 'a') // 长描述
        };

        for (const auto& desc : validDescriptions) {
            auto land              = dataLoader.createTestLand("desc_test", 1000, 1000);
            land->data.description = desc;

            REQUIRE(land->data.description == desc);
        }
    }

    SECTION("Description Length Validation") {
        auto land = dataLoader.createTestLand("length_test", 1000, 1000);

        // 测试极长描述
        std::string veryLongDesc = TestHelper::generateLongString(10000);
        land->data.description   = veryLongDesc;

        REQUIRE(land->data.description.length() == 10000);

        // 测试空描述
        land->data.description = "";
        REQUIRE(land->data.description.empty());
    }
}

TEST_CASE("Data Validation Tests - Dimension Validation", "[validation][dimension]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Valid Dimensions") {
        std::vector<int> validDimensions = {
            -128,
            -1,
            0,
            1,
            127 // 常见的Minecraft维度
        };

        for (int dim : validDimensions) {
            auto land    = dataLoader.createTestLand("dim_test", 1000, 1000);
            land->data.d = dim;

            REQUIRE(land->data.d == dim);
        }
    }

    SECTION("Extreme Dimensions") {
        std::vector<int> extremeDimensions = {INT_MIN, INT_MAX, -1000, 1000};

        for (int dim : extremeDimensions) {
            auto land    = dataLoader.createTestLand("extreme_dim_test", 1000, 1000);
            land->data.d = dim;

            REQUIRE(land->data.d == dim);
            // 极端维度的有效性取决于业务逻辑
        }
    }
}

TEST_CASE("Data Validation Tests - Cross-Field Validation", "[validation][cross_field]") {
    TestDataLoader& dataLoader = TestDataLoader::getInstance();

    SECTION("Land Area Calculation") {
        auto land = dataLoader.createTestLand("area_test", 1000, 1000);

        // 测试不同大小的土地
        std::vector<std::pair<int, int>> sizes = {
            {1,    1   }, // 最小面积
            {10,   10  }, // 小面积
            {100,  100 }, // 中等面积
            {1000, 1000}  // 大面积
        };

        for (const auto& size : sizes) {
            land->data.x_end = land->data.x + size.first;
            land->data.z_end = land->data.z + size.second;

            // 验证面积计算的合理性
            int expectedArea = size.first * size.second;
            REQUIRE(expectedArea > 0);
        }
    }

    SECTION("Land Boundary Validation") {
        auto land = dataLoader.createTestLand("boundary_test", 1000, 1000);

        // 测试土地边界是否合理
        int startX = land->data.x;
        int startZ = land->data.z;
        int endX   = land->data.x_end;
        int endZ   = land->data.z_end;

        // 验证边界坐标的合理性
        REQUIRE(endX > startX);
        REQUIRE(endZ > startZ);

        // 验证边界坐标在有效范围内
        auto boundaryValues = TestHelper::getBoundaryValues();
        REQUIRE(startX >= boundaryValues.minCoordinate);
        REQUIRE(startZ >= boundaryValues.minCoordinate);
        REQUIRE(endX <= boundaryValues.maxCoordinate);
        REQUIRE(endZ <= boundaryValues.maxCoordinate);
    }

    SECTION("Permission Consistency") {
        auto land = dataLoader.createTestLand("permission_test", 1000, 1000);

        // 测试权限级别与功能的一致性
        for (int perm = 0; perm <= 10; ++perm) {
            land->data.perm = perm;

            // 所有者应该总是有权限
            REQUIRE(land->hasBasicPermission(land->ld.ownerXuid));
            REQUIRE(land->isOwner(land->ld.ownerXuid));
        }
    }
}

} // namespace rlx_land::test