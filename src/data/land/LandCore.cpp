#include "LandCore.h"
#include "common/JsonLoader.h"
#include "common/LeviLaminaAPI.h"
#include "common/exceptions/LandExceptions.h"
#include <format>


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
    // 更新 dataRef 指向 landData 中的 BaseData 部分
    updateDataRef(static_cast<BaseData&>(landData));
    // 使用 refreshOwnerName() 来获取 ownerName，它会尝试从文件名获取作为 fallback
    refreshOwnerName();
}

bool LandInformation::checkIsOwner(const std::string& xuid) const { return landData.ownerXuid == xuid; }

void LandInformation::refreshOwnerName() {
    // 优先使用 API 获取玩家名称（适用于测试和生产环境）
    std::string playerName = LeviLaminaAPI::getPlayerNameByXuid(landData.ownerXuid);
    // 如果 API 返回空，尝试从文件名读取（用于从文件加载的情况）
    if (playerName.empty()) {
        playerName = JsonLoader::getPlayerNameFromFileName(landData.ownerXuid);
    }
    // 如果还是为空，使用 "Unknown" 作为 fallback
    if (playerName.empty()) {
        playerName = "Unknown";
    }
    setOwnerName(playerName);
}

void LandInformation::setOwnerXuid(const std::string& xuid) {
    landData.ownerXuid = xuid;
    // 使用 refreshOwnerName() 来保持一致性，优先从文件名读取
    refreshOwnerName();
}

// 验证当前数据的合法性
void LandData::validate() const { validateBasicData(x, z, x_end, z_end, ownerXuid, perm); }

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