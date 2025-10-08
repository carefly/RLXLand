// 简单的测试文件，用于验证按玩家存储的lands功能
#include "src/common/JsonLoader.h"
#include "src/data/land/LandCore.h"
#include <iostream>

using namespace rlx_land;

int main() {
    std::cout << "Testing player-based lands storage..." << std::endl;

    // 创建测试数据
    std::vector<LandData> testLands;

    // 玩家1的lands
    LandData land1;
    land1.id          = 1;
    land1.x           = 100;
    land1.z           = 200;
    land1.dx          = 10;
    land1.dz          = 10;
    land1.d           = 0;
    land1.perm        = 1;
    land1.ownerXuid   = "12345678-1234-1234-1234-123456789abc";
    land1.description = "Test Land 1";
    testLands.push_back(land1);

    // 玩家2的lands
    LandData land2;
    land2.id          = 2;
    land2.x           = 300;
    land2.z           = 400;
    land2.dx          = 15;
    land2.dz          = 15;
    land2.d           = 0;
    land2.perm        = 1;
    land2.ownerXuid   = "87654321-4321-4321-4321-cba987654321";
    land2.description = "Test Land 2";
    testLands.push_back(land2);

    // 玩家1的第二个land
    LandData land3;
    land3.id          = 3;
    land3.x           = 500;
    land3.z           = 600;
    land3.dx          = 8;
    land3.dz          = 8;
    land3.d           = 0;
    land3.perm        = 1;
    land3.ownerXuid   = "12345678-1234-1234-1234-123456789abc";
    land3.description = "Test Land 3";
    testLands.push_back(land3);

    std::cout << "Created " << testLands.size() << " test lands" << std::endl;

    // 测试保存
    try {
        JsonLoader::saveLandsToFile(testLands);
        std::cout << "Successfully saved lands to player files" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error saving lands: " << e.what() << std::endl;
        return 1;
    }

    // 测试加载
    try {
        auto loadedLands = JsonLoader::loadLandsFromFile();
        std::cout << "Successfully loaded " << loadedLands.size() << " lands from player files" << std::endl;

        // 验证数据
        if (loadedLands.size() == testLands.size()) {
            std::cout << "✓ Data count matches" << std::endl;
        } else {
            std::cout << "✗ Data count mismatch!" << std::endl;
        }

        // 显示加载的数据
        for (const auto& land : loadedLands) {
            std::cout << "Land ID: " << land.id << ", Owner: " << land.ownerXuid << ", Desc: " << land.description
                      << std::endl;
        }

    } catch (const std::exception& e) {
        std::cout << "Error loading lands: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Test completed successfully!" << std::endl;
    return 0;
}