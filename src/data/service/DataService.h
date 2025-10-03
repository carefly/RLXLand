#pragma once

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

    // 统一的模板化接口
    template <typename T, typename U>
    void loadItems();

    template <typename T, typename U>
    void createItem(T data);

    template <typename T, typename U>
    void deleteItem(T data);

    template <typename T, typename U>
    void modifyItemPermission(U* info, int perm);

    template <typename T, typename U>
    void addItemMember(U* info, const std::string& playerName);

    template <typename T, typename U>
    void removeItemMember(U* info, const std::string& playerName);

    template <typename T, typename U>
    static LONG64 getMaxId();

    template <typename T, typename U>
    std::vector<U*> getAllItems();

    // Town 特有的方法（无法统一的方法）
    void             transferTownMayor(TownInformation* ti, const std::string& playerName);
    TownInformation* findTownByName(const std::string& name);

private:
    std::unique_ptr<LandDataManager> landManager;
    std::unique_ptr<TownDataManager> townManager;

    DataService();

    // 模板化的通用方法声明
    template <typename T, typename U, typename ManagerType>
    void loadData(const std::string& jsonPath, ManagerType* manager, const std::string& typeName);

    template <typename T, typename U, typename ManagerType, typename MapType>
    void createItemInternal(T data, ManagerType* manager);

    template <typename T, typename U, typename ManagerType, typename MapType>
    void deleteItemInternal(T data, ManagerType* manager);

    template <typename T, typename U, typename ManagerType>
    void modifyItemPermissionInternal(U* info, int perm, ManagerType* manager);

    template <typename T, typename U, typename ManagerType>
    void addItemMemberInternal(U* info, const std::string& playerName, ManagerType* manager);

    template <typename T, typename U, typename ManagerType>
    void removeItemMemberInternal(U* info, const std::string& playerName, ManagerType* manager);
};

} // namespace rlx_land