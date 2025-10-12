#pragma once

#include "common/LeviLaminaAPI.h"
#include "data/core/BaseInformation.h"
#include <basetsd.h>
#include <string>


namespace rlx_land {

class LandData : public BaseData {
public:
    std::string ownerXuid;
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
