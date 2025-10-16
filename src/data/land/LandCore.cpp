#include "LandCore.h"
#include "common/LeviLaminaAPI.h"
#include "common/exceptions/LandExceptions.h"

namespace rlx_land {

// LandData 构造函数实现
LandData::LandData(
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
)
: ownerXuid(ownerXuid) {
    // 基础数据校验
    validateBasicData(x, z, x_end, z_end, ownerXuid, perm);

    this->x           = x;
    this->z           = z;
    this->x_end       = x_end;
    this->z_end       = z_end;
    this->d           = d;
    this->perm        = perm;
    this->id          = id;
    this->description = description;
    this->memberXuids = memberXuids;
}

LandData::LandData(int x, int z, int x_end, int z_end, const std::string& ownerXuid, int d, LONG64 id)
: ownerXuid(ownerXuid) {
    // 基础数据校验
    validateBasicData(x, z, x_end, z_end, ownerXuid, 0);

    this->x           = x;
    this->z           = z;
    this->x_end       = x_end;
    this->z_end       = z_end;
    this->d           = d;
    this->perm        = 0;
    this->id          = id;
    this->description = "";
    this->memberXuids = {};
}

LandInformation::LandInformation(LandData ld) : BaseInformation(static_cast<BaseData&>(ld)), landData(std::move(ld)) {
    setOwnerName(LeviLaminaAPI::getPlayerNameByXuid(landData.ownerXuid));
}

bool LandInformation::checkIsOwner(const std::string& xuid) const { return landData.ownerXuid == xuid; }

void LandInformation::setOwnerXuid(const std::string& xuid) {
    landData.ownerXuid = xuid;
    setOwnerName(LeviLaminaAPI::getPlayerNameByXuid(xuid));
}

// 基础数据校验方法
void LandData::validateBasicData(int x, int z, int x_end, int z_end, const std::string& ownerXuid, int perm) {
    // 1. 坐标基本有效性校验
    if (x > x_end) {
        throw InvalidCoordinatesException(std::format("起始X坐标({})不能大于结束X坐标({})", x, x_end));
    }

    if (z > z_end) {
        throw InvalidCoordinatesException(std::format("起始Z坐标({})不能大于结束Z坐标({})", z, z_end));
    }

    // 2. 所有者XUID校验
    if (ownerXuid.empty()) {
        throw InvalidPlayerInfoException("所有者XUID不能为空");
    }

    // 3. 权限值基础校验
    if (perm < 0) {
        throw InvalidPermissionException("权限值不能小于0");
    }
}

} // namespace rlx_land