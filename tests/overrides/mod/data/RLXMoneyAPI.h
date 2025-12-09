#pragma once

#include <optional>
#include <string>

namespace rlx_money {

// 测试环境下的 RLXMoneyAPI 空实现，直接使用本地缓存逻辑
class RLXMoneyAPI {
public:
    static inline std::string getDefaultCurrencyId() { return "default"; }

    static inline std::optional<int> getBalance(const std::string&, const std::string&) {
        return std::nullopt;
    }

    static inline bool setBalance(const std::string&, const std::string&, int, const std::string&) {
        return true;
    }

    static inline bool addMoney(const std::string&, const std::string&, int, const std::string&) {
        return true;
    }

    static inline bool reduceMoney(const std::string&, const std::string&, int, const std::string&) {
        return true;
    }
};

} // namespace rlx_money

