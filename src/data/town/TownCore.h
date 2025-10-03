#pragma once

#include "../core/BaseInformation.h"
#include <basetsd.h>
#include <string>


namespace rlx_land {

class TownData : public BaseData {
public:
    std::string name;
    std::string mayorXuid;
};

class TownInformation : public BaseInformation {
public:
    explicit TownInformation(TownData td);

protected:
    [[nodiscard]] bool checkIsOwner(const std::string& xuid) const override;

public:
    TownData    td;
    std::string mayorName;
};

} // namespace rlx_land