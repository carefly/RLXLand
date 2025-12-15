#include <format>
#ifndef TESTING
#include "mod/RLXLand.h"
#else
// 测试环境下的简单日志实现
#include <iostream>
#define LOG_INFO(msg) std::cout << "[INFO] " << msg << std::endl
#endif

#include "DataService.h"
#include "common/JsonLoader.h"
#include "common/exceptions/LandExceptions.h"
#include "data/land/LandCore.h"
#include "data/spatial/SpatialMap.h"
#include "mod/town/permissions/TownPermissionChecker.h"
#include <memory>


namespace rlx_land {

DataService::DataService()
: landManager(std::make_unique<LandDataManager>()),
  townManager(std::make_unique<rlx_town::TownDataManager>()) {}

std::shared_ptr<DataService> DataService::getInstance() {
    static std::shared_ptr<DataService> instance(new DataService());
    return instance;
}

void DataService::initialize() {
    // 加载所有领地数据
    loadItems<LandData>();

    // 加载所有城镇数据
    loadItems<rlx_town::TownData>();
}

// Town 特有的方法实现
void DataService::transferTownMayor(LONG64 x, LONG64 z, int dimension, const std::string& playerName) {
    // 1. 验证城镇是否存在
    auto ti = findTownAt(x, z, dimension);
    if (ti == nullptr) {
        throw RealmNotFoundException("找不到指定的城镇");
    }

    townManager->transferMayor(ti, playerName);
}

rlx_town::TownInformation* DataService::findTownByName(const std::string& name) {
    auto towns = getInstance()->townManager->getAllItems();
    for (const auto& townInfo : towns) {
        if (townInfo->getTownName() == name) return townInfo;
    }
    return nullptr;
}

#ifdef TESTING
void DataService::clearAllData() {

    // 1. 先清理空间索引结构（不删除对象）
    // 清理Land空间地图
    SpatialMap<LandInformation>::getInstance()->clearAll();

    // 清理Town空间地图
    SpatialMap<rlx_town::TownInformation>::getInstance()->clearAll();

    // 2. 再清理数据管理器（负责实际删除对象）
    // 清理Land数据
    landManager->clearAllItems();

    // 清理Town数据
    townManager->clearAllItems();

    // 3. 清理JSON文件
    // 清空Land数据文件
    DataLoaderTraits<LandData>::saveToFile(std::vector<LandData>());

    // 清空Town数据文件
    DataLoaderTraits<rlx_town::TownData>::saveToFile(std::vector<rlx_town::TownData>());
}
#endif

// 统一的公共接口实现
template <typename T>
void DataService::loadItems() {
    using Traits = DataLoaderTraits<T>;
    loadData<T>(getManager<T>(), Traits::getTypeName());
}

template <typename T>
void DataService::createItem(typename DataLoaderTraits<T>::DataType data, const PlayerInfo& playerInfo) {
    // 首先验证 PlayerInfo 的合法性
    validatePlayerInfo(playerInfo);

    if constexpr (std::is_same_v<T, LandData>) {
        validateLandCreation(data, playerInfo);
    } else if constexpr (std::is_same_v<T, rlx_town::TownData>) {
        validateTownCreation(data, playerInfo);
    }
    createItemInternal<T>(std::move(data), getManager<T>());
}

template <typename T>
void DataService::deleteItem(LONG64 x, LONG64 z, int dimension) {
    deleteItemInternal<T>(x, z, dimension, getManager<T>());
}

template <typename T>
void DataService::modifyItemPermission(LONG64 x, LONG64 z, int dimension, int perm, const PlayerInfo& playerInfo) {
    modifyItemPermissionInternal<T>(x, z, dimension, perm, playerInfo, getManager<T>());
}

template <typename T>
void DataService::addItemMember(
    LONG64             x,
    LONG64             z,
    int                dimension,
    const PlayerInfo&  playerInfo,
    const std::string& playerName
) {
    // 1. 验证领地是否存在
    auto li = findItemAt<T>(x, z, dimension);
    if (li == nullptr) {
        throw RealmNotFoundException("找不到指定的领地");
    }

    // 2. 验证领地所有权
    if (!li->isOwner(playerInfo.xuid) && !playerInfo.isOperator) {
        throw RealmPermissionException("你不是领地主人");
    }

    // 2. 验证目标玩家存在
    auto memberXuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
    if (memberXuid.empty()) {
        throw ::PlayerNotFoundException(std::format("找不到玩家 {}，请检查玩家ID拼写", playerName));
    }

    // 3. 调用原有的addItemMember方法执行实际操作
    addItemMemberInternal<T>(li, playerName, getManager<T>());
}

template <typename T>
void DataService::removeItemMember(
    LONG64             x,
    LONG64             z,
    int                dimension,
    const PlayerInfo&  playerInfo,
    const std::string& playerName
) {
    // 1. 验证领地是否存在
    auto li = findItemAt<T>(x, z, dimension);
    if (li == nullptr) {
        throw RealmNotFoundException("找不到指定的领地");
    }

    // 2. 验证领地所有权
    if (!li->isOwner(playerInfo.xuid) && !playerInfo.isOperator) {
        throw RealmPermissionException("你不是领地主人");
    }

    // 2. 验证目标玩家存在
    auto memberXuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
    if (memberXuid.empty()) {
        throw ::PlayerNotFoundException(std::format("找不到玩家 {}，请检查玩家ID拼写", playerName));
    }

    // 3. 调用原有的removeItemMember方法执行实际操作
    removeItemMemberInternal<T>(li, playerName, getManager<T>());
}

template <typename T>
LONG64 DataService::getMaxId() {
    return getInstance()->getManager<T>()->getMaxId();
}

template <typename T>
std::vector<typename DataLoaderTraits<T>::InfoType*> DataService::getAllItems() {
    return getInstance()->getManager<T>()->getAllItems();
}

// 空间查询方法实现
template <typename T>
typename DataLoaderTraits<T>::InfoType* DataService::findItemAt(LONG64 x, LONG64 z, int dimension) {
    using InfoType = typename DataLoaderTraits<T>::InfoType;
    return SpatialMap<InfoType>::getInstance()->find(x, z, dimension);
}

// 便捷方法实现
LandInformation* DataService::findLandAt(LONG64 x, LONG64 z, int dimension) {
    return findItemAt<LandData>(x, z, dimension);
}

rlx_town::TownInformation* DataService::findTownAt(LONG64 x, LONG64 z, int dimension) {
    return findItemAt<rlx_town::TownData>(x, z, dimension);
}

// 私有模板方法实现
template <typename T>
void DataService::loadData(typename DataLoaderTraits<T>::ManagerType* manager, const std::string& typeName) {
    using Traits   = DataLoaderTraits<T>;
    using DataType = typename Traits::DataType;
    using InfoType = typename Traits::InfoType;

    std::vector<DataType> items = Traits::loadFromFile();

    // 统计信息
    size_t successCount     = 0;
    size_t outOfRangeCount  = 0;
    size_t invalidDataCount = 0;

#ifdef TESTING
    LOG_INFO(std::format("load {}s from JSON", typeName));
#else
    size_t totalCount = items.size();
    RLXLand::getInstance().getSelf().getLogger().info(std::format("开始加载 {} 个 {} 数据", totalCount, typeName));
#endif

    for (const auto& item : items) {
        // 范围检查和数据验证
        if constexpr (std::is_same_v<T, LandData>) {
            LONG64 x1 = item.x;
            LONG64 x2 = item.x_end;
            LONG64 z1 = item.z;
            LONG64 z2 = item.z_end;

            // 检查是否超出范围
            if (x1 >= LAND_RANGE || x1 <= -LAND_RANGE || x2 >= LAND_RANGE || x2 <= -LAND_RANGE || z1 >= LAND_RANGE
                || z1 <= -LAND_RANGE || z2 >= LAND_RANGE || z2 <= -LAND_RANGE) {
                outOfRangeCount++;
#ifndef TESTING
                RLXLand::getInstance().getSelf().getLogger().warn(
                    "跳过超出范围的{}数据: owner={}, x={}, z={}, x_end={}, z_end={}, d={}, id={} (LAND_RANGE={})",
                    typeName,
                    item.ownerXuid,
                    item.x,
                    item.z,
                    item.x_end,
                    item.z_end,
                    item.d,
                    item.id,
                    LAND_RANGE
                );
#endif
                continue;
            }

            // 验证数据合法性
            try {
                item.validate();
            } catch (const std::exception&
#ifndef TESTING
                         e
#endif
            ) {
                invalidDataCount++;
#ifndef TESTING
                RLXLand::getInstance().getSelf().getLogger().error(
                    "跳过非法{}数据: owner={}, x={}, z={}, x_end={}, z_end={}, d={}, perm={}, id={}, 原因: {}",
                    typeName,
                    item.ownerXuid,
                    item.x,
                    item.z,
                    item.x_end,
                    item.z_end,
                    item.d,
                    item.perm,
                    item.id,
                    e.what()
                );
#endif
                continue;
            }
        } else if constexpr (std::is_same_v<T, rlx_town::TownData>) {
            LONG64 x1 = item.x;
            LONG64 x2 = item.x_end;
            LONG64 z1 = item.z;
            LONG64 z2 = item.z_end;

            // 检查是否超出范围（Town 也使用相同的范围限制）
            if (x1 >= LAND_RANGE || x1 <= -LAND_RANGE || x2 >= LAND_RANGE || x2 <= -LAND_RANGE || z1 >= LAND_RANGE
                || z1 <= -LAND_RANGE || z2 >= LAND_RANGE || z2 <= -LAND_RANGE) {
                outOfRangeCount++;
#ifndef TESTING
                RLXLand::getInstance().getSelf().getLogger().warn(
                    "跳过超出范围的{}数据: name={}, mayor={}, x={}, z={}, x_end={}, z_end={}, d={}, id={} "
                    "(LAND_RANGE={})",
                    typeName,
                    item.name,
                    item.mayorXuid,
                    item.x,
                    item.z,
                    item.x_end,
                    item.z_end,
                    item.d,
                    item.id,
                    LAND_RANGE
                );
#endif
                continue;
            }

            // 验证数据合法性
            try {
                item.validate();
            } catch (const std::exception&
#ifndef TESTING
                         e
#endif
            ) {
                invalidDataCount++;
#ifndef TESTING
                RLXLand::getInstance().getSelf().getLogger().error(
                    "跳过非法{}数据: name={}, mayor={}, x={}, z={}, x_end={}, z_end={}, d={}, perm={}, id={}, 原因: {}",
                    typeName,
                    item.name,
                    item.mayorXuid,
                    item.x,
                    item.z,
                    item.x_end,
                    item.z_end,
                    item.d,
                    item.perm,
                    item.id,
                    e.what()
                );
#endif
                continue;
            }
        }

        auto info = new InfoType(item);

        // 初始化特定信息
        if constexpr (std::is_same_v<InfoType, LandInformation>
                      || std::is_same_v<InfoType, rlx_town::TownInformation>) {
            info->refreshOwnerName();
        }

        manager->addItemDirectly(info);

        // 更新空间地图
        updateSpatialMapRange<InfoType>(info, item.x, item.z, item.x_end, item.z_end, item.d);

        successCount++;
    }

    // 输出加载统计信息
#ifdef TESTING
    LOG_INFO(std::format("loaded {} {}s successfully", successCount, typeName));
#else
    if (outOfRangeCount > 0 || invalidDataCount > 0) {
        RLXLand::getInstance().getSelf().getLogger().warn(
            "加载{}数据完成: 总数={}, 成功={}, 跳过={} (超出范围={}, 非法数据={})",
            typeName,
            totalCount,
            successCount,
            outOfRangeCount + invalidDataCount,
            outOfRangeCount,
            invalidDataCount
        );
    } else {
        RLXLand::getInstance().getSelf().getLogger().info("加载{}数据完成: 总数={}, 全部成功", typeName, totalCount);
    }

#endif
}

template <typename T>
void DataService::createItemInternal(
    typename DataLoaderTraits<T>::DataType     data,
    typename DataLoaderTraits<T>::ManagerType* manager
) {
    using InfoType = typename DataLoaderTraits<T>::InfoType;

    manager->create(std::move(data));

    // 更新地图
    auto items = manager->getAllItems();
    if (!items.empty()) {
        InfoType* info = items.back();
        updateSpatialMapRange<InfoType>(
            info,
            info->getX(),
            info->getZ(),
            info->getXEnd(),
            info->getZEnd(),
            info->getDimension()
        );
    }
}

template <typename T>
void DataService::deleteItemInternal(
    LONG64                                     x,
    LONG64                                     z,
    int                                        dimension,
    typename DataLoaderTraits<T>::ManagerType* manager
) {
    using InfoType = typename DataLoaderTraits<T>::InfoType;

    // 1. 查找目标位置的项目
    auto info = findItemAt<T>(x, z, dimension);
    if (info == nullptr) {
        throw RealmNotFoundException("找不到指定的项目");
    }

    // 2. 构造DataType用于删除
    typename DataLoaderTraits<T>::DataType data;
    data.x     = info->getX();
    data.z     = info->getZ();
    data.x_end = info->getXEnd();
    data.z_end = info->getZEnd();
    data.d     = info->getDimension();
    data.id    = info->getId();

    manager->remove(data.id);

    updateSpatialMapRange<InfoType>(nullptr, data.x, data.z, data.x_end, data.z_end, data.d);
}

template <typename T>
void DataService::modifyItemPermissionInternal(
    LONG64                                     x,
    LONG64                                     z,
    int                                        dimension,
    int                                        perm,
    const PlayerInfo&                          playerInfo,
    typename DataLoaderTraits<T>::ManagerType* manager
) {
    if (perm < 0) {
        throw InvalidPermissionException("perm 不能小于0");
    }

    auto info = findItemAt<T>(x, z, dimension);
    if (info == nullptr) {
        throw RealmNotFoundException("找不到指定的项目");
    }

    // 验证所有权
    if (!info->isOwner(playerInfo.xuid) && !playerInfo.isOperator) {
        throw RealmPermissionException("你不是领地主人");
    }

    manager->modifyPerm(info, perm);
}

template <typename T>
void DataService::addItemMemberInternal(
    typename DataLoaderTraits<T>::InfoType*    info,
    const std::string&                         playerName,
    typename DataLoaderTraits<T>::ManagerType* manager
) {
    manager->addMember(info, playerName);
}

template <typename T>
void DataService::removeItemMemberInternal(
    typename DataLoaderTraits<T>::InfoType*    info,
    const std::string&                         playerName,
    typename DataLoaderTraits<T>::ManagerType* manager
) {
    manager->removeMember(info, playerName);
}

// 模板化的地图更新函数实现
template <typename U>
void DataService::updateSpatialMap(U* info, LONG64 x, LONG64 z, int d) {
    SpatialMap<U>::getInstance()->set(info, x, z, d);
}

template <typename U>
void DataService::updateSpatialMapRange(U* info, LONG64 x1, LONG64 z1, LONG64 x2, LONG64 z2, int d) {
    // 使用优化的范围设置方法，支持批量整块填充
    SpatialMap<U>::getInstance()->setRange(info, x1, z1, x2, z2, d);
}

// 显式模板实例化
template void DataService::loadItems<LandData>();
template void DataService::createItem<LandData>(LandData data, const PlayerInfo& playerInfo);
template void DataService::deleteItem<LandData>(LONG64 x, LONG64 z, int dimension);
template void
DataService::modifyItemPermission<LandData>(LONG64 x, LONG64 z, int dimension, int perm, const PlayerInfo& playerInfo);
template void DataService::addItemMember<
    LandData>(LONG64 x, LONG64 z, int dimension, const PlayerInfo& playerInfo, const std::string& playerName);
template void DataService::removeItemMember<
    LandData>(LONG64 x, LONG64 z, int dimension, const PlayerInfo& playerInfo, const std::string& playerName);
template LONG64                        DataService::getMaxId<LandData>();
template std::vector<LandInformation*> DataService::getAllItems<LandData>();

template void DataService::loadItems<rlx_town::TownData>();
template void DataService::createItem<rlx_town::TownData>(rlx_town::TownData data, const PlayerInfo& playerInfo);
template void DataService::deleteItem<rlx_town::TownData>(LONG64 x, LONG64 z, int dimension);
template void DataService::modifyItemPermission<
    rlx_town::TownData>(LONG64 x, LONG64 z, int dimension, int perm, const PlayerInfo& playerInfo);
template void DataService::addItemMember<
    rlx_town::TownData>(LONG64 x, LONG64 z, int dimension, const PlayerInfo& playerInfo, const std::string& playerName);
template void DataService::removeItemMember<
    rlx_town::TownData>(LONG64 x, LONG64 z, int dimension, const PlayerInfo& playerInfo, const std::string& playerName);
template LONG64                                  DataService::getMaxId<rlx_town::TownData>();
template std::vector<rlx_town::TownInformation*> DataService::getAllItems<rlx_town::TownData>();

// 地图更新函数的模板实例化
template void DataService::updateSpatialMap<LandInformation>(LandInformation* info, LONG64 x, LONG64 z, int d);
template void
DataService::updateSpatialMap<rlx_town::TownInformation>(rlx_town::TownInformation* info, LONG64 x, LONG64 z, int d);
template void DataService::updateSpatialMapRange<
    LandInformation>(LandInformation* info, LONG64 x1, LONG64 z1, LONG64 x2, LONG64 z2, int d);
template void DataService::updateSpatialMapRange<
    rlx_town::TownInformation>(rlx_town::TownInformation* info, LONG64 x1, LONG64 z1, LONG64 x2, LONG64 z2, int d);

// 空间查询方法的模板实例化
template LandInformation*           DataService::findItemAt<LandData>(LONG64 x, LONG64 z, int dimension);
template rlx_town::TownInformation* DataService::findItemAt<rlx_town::TownData>(LONG64 x, LONG64 z, int dimension);

// 坐标验证方法的模板实例化
template void DataService::validateCoordinatesRange<LandData>(const LandData& data);
template void DataService::validateCoordinatesRange<rlx_town::TownData>(const rlx_town::TownData& data);

// 验证方法实现
void DataService::validatePlayerInfo(const PlayerInfo& playerInfo) {
    // 验证 XUID 不为空
    if (playerInfo.xuid.empty()) {
        throw InvalidPlayerInfoException("玩家XUID不能为空");
    }

    // 通过 ll 的 API 使用 xuid 获取 playername 并验证不为空
    std::string playerName = LeviLaminaAPI::getPlayerNameByXuid(playerInfo.xuid);
    if (playerName.empty()) {
        throw InvalidPlayerInfoException("无法通过XUID获取玩家名称");
    }
}

void DataService::validateLandCreation(const LandData& data, const PlayerInfo& playerInfo) {
    // 首先验证数据的基本合法性
    data.validate();
    validateCoordinatesRange<LandData>(data);
    validatePermission(data.perm);
    validateTownPermission(data, playerInfo);
    validateLandConflict(data);
}

// 通用坐标范围验证模板函数实现
template <typename T>
void DataService::validateCoordinatesRange(const T& data) {
    // 提取类型特定参数
    const char* typeName = std::is_same_v<T, LandData> ? "领地" : "城镇";

    // 统一的验证逻辑
    // 1. 检查坐标基本有效性：起始坐标不能大于结束坐标
    if (data.x > data.x_end) {
        throw InvalidCoordinatesException(std::format("起始X坐标({})不能大于结束X坐标({})", data.x, data.x_end));
    }

    if (data.z > data.z_end) {
        throw InvalidCoordinatesException(std::format("起始Z坐标({})不能大于结束Z坐标({})", data.z, data.z_end));
    }

    // 2. 检查是否超出LAND_RANGE范围
    if (data.x > LAND_RANGE || data.x < -LAND_RANGE || data.x_end > LAND_RANGE || data.x_end < -LAND_RANGE
        || data.z > LAND_RANGE || data.z < -LAND_RANGE || data.z_end > LAND_RANGE || data.z_end < -LAND_RANGE) {
        throw RealmOutOfRangeException(std::format("{}坐标超出范围，坐标范围不能超过 +/-{}", typeName, LAND_RANGE));
    }

    if ((abs(data.x) > LAND_RANGE) || (abs(data.z) > LAND_RANGE)) {
        throw RealmOutOfRangeException(std::format("{}不能超过 {} 格", typeName, LAND_RANGE));
    }
}

void DataService::validateTownPermission(const LandData& data, const PlayerInfo& playerInfo) {
    // 检查整个领地区域是否可以圈地（确保所有坐标点都有权限）
    for (int xi = data.x; xi <= data.x_end; xi++) {
        for (int zi = data.z; zi <= data.z_end; zi++) {
            if (!rlx_town::TownPermissionChecker::canClaimLand(playerInfo, xi, zi, data.d)) {
                throw RealmPermissionException("您没有在此区域圈地的权限");
            }
        }
    }
}

void DataService::validateLandConflict(const LandData& data) {
    for (int xi = data.x; xi <= data.x_end; xi++) {
        for (int zi = data.z; zi <= data.z_end; zi++) {
            auto li = findLandAt(xi, zi, data.d);
            if (li != nullptr) {
                throw RealmConflictException(
                    std::format("领地冲突 x:{} z:{} 领地所有者 {} 请重新圈地", xi, zi, li->getOwnerName())
                );
            }
        }
    }
}

// Town验证方法实现
void DataService::validateTownCreation(const rlx_town::TownData& data, const PlayerInfo& playerInfo) {
    // 首先验证数据的基本合法性
    data.validate();
    validateCoordinatesRange<rlx_town::TownData>(data);
    validatePermission(data.perm);
    validateOperatorPermission(playerInfo);
    validateTownOverlap(data);
}


void DataService::validateOperatorPermission(const PlayerInfo& playerInfo) {
    if (!playerInfo.isOperator) {
        throw RealmPermissionException("只有腐竹可以创建城镇");
    }
}

void DataService::validateTownOverlap(const rlx_town::TownData& data) {
    for (int xi = data.x; xi <= data.x_end; xi++) {
        for (int zi = data.z; zi <= data.z_end; zi++) {
            auto town = findTownAt(xi, zi, data.d);
            if (town != nullptr) {
                throw RealmConflictException(
                    std::format("城镇冲突 x:{} z:{} 城镇所有者 {} 请重新选择区域", xi, zi, town->getOwnerName())
                );
            }
        }
    }
}

void DataService::validatePermission(int perm) {
    if (perm < 0) {
        throw InvalidPermissionException("perm 不能小于0");
    }
}

} // namespace rlx_land
