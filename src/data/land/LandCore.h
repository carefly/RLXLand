#pragma once

#include "common/LeviLaminaAPI.h"
#include "data/core/BaseInformation.h"
#include <basetsd.h>
#include <format>
#include <string>



namespace rlx_land {

class LandData : public BaseData {
public:
    std::string ownerXuid;

    // 默认构造函数
    LandData() = default;

    // 完整构造函数
    LandData(
        int                             x,
        int                             z,
        int                             x_end,
        int                             z_end,
        const std::string&              ownerXuid,
        int                             d,
        int                             perm,
        const std::string&              description,
        const std::vector<std::string>& memberXuids,
        LONG64                          id
    );

    // 便利构造函数（用于创建新土地）
    LandData(int x, int z, int x_end, int z_end, const std::string& ownerXuid, int d, LONG64 id);

private:
    // 基础数据校验方法
    static void validateBasicData(int x, int z, int x_end, int z_end, const std::string& ownerXuid, int perm);
};

class LandInformation : public BaseInformation {
public:
    explicit LandInformation(LandData ld);

    // 删除重复字段：不再有 LandData ld 和 std::string ownerName

    // Land 特有数据的访问接口（不暴露 LandData）
    [[nodiscard]] const std::string& getOwnerXuid() const { return landData.ownerXuid; }

    // 设置接口
    void setOwnerXuid(const std::string& xuid);

    // 重新初始化 ownerName（用于管理器）
    void refreshOwnerName() { setOwnerName(LeviLaminaAPI::getPlayerNameByXuid(landData.ownerXuid)); }

protected:
    [[nodiscard]] bool checkIsOwner(const std::string& xuid) const override;

private:
    LandData landData; // 私有，不对外暴露
};

} // namespace rlx_land
