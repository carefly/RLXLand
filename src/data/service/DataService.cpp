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
template <typename T, typename U>
void DataService::loadItems() {
    if constexpr (std::is_same_v<T, LandData>) {
        loadData<T, U, LandDataManager>(JsonLoader::LANDS_JSON_PATH, landManager.get(), "land");
    } else if constexpr (std::is_same_v<T, TownData>) {
        loadData<T, U, TownDataManager>(JsonLoader::TOWNS_JSON_PATH, townManager.get(), "town");
    }
}

template <typename T, typename U>
void DataService::createItem(T data) {
    if constexpr (std::is_same_v<T, LandData>) {
        createItemInternal<T, U, LandDataManager>(std::move(data), landManager.get());
    } else if constexpr (std::is_same_v<T, TownData>) {
        createItemInternal<T, U, TownDataManager>(std::move(data), townManager.get());
    }
}

template <typename T, typename U>
void DataService::deleteItem(T data) {
    if constexpr (std::is_same_v<T, LandData>) {
        deleteItemInternal<T, U, LandDataManager>(std::move(data), landManager.get());
    } else if constexpr (std::is_same_v<T, TownData>) {
        deleteItemInternal<T, U, TownDataManager>(std::move(data), townManager.get());
    }
}

template <typename T, typename U>
void DataService::modifyItemPermission(U* info, int perm) {
    if constexpr (std::is_same_v<T, LandData>) {
        modifyItemPermissionInternal<T, U, LandDataManager>(info, perm, landManager.get());
    } else if constexpr (std::is_same_v<T, TownData>) {
        modifyItemPermissionInternal<T, U, TownDataManager>(info, perm, townManager.get());
    }
}

template <typename T, typename U>
void DataService::addItemMember(U* info, const std::string& playerName) {
    if constexpr (std::is_same_v<T, LandData>) {
        addItemMemberInternal<T, U, LandDataManager>(info, playerName, landManager.get());
    } else if constexpr (std::is_same_v<T, TownData>) {
        addItemMemberInternal<T, U, TownDataManager>(info, playerName, townManager.get());
    }
}

template <typename T, typename U>
void DataService::removeItemMember(U* info, const std::string& playerName) {
    if constexpr (std::is_same_v<T, LandData>) {
        removeItemMemberInternal<T, U, LandDataManager>(info, playerName, landManager.get());
    } else if constexpr (std::is_same_v<T, TownData>) {
        removeItemMemberInternal<T, U, TownDataManager>(info, playerName, townManager.get());
    }
}

template <typename T, typename U>
LONG64 DataService::getMaxId() {
    if constexpr (std::is_same_v<T, LandData>) {
        return getInstance()->landManager->getMaxId();
    } else if constexpr (std::is_same_v<T, TownData>) {
        return getInstance()->townManager->getMaxId();
    }
    return 0;
}

template <typename T, typename U>
std::vector<U*> DataService::getAllItems() {
    if constexpr (std::is_same_v<T, LandData>) {
        return getInstance()->landManager->getAllItems();
    } else if constexpr (std::is_same_v<T, TownData>) {
        return getInstance()->townManager->getAllItems();
    }
    return {};
}

// 私有模板方法实现
template <typename T, typename U, typename ManagerType>
void DataService::loadData(const std::string& jsonPath, ManagerType* manager, const std::string& typeName) {
    std::vector<T> items;

    if constexpr (std::is_same_v<T, LandData>) {
        items = JsonLoader::loadLandsFromFile(jsonPath);
    } else if constexpr (std::is_same_v<T, TownData>) {
        items = JsonLoader::loadTownsFromFile(jsonPath);
    }

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

        auto info = new U(item);

        // 初始化特定信息
        if constexpr (std::is_same_v<U, LandInformation>) {
            info->ownerName = LeviLaminaAPI::getPlayerNameByXuid(item.ownerXuid);
        } else if constexpr (std::is_same_v<U, TownInformation>) {
            info->mayorName = LeviLaminaAPI::getPlayerNameByXuid(item.mayorXuid);
        }

        manager->getAllItems().push_back(info);

        // 更新空间地图
        updateSpatialMapRange<U>(info, item.x, item.z, item.dx, item.dz, item.d);
    }
}

template <typename T, typename U, typename ManagerType>
void DataService::createItemInternal(T data, ManagerType* manager) {
    manager->create(std::move(data));

    // 更新地图
    auto items = manager->getAllItems();
    if (!items.empty()) {
        U* info = items.back();
        updateSpatialMapRange<U>(info, data.x, data.z, data.dx, data.dz, data.d);
    }
}

template <typename T, typename U, typename ManagerType>
void DataService::deleteItemInternal(T data, ManagerType* manager) {
    manager->remove(std::move(data));

    updateSpatialMapRange<U>(nullptr, data.x, data.z, data.dx, data.dz, data.d);
}

template <typename T, typename U, typename ManagerType>
void DataService::modifyItemPermissionInternal(U* info, int perm, ManagerType* manager) {
    manager->modifyPerm(info, perm);
}

template <typename T, typename U, typename ManagerType>
void DataService::addItemMemberInternal(U* info, const std::string& playerName, ManagerType* manager) {
    manager->addMember(info, playerName);
}

template <typename T, typename U, typename ManagerType>
void DataService::removeItemMemberInternal(U* info, const std::string& playerName, ManagerType* manager) {
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
template void DataService::loadItems<LandData, LandInformation>();
template void DataService::createItem<LandData, LandInformation>(LandData data);
template void DataService::deleteItem<LandData, LandInformation>(LandData data);
template void DataService::modifyItemPermission<LandData, LandInformation>(LandInformation* info, int perm);
template void
DataService::addItemMember<LandData, LandInformation>(LandInformation* info, const std::string& playerName);
template void
DataService::removeItemMember<LandData, LandInformation>(LandInformation* info, const std::string& playerName);
template LONG64                        DataService::getMaxId<LandData, LandInformation>();
template std::vector<LandInformation*> DataService::getAllItems<LandData, LandInformation>();

template void DataService::loadItems<TownData, TownInformation>();
template void DataService::createItem<TownData, TownInformation>(TownData data);
template void DataService::deleteItem<TownData, TownInformation>(TownData data);
template void DataService::modifyItemPermission<TownData, TownInformation>(TownInformation* info, int perm);
template void
DataService::addItemMember<TownData, TownInformation>(TownInformation* info, const std::string& playerName);
template void
DataService::removeItemMember<TownData, TownInformation>(TownInformation* info, const std::string& playerName);
template LONG64                        DataService::getMaxId<TownData, TownInformation>();
template std::vector<TownInformation*> DataService::getAllItems<TownData, TownInformation>();

// 地图更新函数的模板实例化
template void DataService::updateSpatialMap<LandInformation>(LandInformation* info, LONG64 x, LONG64 z, int d);
template void DataService::updateSpatialMap<TownInformation>(TownInformation* info, LONG64 x, LONG64 z, int d);
template void DataService::updateSpatialMapRange<
    LandInformation>(LandInformation* info, LONG64 x1, LONG64 z1, LONG64 x2, LONG64 z2, int d);
template void DataService::updateSpatialMapRange<
    TownInformation>(TownInformation* info, LONG64 x1, LONG64 z1, LONG64 x2, LONG64 z2, int d);

} // namespace rlx_land