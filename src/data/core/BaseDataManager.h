#pragma once

#include "BaseInformation.h"
#include "DataLoaderTraits.h"
#include <vector>


namespace rlx_land {

template <typename T, typename U>
class BaseDataManager {
protected:
    std::vector<U*> informationList;
    using Traits = DataLoaderTraits<T>; // 类型别名简化代码

    virtual void initInformation(U* info) = 0;

public:
    // 构造函数
    BaseDataManager() = default;

    virtual ~BaseDataManager();

    // 创建数据项
    void create(T data);

    // 删除数据项
    void remove(T data);

    // 修改权限
    void modifyPerm(U* info, int perm);

    // 添加成员
    void addMember(U* info, const std::string& playerName);

    // 移除成员
    void removeMember(U* info, const std::string& playerName);

    // 获取最大ID
    [[nodiscard]] LONG64 getMaxId() const;

    // 获取所有项
    std::vector<U*> getAllItems() const;

    // 清理所有数据（测试专用）
    void clearAllItems();
};

} // namespace rlx_land
