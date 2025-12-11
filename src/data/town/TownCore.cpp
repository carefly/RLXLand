#include "TownCore.h"
#include "common/LeviLaminaAPI.h"
#include "common/exceptions/LandExceptions.h"
#include <format>

namespace rlx_town {

using rlx_land::LeviLaminaAPI;
using ::InvalidCoordinatesException;
using ::InvalidPermissionException;
using ::InvalidPlayerInfoException;

// TownData 构造函数实现
TownData::TownData(
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
)
: name(name),
  mayorXuid(mayorXuid) {
    // 基础数据校验
    validateBasicData(x, z, x_end, z_end, name, mayorXuid, perm);

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

TownData::TownData(
    int                x,
    int                z,
    int                x_end,
    int                z_end,
    const std::string& name,
    const std::string& mayorXuid,
    int                d,
    LONG64             id
)
: name(name),
  mayorXuid(mayorXuid) {
    // 基础数据校验
    validateBasicData(x, z, x_end, z_end, name, mayorXuid, 0);

    this->x           = x;
    this->z           = z;
    this->x_end       = x_end;
    this->z_end       = z_end;
    this->d           = d;
    this->perm        = 0;
    this->id          = id;
    this->description = "城镇 " + name;
    this->memberXuids = {};
}

TownInformation::TownInformation(TownData td) 
    : BaseInformation(static_cast<BaseData&>(td)), 
      townData(std::move(td)) {
    // 更新 dataRef 指向 townData 中的 BaseData 部分
    updateDataRef(static_cast<BaseData&>(townData));
    setOwnerName(LeviLaminaAPI::getPlayerNameByXuid(townData.mayorXuid));
}

bool TownInformation::checkIsOwner(const std::string& xuid) const { return townData.mayorXuid == xuid; }

void TownInformation::setMayorXuid(const std::string& xuid) {
    townData.mayorXuid = xuid;
    setOwnerName(LeviLaminaAPI::getPlayerNameByXuid(xuid));
}

// 基础数据校验方法
void TownData::validateBasicData(
    int                x,
    int                z,
    int                x_end,
    int                z_end,
    const std::string& name,
    const std::string& mayorXuid,
    int                perm
) {
    // 1. 坐标基本有效性校验
    if (x > x_end) {
        throw InvalidCoordinatesException(std::format("起始X坐标({})不能大于结束X坐标({})", x, x_end));
    }

    if (z > z_end) {
        throw InvalidCoordinatesException(std::format("起始Z坐标({})不能大于结束Z坐标({})", z, z_end));
    }

    // 2. 城镇名称校验
    if (name.empty()) {
        throw InvalidPlayerInfoException("城镇名称不能为空");
    }

    // 3. 镇长XUID校验
    if (mayorXuid.empty()) {
        throw InvalidPlayerInfoException("镇长XUID不能为空");
    }

    // 4. 权限值基础校验
    if (perm < 0) {
        throw InvalidPermissionException("权限值不能小于0");
    }
}

} // namespace rlx_town