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
    [[nodiscard]] int                             getX() const { return data.x; }
    [[nodiscard]] int                             getZ() const { return data.z; }
    [[nodiscard]] int                             getXEnd() const { return data.x_end; }
    [[nodiscard]] int                             getZEnd() const { return data.z_end; }
    [[nodiscard]] int                             getDimension() const { return data.d; }
    [[nodiscard]] int                             getPermission() const { return data.perm; }
    [[nodiscard]] LONG64                          getId() const { return data.id; }
    [[nodiscard]] const std::string&              getDescription() const { return data.description; }
    [[nodiscard]] const std::vector<std::string>& getMemberXuids() const { return data.memberXuids; }

    // 计算属性
    [[nodiscard]] int  getWidth() const { return data.x_end - data.x; }
    [[nodiscard]] int  getHeight() const { return data.z_end - data.z; }
    [[nodiscard]] int  getArea() const { return getWidth() * getHeight(); }
    [[nodiscard]] bool contains(int px, int pz) const {
        return px >= data.x && px <= data.x_end && pz >= data.z && pz <= data.z_end;
    }

    // 公共功能方法
    [[nodiscard]] bool               hasBasicPermission(const std::string& xuid) const;
    [[nodiscard]] bool               isOwner(const std::string& xuid) const;
    [[nodiscard]] const std::string& getOwnerName() const { return ownerName; }
    [[nodiscard]] std::string        getMembers() const;

    // 设置接口（如果需要）
    void setPermission(int perm) { data.perm = perm; }
    void setDescription(const std::string& desc) { data.description = desc; }
    void addMember(const std::string& xuid);
    void removeMember(const std::string& xuid);
    void setId(LONG64 id) { data.id = id; }
    void setCoordinates(int x, int z, int x_end, int z_end) {
        data.x     = x;
        data.z     = z;
        data.x_end = x_end;
        data.z_end = z_end;
    }

protected:
    explicit BaseInformation(BaseData data);

    // 子类访问基础数据的保护方法
    BaseData&       getBaseData() { return data; }
    const BaseData& getBaseData() const { return data; }

    // 子类设置 ownerName 的保护方法
    void setOwnerName(const std::string& name) { ownerName = name; }

    // 纯虚函数
    [[nodiscard]] virtual bool checkIsOwner(const std::string& xuid) const = 0;

private:
    BaseData    data;
    std::string ownerName;
};

} // namespace rlx_land