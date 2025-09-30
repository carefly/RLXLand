#pragma once

#include "data/BaseInformation.h"
#include <basetsd.h>
#include <string>
#include <vector>


namespace rlx_land {

#define TOWN_BIG_SIZE    100000
#define TOWN_MIDDLE_SIZE 10000
#define TOWN_SMALL_SIZE  100


class TownData : public BaseData {
public:
    std::string              name;
    std::string              mayorXuid;
    std::vector<std::string> memberXuids;
    int                      perm;
    int                      x, z, dx, dz, d;
    std::string              description;
};

class TownInformation : public BaseInformation {
public:
    explicit TownInformation(TownData td);

protected:
    [[nodiscard]] bool checkIsOwner(const std::string& xuid) const override;

public:
    TownData    td;
    std::string mayorName;

    [[nodiscard]] bool isMayor(const std::string& xuid) const;
};

} // namespace rlx_land