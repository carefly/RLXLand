#pragma once
#include <cstdint>
#include <string>
#include <vector>


namespace rlx_land {

typedef int64_t LONG64;

class BaseData {
public:
    int                      x, z;         // 起始坐标
    int                      x_end, z_end; // 终点坐标（原dx/dz字段重命名）
    int                      d, perm;
    LONG64                   id;
    std::string              description;
    std::vector<std::string> memberXuids;

    // 便利函数
    [[nodiscard]] int  getWidth() const { return x_end - x; }               // 获取领地宽度
    [[nodiscard]] int  getHeight() const { return z_end - z; }              // 获取领地高度
    [[nodiscard]] int  getArea() const { return getWidth() * getHeight(); } // 获取领地面积
    [[nodiscard]] bool contains(int px, int pz) const {                     // 检查点是否在领地内
        return px >= x && px <= x_end && pz >= z && pz <= z_end;
    }

    // 构造函数
    BaseData() : x(0), z(0), x_end(0), z_end(0), d(0), perm(0), id(0) {}
};

class BaseInformation {
public:
    virtual ~BaseInformation() = default;

    // 基础数据访问接口（不暴露 BaseData）
    [[nodiscard]] int                             getX() const { return dataRef->x; }
    [[nodiscard]] int                             getZ() const { return dataRef->z; }
    [[nodiscard]] int                             getXEnd() const { return dataRef->x_end; }
    [[nodiscard]] int                             getZEnd() const { return dataRef->z_end; }
    [[nodiscard]] int                             getDimension() const { return dataRef->d; }
    [[nodiscard]] int                             getPermission() const { return dataRef->perm; }
    [[nodiscard]] LONG64                          getId() const { return dataRef->id; }
    [[nodiscard]] const std::string&              getDescription() const { return dataRef->description; }
    [[nodiscard]] const std::vector<std::string>& getMemberXuids() const { return dataRef->memberXuids; }

    // 计算属性
    [[nodiscard]] int  getWidth() const { return dataRef->x_end - dataRef->x; }
    [[nodiscard]] int  getHeight() const { return dataRef->z_end - dataRef->z; }
    [[nodiscard]] int  getArea() const { return getWidth() * getHeight(); }
    [[nodiscard]] bool contains(int px, int pz) const {
        return px >= dataRef->x && px <= dataRef->x_end && pz >= dataRef->z && pz <= dataRef->z_end;
    }

    // 公共功能方法
    [[nodiscard]] bool               hasBasicPermission(const std::string& xuid) const;
    [[nodiscard]] bool               isOwner(const std::string& xuid) const;
    [[nodiscard]] const std::string& getOwnerName() const { return ownerName; }
    [[nodiscard]] std::string        getMembers() const;

    // 设置接口（如果需要）
    void setPermission(int perm) { dataRef->perm = perm; }
    void setDescription(const std::string& desc) { dataRef->description = desc; }
    void addMember(const std::string& xuid);
    void removeMember(const std::string& xuid);
    void setId(LONG64 id) { dataRef->id = id; }
    void setCoordinates(int x, int z, int x_end, int z_end) {
        dataRef->x     = x;
        dataRef->z     = z;
        dataRef->x_end = x_end;
        dataRef->z_end = z_end;
    }

protected:
    explicit BaseInformation(BaseData& data);

    // 子类访问基础数据的保护方法
    BaseData&       getBaseData() { return *dataRef; }
    const BaseData& getBaseData() const { return *dataRef; }

    // 子类设置 ownerName 的保护方法
    void setOwnerName(const std::string& name) { ownerName = name; }
    
    // 子类更新 dataRef 的保护方法（用于构造函数中）
    void updateDataRef(BaseData& data) { dataRef = &data; }

    // 纯虚函数
    [[nodiscard]] virtual bool checkIsOwner(const std::string& xuid) const = 0;

private:
    BaseData*   dataRef;  // 指向派生类中的 BaseData 部分的引用
    std::string ownerName;
};

} // namespace rlx_land