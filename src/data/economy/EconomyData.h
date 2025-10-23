#pragma once
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>



namespace rlx_land {

// 玩家经济数据结构
struct EconomyData {
    std::string xuid;  // 玩家XUID
    int64_t     money; // 玩家金钱

    EconomyData() : money(0) {}
    explicit EconomyData(std::string x, int64_t m) : xuid(std::move(x)), money(m) {}

    // 序列化方法
    [[nodiscard]] nlohmann::json toJson() const {
        nlohmann::json j;
        j["xuid"]  = xuid;
        j["money"] = money;
        return j;
    }

    // 反序列化方法
    static EconomyData fromJson(const nlohmann::json& j) {
        EconomyData data;
        data.xuid  = j.value("xuid", "");
        data.money = j.value("money", 0LL);
        return data;
    }
};

} // namespace rlx_land