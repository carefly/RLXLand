#define CATCH_CONFIG_MAIN
#include "utils/TestEnvironment.h"
#include <catch2/catch_test_macros.hpp>

// 全局测试固定装置
struct GlobalTestFixture {
    GlobalTestFixture() {
        // 在所有测试开始前执行一次
        rlx_land::test::TestEnvironment::getInstance().initialize();
    }

    ~GlobalTestFixture() {
        // 在所有测试结束后执行一次
        rlx_land::test::TestEnvironment::getInstance().cleanup();
    }
};

// 创建全局实例，确保在程序启动时初始化，结束时清理
static GlobalTestFixture g_globalTestFixture;