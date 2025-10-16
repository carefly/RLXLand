#pragma once

#include "common/LeviLaminaAPI.h"
#include "data/core/BaseInformation.h"
#include <basetsd.h>
#include <format>
#include <string>



namespace rlx_land {

class TownData : public BaseData {
public:
    std::string name;
    std::string mayorXuid;

    // 默认构造函数
    TownData() = default;

    // 完整构造函数
    TownData(
        int                             x,
        int                             z,
        int                             x_end,
        int                             z_end,
        const std::string&              name,
        const std::string&              mayorXuid,
        int                             d,
        int                             perm,
        const std::string&              description,
        const std::vector<std::string>& memberXuids,
        LONG64                          id
    );

    // 便利构造函数（用于创建新城镇）
    TownData(
        int                x,
        int                z,
        int                x_end,
        int                z_end,
        const std::string& name,
        const std::string& mayorXuid,
        int                d,
        LONG64             id
    );

private:
    // 基础数据校验方法
    static void validateBasicData(
        int                x,
        int                z,
        int                x_end,
        int                z_end,
        const std::string& name,
        const std::string& mayorXuid,
        int                perm
    );
};

class TownInformation : public BaseInformation {
public:
    explicit TownInformation(TownData td);

    // 删除重复字段：不再有 TownData td 和 std::string mayorName
    // 统一使用基类的 ownerName 存储市长名称

    // Town 特有数据的访问接口（不暴露 TownData）
    [[nodiscard]] const std::string& getTownName() const { return townData.name; }
    [[nodiscard]] const std::string& getMayorXuid() const { return townData.mayorXuid; }

    // 设置接口
    void setTownName(const std::string& name) { townData.name = name; }
    void setMayorXuid(const std::string& xuid);

    // 重新初始化 ownerName（用于管理器）
    void refreshOwnerName() { setOwnerName(LeviLaminaAPI::getPlayerNameByXuid(townData.mayorXuid)); }

protected:
    [[nodiscard]] bool checkIsOwner(const std::string& xuid) const override;

private:
    TownData townData; // 私有，不对外暴露
};

} // namespace rlx_land