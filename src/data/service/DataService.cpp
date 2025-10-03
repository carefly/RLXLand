#include "DataService.h"
#include "common/JsonLoader.h"
#include "common/LeviLaminaAPI.h"
#include "data/land/LandCore.h"
#include "data/spatial/SpatialMap.h"
#include "mod/RLXLand.h"
#include <memory>


namespace rlx_land {

DataService::DataService()
: landManager(std::make_unique<LandDataManager>()),
  townManager(std::make_unique<TownDataManager>()) {}

std::shared_ptr<DataService> DataService::getInstance() {
    static std::shared_ptr<DataService> instance(new DataService());
    return instance;
}

// Town 特有的方法实现
void DataService::transferTownMayor(TownInformation* ti, const std::string& playerName) {
    townManager->transferMayor(ti, playerName);
}

TownInformation* DataService::findTownByName(const std::string& name) {
    auto towns = getInstance()->townManager->getAllItems();
    for (const auto& townInfo : towns) {
        if (townInfo->td.name == name) return townInfo;
    }
    return nullptr;
}

// 统一的公共接口实现
template <typename T>
void DataService::loadItems() {
    using Traits = DataLoaderTraits<T>;
    loadData<T>(Traits::getDefaultFilePath(), getManager<T>(), Traits::getTypeName());
}

template <typename T>
void DataService::createItem(typename DataLoaderTraits<T>::DataType data) {
    createItemInternal<T>(std::move(data), getManager<T>());
}

template <typename T>
void DataService::deleteItem(typename DataLoaderTraits<T>::DataType data) {
    deleteItemInternal<T>(std::move(data), getManager<T>());
}

template <typename T>
void DataService::modifyItemPermission(typename DataLoaderTraits<T>::InfoType* info, int perm) {
    modifyItemPermissionInternal<T>(info, perm, getManager<T>());
}

template <typename T>
void DataService::addItemMember(typename DataLoaderTraits<T>::InfoType* info, const std::string& playerName) {
    addItemMemberInternal<T>(info, playerName, getManager<T>());
}

template <typename T>
void DataService::removeItemMember(typename DataLoaderTraits<T>::InfoType* info, const std::string& playerName) {
    removeItemMemberInternal<T>(info, playerName, getManager<T>());
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

TownInformation* DataService::findTownAt(LONG64 x, LONG64 z, int dimension) {
    return findItemAt<TownData>(x, z, dimension);
}

// 私有模板方法实现
template <typename T>
void DataService::loadData(
    const std::string&                         jsonPath,
    typename DataLoaderTraits<T>::ManagerType* manager,
    const std::string&                         typeName
) {
    using Traits   = DataLoaderTraits<T>;
    using DataType = typename Traits::DataType;
    using InfoType = typename Traits::InfoType;

    std::vector<DataType> items = Traits::loadFromFile(jsonPath);

    RLXLand::getInstance().getSelf().getLogger().info(std::format("load {} {}s from JSON", items.size(), typeName));

    for (const auto& item : items) {
        // Land 特有的范围检查
        if constexpr (std::is_same_v<T, LandData>) {
            LONG64 x1 = item.x;
            LONG64 x2 = item.dx;
            LONG64 z1 = item.z;
            LONG64 z2 = item.dz;

            if (x1 >= LAND_RANGE || x1 <= -LAND_RANGE || x2 >= LAND_RANGE || x2 <= -LAND_RANGE || z1 >= LAND_RANGE
                || z1 <= -LAND_RANGE || z2 >= LAND_RANGE || z2 <= -LAND_RANGE)
                continue;
        }

        auto info = new InfoType(item);

        // 初始化特定信息
        if constexpr (std::is_same_v<InfoType, LandInformation>) {
            info->ownerName = LeviLaminaAPI::getPlayerNameByXuid(item.ownerXuid);
        } else if constexpr (std::is_same_v<InfoType, TownInformation>) {
            info->mayorName = LeviLaminaAPI::getPlayerNameByXuid(item.mayorXuid);
        }

        manager->getAllItems().push_back(info);

        // 更新空间地图
        updateSpatialMapRange<InfoType>(info, item.x, item.z, item.dx, item.dz, item.d);
    }
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
        updateSpatialMapRange<InfoType>(info, data.x, data.z, data.dx, data.dz, data.d);
    }
}

template <typename T>
void DataService::deleteItemInternal(
    typename DataLoaderTraits<T>::DataType     data,
    typename DataLoaderTraits<T>::ManagerType* manager
) {
    using InfoType = typename DataLoaderTraits<T>::InfoType;

    manager->remove(std::move(data));

    updateSpatialMapRange<InfoType>(nullptr, data.x, data.z, data.dx, data.dz, data.d);
}

template <typename T>
void DataService::modifyItemPermissionInternal(
    typename DataLoaderTraits<T>::InfoType*    info,
    int                                        perm,
    typename DataLoaderTraits<T>::ManagerType* manager
) {
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
    for (LONG64 x = x1; x <= x2; x++) {
        for (LONG64 z = z1; z <= z2; z++) {
            SpatialMap<U>::getInstance()->set(info, x, z, d);
        }
    }
}

// 显式模板实例化
template void   DataService::loadItems<LandData>();
template void   DataService::createItem<LandData>(LandData data);
template void   DataService::deleteItem<LandData>(LandData data);
template void   DataService::modifyItemPermission<LandData>(LandInformation* info, int perm);
template void   DataService::addItemMember<LandData>(LandInformation* info, const std::string& playerName);
template void   DataService::removeItemMember<LandData>(LandInformation* info, const std::string& playerName);
template LONG64 DataService::getMaxId<LandData>();
template std::vector<LandInformation*> DataService::getAllItems<LandData>();

template void   DataService::loadItems<TownData>();
template void   DataService::createItem<TownData>(TownData data);
template void   DataService::deleteItem<TownData>(TownData data);
template void   DataService::modifyItemPermission<TownData>(TownInformation* info, int perm);
template void   DataService::addItemMember<TownData>(TownInformation* info, const std::string& playerName);
template void   DataService::removeItemMember<TownData>(TownInformation* info, const std::string& playerName);
template LONG64 DataService::getMaxId<TownData>();
template std::vector<TownInformation*> DataService::getAllItems<TownData>();

// 地图更新函数的模板实例化
template void DataService::updateSpatialMap<LandInformation>(LandInformation* info, LONG64 x, LONG64 z, int d);
template void DataService::updateSpatialMap<TownInformation>(TownInformation* info, LONG64 x, LONG64 z, int d);
template void DataService::updateSpatialMapRange<
    LandInformation>(LandInformation* info, LONG64 x1, LONG64 z1, LONG64 x2, LONG64 z2, int d);
template void DataService::updateSpatialMapRange<
    TownInformation>(TownInformation* info, LONG64 x1, LONG64 z1, LONG64 x2, LONG64 z2, int d);

// 空间查询方法的模板实例化
template LandInformation* DataService::findItemAt<LandData>(LONG64 x, LONG64 z, int dimension);
template TownInformation* DataService::findItemAt<TownData>(LONG64 x, LONG64 z, int dimension);

} // namespace rlx_land