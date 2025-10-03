#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace rlx_land {

typedef int64_t LONG64;

class BaseData {
public:
    int                      x, z, dx, dz, d, perm;
    LONG64                   id;
    std::string              description;
    std::vector<std::string> memberXuids;
};

class BaseInformation {
public:
    explicit BaseInformation(BaseData data) : data(std::move(data)) {}

    virtual ~BaseInformation() = default;

    [[nodiscard]] bool hasBasicPermission(const std::string& xuid) const;
    [[nodiscard]] bool isOwner(const std::string& xuid) const;

    [[nodiscard]] std::string getOwnerName() const;
    [[nodiscard]] std::string getMembers() const;

    BaseData    data;
    std::string ownerName;

protected:
    [[nodiscard]] virtual bool checkIsOwner(const std::string& xuid) const = 0;
};

} // namespace rlx_land