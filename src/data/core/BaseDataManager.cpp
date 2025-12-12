#include "BaseDataManager.h"
#include "common/LeviLaminaAPI.h"
#include "common/exceptions/LandExceptions.h"
#include "data/town/TownCore.h"


namespace rlx_land {

template <typename T, typename U>
BaseDataManager<T, U>::~BaseDataManager() {
    for (auto* info : informationList) {
        delete info;
    }
    informationList.clear();
}

template <typename T, typename U>
void BaseDataManager<T, U>::create(T data) {
    // 先从文件加载现有数据
    std::vector<T> items = Traits::loadFromFile();

    // 检查ID是否已存在
    auto it = std::find_if(items.begin(), items.end(), [&data](const T& item) { return item.id == data.id; });

    if (it != items.end()) {
        throw DuplicateException("Item duplicate with ID: " + std::to_string(data.id));
    }

    // 添加新项到列表
    items.push_back(data);

    // 保存回文件
    Traits::saveToFile(items);

    // 添加到内存中
    auto info = new U(data);
    initInformation(info);
    informationList.push_back(info);
}

template <typename T, typename U>
void BaseDataManager<T, U>::remove(LONG64 id) {
    // 从文件加载现有数据
    std::vector<T> items = Traits::loadFromFile();

    // 查找并删除指定项
    auto it = std::find_if(items.begin(), items.end(), [&id](const T& item) { return item.id == id; });

    if (it != items.end()) {
        items.erase(it);

        // 保存回文件
        Traits::saveToFile(items);

        // 从内存中删除
        auto memIt = std::find_if(informationList.begin(), informationList.end(), [&id](U* info) {
            return info->getId() == id;
        });

        if (memIt != informationList.end()) {
            U* info = *memIt;
            informationList.erase(memIt);
            delete info;
        }
    } else {
        throw RealmNotFoundException("此处没有领地");
    }
}

template <typename T, typename U>
void BaseDataManager<T, U>::modifyPerm(U* info, int perm) {
    if (info == nullptr) {
        throw RealmNotFoundException("领地信息为空");
    }

    // 在JSON中修改权限
    // 先从文件加载现有数据
    std::vector<T> items = Traits::loadFromFile();

    // 查找并更新指定项
    auto it = std::find_if(items.begin(), items.end(), [info](const T& item) { return item.id == info->getId(); });

    if (it != items.end()) {
        it->perm = perm;

        // 保存回文件
        Traits::saveToFile(items);

        // 只有在文件更新成功后才更新内存中的权限
        info->setPermission(perm);
    } else {
        throw RealmNotFoundException("此处没有领地");
    }
}

template <typename T, typename U>
void BaseDataManager<T, U>::addMember(U* info, const std::string& playerName) {
    if (info == nullptr) throw RealmNotFoundException("Item not found");

    std::string xuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
    if (xuid.empty()) throw ::PlayerNotFoundException("Player not found: " + playerName);

    // 检查玩家是否已经是成员
    const auto& members = info->getMemberXuids();
    if (std::find(members.begin(), members.end(), xuid) != members.end()) {
        throw DuplicateException("Player is already a member: " + playerName);
    }

    // 添加新成员
    info->addMember(xuid);

    std::vector<T> items = Traits::loadFromFile();

    auto it = std::find_if(items.begin(), items.end(), [info](const T& item) { return item.id == info->getId(); });

    if (it != items.end()) {
        it->memberXuids = info->getMemberXuids();
        Traits::saveToFile(items);
    }
}

template <typename T, typename U>
void BaseDataManager<T, U>::removeMember(U* info, const std::string& playerName) {
    if (info == nullptr) throw RealmNotFoundException("Item not found");

    std::string xuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
    if (xuid.empty()) throw ::PlayerNotFoundException("Player not found: " + playerName);

    // 查找并移除成员
    const auto& members = info->getMemberXuids();
    auto        it      = std::find(members.begin(), members.end(), xuid);
    if (it == members.end()) {
        throw NotMemberException("Player is not a member: " + playerName);
    }

    info->removeMember(xuid);

    std::vector<T> items = Traits::loadFromFile();

    auto it2 = std::find_if(items.begin(), items.end(), [info](const T& item) { return item.id == info->getId(); });

    if (it2 != items.end()) {
        it2->memberXuids = info->getMemberXuids();
        Traits::saveToFile(items);
    }
}

template <typename T, typename U>
LONG64 BaseDataManager<T, U>::getMaxId() const {
    LONG64 max = 0;
    for (const auto& itemInfo : informationList) {
        if (max < itemInfo->getId()) max = itemInfo->getId();
    }
    return max;
}

template <typename T, typename U>
std::vector<U*> BaseDataManager<T, U>::getAllItems() const {
    return informationList;
}

template <typename T, typename U>
void BaseDataManager<T, U>::addItemDirectly(U* info) {
    informationList.push_back(info);
}

template <typename T, typename U>
void BaseDataManager<T, U>::clearAllItems() {
    for (auto* info : informationList) {
        delete info;
    }
    informationList.clear();
}

// 显式实例化 - 需要根据实际使用的类型进行调整
template class BaseDataManager<LandData, LandInformation>;
template class BaseDataManager<rlx_town::TownData, rlx_town::TownInformation>;

} // namespace rlx_land
