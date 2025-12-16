#include "common/ModConfig.h"
#include "common/Utf8Utils.h"
#include "mod/RLXLand.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>


#ifdef _WIN32
#include <Windows.h>
#endif

namespace rlx_land {

bool ModConfig::s_requireMoneyPlugin = false; // 默认值：false（可选）
bool ModConfig::s_configLoaded       = false;

bool ModConfig::load(const std::string& configPath) {
    if (s_configLoaded) {
        return true; // 已经加载过
    }

    // 确保配置目录存在
    std::filesystem::path configFile(configPath);
    std::filesystem::path configDir = configFile.parent_path();
    if (!configDir.empty()) {
        try {
            std::filesystem::create_directories(configDir);
        } catch (const std::exception& e) {
            RLXLand::getInstance().getSelf().getLogger().warn(
                std::format("Failed to create config directory {}: {}", configDir.string(), e.what())
            );
        }
    }

    // 尝试加载配置文件
    std::ifstream  file = Utf8Utils::createUtf8InputStream(configPath);
    nlohmann::json json;
    bool           fileExists = file.is_open();
    bool           needWrite  = false;

    if (fileExists) {
        // 配置文件存在，读取它
        try {
            file >> json;
            file.close();
        } catch (const std::exception& e) {
            file.close();
            RLXLand::getInstance().getSelf().getLogger().error(
                std::format("Failed to parse config file {}: {}, creating new default config", configPath, e.what())
            );
            // 解析失败，创建新的默认配置
            json      = nlohmann::json::object();
            needWrite = true;
        }
    } else {
        // 配置文件不存在，创建默认配置
        json      = nlohmann::json::object();
        needWrite = true;
        RLXLand::getInstance().getSelf().getLogger().info(
            std::format("Config file not found at {}, creating default config", configPath)
        );
    }

    // 确保 land 配置对象存在
    if (!json.contains("land")) {
        // land 不存在，创建它
        json["land"] = nlohmann::json::object();
        needWrite    = true;
    } else if (!json["land"].is_object()) {
        // land 存在但不是对象，转换为对象
        RLXLand::getInstance().getSelf().getLogger().warn(std::format(
            "Config file {} has 'land' field but it's not an object, converting it to an object",
            configPath
        ));
        json["land"] = nlohmann::json::object();
        needWrite    = true;
    }

    // 如果 requireMoneyPlugin 不存在，添加默认值 true
    if (!json["land"].contains("requireMoneyPlugin")) {
        json["land"]["requireMoneyPlugin"] = true;
        needWrite                          = true;
    }

    // 读取配置项
    s_requireMoneyPlugin = json["land"]["requireMoneyPlugin"].get<bool>();

    // 如果需要写入（新建或更新），保存配置文件
    if (needWrite) {
        try {
            std::ofstream outFile = Utf8Utils::createUtf8OutputStream(configPath);
            if (outFile.is_open()) {
                outFile << json.dump(4); // 使用 4 空格缩进，格式化输出
                outFile.close();
                RLXLand::getInstance().getSelf().getLogger().info(
                    std::format("Config file saved to {}: requireMoneyPlugin={}", configPath, s_requireMoneyPlugin)
                );
            } else {
                RLXLand::getInstance().getSelf().getLogger().warn(
                    std::format("Failed to write config file {}", configPath)
                );
            }
        } catch (const std::exception& e) {
            RLXLand::getInstance().getSelf().getLogger().error(
                std::format("Failed to save config file {}: {}", configPath, e.what())
            );
        }
    } else {
        RLXLand::getInstance().getSelf().getLogger().info(
            std::format("Config loaded from {}: requireMoneyPlugin={}", configPath, s_requireMoneyPlugin)
        );
    }

    s_configLoaded = true;
    return true;
}

bool ModConfig::requireMoneyPlugin() {
    if (!s_configLoaded) {
        load(); // 自动加载配置
    }
    return s_requireMoneyPlugin;
}

bool ModConfig::checkMoneyDllExists() {
    // 检查多个可能的位置
    std::vector<std::string> searchPaths = {
        ".",                   // 当前目录
        "plugins",             // plugins 目录
        "../plugins",          // 上级 plugins 目录
        "plugins/RLXMoney",    // RLXMoney 子目录
        "../plugins/RLXMoney", // 上级 RLXMoney 子目录
    };

    for (const auto& basePath : searchPaths) {
        std::filesystem::path dllPath = std::filesystem::path(basePath) / MONEY_DLL_NAME;
        if (std::filesystem::exists(dllPath) && std::filesystem::is_regular_file(dllPath)) {
            return true;
        }
    }

    // 也尝试使用 Windows API 检查（可以检查系统路径）
#ifdef _WIN32
    HMODULE hModule = LoadLibraryA(MONEY_DLL_NAME);
    if (hModule != nullptr) {
        FreeLibrary(hModule);
        return true;
    }
#endif

    return false;
}

} // namespace rlx_land
