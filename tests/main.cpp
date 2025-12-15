#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_DISABLE_AUTO_RERUN
#include "utils/TestEnvironment.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>

// 全局测试环境管理器
struct TestEnvironmentManager {
    TestEnvironmentManager() {
        rlx_land::test::TestEnvironment::getInstance().initialize();
    }

    ~TestEnvironmentManager() {
        // 空析构函数，避免任何清理操作
    }
};

// 全局实例
static TestEnvironmentManager g_envManager;