#pragma once

#include <cstdint>
#include <string>

namespace rlx_money {

// 玩家余额结构
struct PlayerBalance {
    std::string currencyId; // 币种ID
    int64_t     balance;    // 余额
};

// 财富排行榜条目
struct TopBalanceEntry {
    std::string xuid;    // 玩家XUID
    int64_t     balance; // 余额
};

// 交易记录
struct TransactionRecord {
    std::string transactionId; // 交易ID
    std::string xuid;          // 玩家XUID
    std::string currencyId;    // 币种ID
    int64_t     amount;        // 金额（正数为收入，负数为支出）
    std::string description;   // 交易描述
    int64_t     timestamp;     // 时间戳
};

} // namespace rlx_money
