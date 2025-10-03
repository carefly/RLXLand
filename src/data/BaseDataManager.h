#pragma once
#include "common/LeviLaminaAPI.h"
#include "common/exceptions/LandExceptions.h"
#include "data/BaseInformation.h"
#include "data/DataLoaderTraits.h"
#include <algorithm>
#include <vector>


namespace rlx_land {

template <typename T, typename U>
class BaseDataManager {
protected:
    std::vector<U*> informationList;
    using Traits = DataLoaderTraits<T>; // 类型别名简化代码

    // 虚函数需要子类实现
    virtual std::string getFilePath() const      = 0;
    virtual void        initInformation(U* info) = 0;

public:
    // 构造函数
    BaseDataManager() = default;

    virtual ~BaseDataManager() {
        for (auto* info : informationList) {
            delete info;
        }
        informationList.clear();
    }

    // 加载数据
    void load() {
        std::vector<T> items = Traits::loadFromFile(getFilePath());
        for (const auto& item : items) {
            auto info = new U(item);
            initInformation(info);
            informationList.push_back(info);
        }
    }

    // 创建数据项
    void create(T data) {
        // 先从文件加载现有数据
        std::vector<T> items = Traits::loadFromFile(getFilePath());

        // 检查ID是否已存在
        auto it = std::find_if(items.begin(), items.end(), [&data](const T& item) { return item.id == data.id; });

        if (it != items.end()) {
            throw DuplicateException("Item duplicate with ID: " + std::to_string(data.id));
        }

        // 添加新项到列表
        items.push_back(data);

        // 保存回文件
        Traits::saveToFile(getFilePath(), items);

        // 添加到内存中
        auto info = new U(data);
        initInformation(info);
        informationList.push_back(info);
    }

    // 删除数据项
    void remove(T data) {
        // 从文件加载现有数据
        std::vector<T> items = Traits::loadFromFile(getFilePath());

        // 查找并删除指定项
        auto it = std::find_if(items.begin(), items.end(), [&data](const T& item) { return item.id == data.id; });

        if (it != items.end()) {
            items.erase(it);

            // 保存回文件
            Traits::saveToFile(getFilePath(), items);

            // 从内存中删除
            auto memIt = std::find_if(informationList.begin(), informationList.end(), [&data](U* info) {
                return info->data.id == data.id;
            });

            if (memIt != informationList.end()) {
                U* info = *memIt;
                informationList.erase(memIt);
                delete info;
            }
        } else {
            throw LandNotFoundException("Item not found to delete");
        }
    }

    // 修改权限
    void modifyPerm(U* info, int perm) {
        // 在JSON中修改权限
        // 先从文件加载现有数据
        std::vector<T> items = Traits::loadFromFile(getFilePath());

        // 查找并更新指定项
        auto it = std::find_if(items.begin(), items.end(), [info](const T& item) { return item.id == info->data.id; });

        if (it != items.end()) {
            it->perm = perm;

            // 保存回文件
            Traits::saveToFile(getFilePath(), items);
        } else {
            throw LandNotFoundException("Item not found to modify permission");
        }

        // 更新内存中的权限
        info->data.perm = perm;
    }

    // 添加成员
    void addMember(U* info, const std::string& playerName) {
        if (info == nullptr) throw LandNotFoundException("Item not found");

        std::string xuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
        if (xuid.empty()) throw PlayerNotFoundException("Player not found: " + playerName);

        // 检查玩家是否已经是成员
        if (std::find(info->data.memberXuids.begin(), info->data.memberXuids.end(), xuid)
            != info->data.memberXuids.end()) {
            throw DuplicateException("Player is already a member: " + playerName);
        }

        // 添加新成员
        info->data.memberXuids.push_back(xuid);

        std::vector<T> items = Traits::loadFromFile(getFilePath());

        auto it = std::find_if(items.begin(), items.end(), [info](const T& item) { return item.id == info->data.id; });

        if (it != items.end()) {
            it->memberXuids = info->data.memberXuids;
            Traits::saveToFile(getFilePath(), items);
        }
    }

    // 移除成员
    void removeMember(U* info, const std::string& playerName) {
        if (info == nullptr) throw LandNotFoundException("Item not found");

        std::string xuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
        if (xuid.empty()) throw PlayerNotFoundException("Player not found: " + playerName);

        // 查找并移除成员
        auto it = std::find(info->data.memberXuids.begin(), info->data.memberXuids.end(), xuid);
        if (it == info->data.memberXuids.end()) {
            throw NotMemberException("Player is not a member: " + playerName);
        }

        info->data.memberXuids.erase(it);

        std::vector<T> items = Traits::loadFromFile(getFilePath());

        auto it2 = std::find_if(items.begin(), items.end(), [info](const T& item) { return item.id == info->data.id; });

        if (it2 != items.end()) {
            it2->memberXuids = info->data.memberXuids;
            Traits::saveToFile(getFilePath(), items);
        }
    }

    // 获取最大ID
    LONG64 getMaxId() const {
        LONG64 max = 0;
        for (const auto& itemInfo : informationList) {
            if (max < itemInfo->data.id) max = itemInfo->data.id;
        }
        return max;
    }

    // 获取所有项
    std::vector<U*> getAllItems() const { return informationList; }
};

} // namespace rlx_land