#define CATCH_CONFIG_MAIN
#include "utils/TestEnvironment.h"
#include <catch2/catch_test_macros.hpp>


// 全局测试设置和清理
TEST_CASE("Global Test Setup", "[setup]") {
    // 初始化测试环境
    rlx_land::test::TestEnvironment::getInstance().initialize();
}

// 全局测试清理
TEST_CASE("Global Test Cleanup", "[cleanup]") {
    // 清理测试环境
    rlx_land::test::TestEnvironment::getInstance().cleanup();
}