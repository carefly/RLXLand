#pragma once

#include "data/core/DataLoaderTraits.h"
#include "data/land/LandDataManager.h"
#include "data/town/TownCore.h"
#include "data/town/TownDataManager.h"
#include <memory>
#include <string>
#include <vector>


#define LAND_RANGE 1000000

namespace rlx_land {

class DataService {
public:
    static std::shared_ptr<DataService> getInstance();

    template <typename T>
    void loadItems();

    template <typename T>
    void createItem(typename DataLoaderTraits<T>::DataType data);

    template <typename T>
    void deleteItem(typename DataLoaderTraits<T>::DataType data);

    template <typename T>
    void modifyItemPermission(typename DataLoaderTraits<T>::InfoType* info, int perm);

    template <typename T>
    void addItemMember(typename DataLoaderTraits<T>::InfoType* info, const std::string& playerName);

    template <typename T>
    void removeItemMember(typename DataLoaderTraits<T>::InfoType* info, const std::string& playerName);

    template <typename T>
    static LONG64 getMaxId();

    template <typename T>
    std::vector<typename DataLoaderTraits<T>::InfoType*> getAllItems();

    // 空间查询接口
    template <typename T>
    typename DataLoaderTraits<T>::InfoType* findItemAt(LONG64 x, LONG64 z, int dimension);

    // 便捷的专用查询方法
    LandInformation* findLandAt(LONG64 x, LONG64 z, int dimension);
    TownInformation* findTownAt(LONG64 x, LONG64 z, int dimension);

    // Town 特有的方法（无法统一的方法）
    void             transferTownMayor(TownInformation* ti, const std::string& playerName);
    TownInformation* findTownByName(const std::string& name);

private:
    std::unique_ptr<LandDataManager> landManager;
    std::unique_ptr<TownDataManager> townManager;

    DataService();

    // 统一的管理器获取方法
    template <typename T>
    auto* getManager() {
        if constexpr (std::is_same_v<T, LandData>) {
            return landManager.get();
        } else if constexpr (std::is_same_v<T, TownData>) {
            return townManager.get();
        }
    }

    // 简化后的模板化通用方法
    template <typename T>
    void loadData(typename DataLoaderTraits<T>::ManagerType* manager, const std::string& typeName);

    template <typename T>
    void
    createItemInternal(typename DataLoaderTraits<T>::DataType data, typename DataLoaderTraits<T>::ManagerType* manager);

    template <typename T>
    void
    deleteItemInternal(typename DataLoaderTraits<T>::DataType data, typename DataLoaderTraits<T>::ManagerType* manager);

    template <typename T>
    void modifyItemPermissionInternal(
        typename DataLoaderTraits<T>::InfoType*    info,
        int                                        perm,
        typename DataLoaderTraits<T>::ManagerType* manager
    );

    template <typename T>
    void addItemMemberInternal(
        typename DataLoaderTraits<T>::InfoType*    info,
        const std::string&                         playerName,
        typename DataLoaderTraits<T>::ManagerType* manager
    );

    template <typename T>
    void removeItemMemberInternal(
        typename DataLoaderTraits<T>::InfoType*    info,
        const std::string&                         playerName,
        typename DataLoaderTraits<T>::ManagerType* manager
    );

    // 模板化的地图更新函数
    template <typename U>
    void updateSpatialMap(U* info, LONG64 x, LONG64 z, int d);

    template <typename U>
    void updateSpatialMapRange(U* info, LONG64 x1, LONG64 z1, LONG64 x2, LONG64 z2, int d);
};

} // namespace rlx_land