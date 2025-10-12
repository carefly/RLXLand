add_rules("mode.debug", "mode.release")

add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")

-- add_requires("levilamina x.x.x") for a specific version
-- add_requires("levilamina develop") to use develop version
-- please note that you should add bdslibrary yourself if using dev version
if is_config("target_type", "server") then
    add_requires("levilamina 1.5.1", {configs = {target_type = "server"}})
else
    add_requires("levilamina 1.5.1", {configs = {target_type = "client"}})
end

add_requires("levibuildscript")
add_requires("nlohmann_json")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

option("target_type")
    set_default("server")
    set_showmenu(true)
    set_values("server", "client")
option_end()

-- 添加测试选项
option("tests")
    set_default(false)
    set_showmenu(true)
    set_description("Build tests")
option_end()

-- 添加Catch2依赖（仅在构建测试时）
if has_config("tests") then
    add_requires("catch2")
end

target("RLXLand") -- Change this to your mod name.
    add_rules("@levibuildscript/linkrule")
    add_rules("@levibuildscript/modpacker")
    add_cxflags( "/EHa", "/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204")
    add_defines("NOMINMAX", "UNICODE")
    add_packages("levilamina")
    add_packages("nlohmann_json")
    set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")
    add_headerfiles("src/**.h")
    add_files("src/**.cpp")
    add_includedirs("src")
    -- if is_config("target_type", "server") then
    --     add_includedirs("src-server")
    --     add_files("src-server/**.cpp")
    -- else
    --     add_includedirs("src-client")
    --     add_files("src-client/**.cpp")
    -- end

-- 新增测试目标
if has_config("tests") then
    target("tests")
        set_kind("binary")
        set_languages("c++20")
        add_packages("catch2")
        add_packages("nlohmann_json")
        -- 包含必要的源文件（使用条件编译）
        add_headerfiles("src/data/core/**.h")
        add_headerfiles("src/data/land/**.h")
        add_headerfiles("src/data/town/**.h")
        add_headerfiles("src/data/service/**.h")
        add_headerfiles("src/common/**.h")
        add_includedirs("src")
        
        -- 包含源文件（这些文件会使用 TESTING 宏）
        add_files("src/data/core/BaseDataManager.cpp")
        add_files("src/data/land/LandCore.cpp")
        add_files("src/data/town/TownCore.cpp")
        add_files("src/data/service/DataService.cpp")
        add_files("src/common/JsonLoader.cpp")
        add_files("src/common/LeviLaminaAPI.cpp")  -- 这个文件会被条件编译
        add_files("src/mod/town/permissions/TownPermissionChecker.cpp")  -- 添加权限检查器实现
        
        -- 包含测试文件（排除与源文件重复的测试文件）
        add_files("tests/mocks/**.cpp")
        add_files("tests/utils/TestBaseInformation.cpp")
        add_files("tests/utils/TestDataLoader.cpp")
        add_files("tests/utils/TestEnvironment.cpp")
        add_files("tests/utils/TestHelper.cpp")
        add_files("tests/integration/**.cpp")
        add_files("tests/main.cpp")
        add_includedirs("tests")
        
        -- 添加必要的编译标志
        add_cxflags("/EHa", "/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204")
        add_defines("NOMINMAX", "UNICODE", "BUILDING_TESTS", "TESTING")
        
        -- 不链接LeviLamina，使用模拟实现
        set_exceptions("none") -- To avoid conflicts with /EHa.
        set_symbols("debug")
        set_runtimes("MD")
end
