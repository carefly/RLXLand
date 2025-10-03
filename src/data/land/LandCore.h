#pragma once

#include "data/core/BaseInformation.h"
#include <basetsd.h>
#include <string>


namespace rlx_land {

class LandData : public BaseData {
public:
    std::string ownerXuid;
};

class LandInformation : public BaseInformation {
public:
    explicit LandInformation(LandData ld);
    LandData ld;

    std::string ownerName;

protected:
    [[nodiscard]] bool checkIsOwner(const std::string& xuid) const override;
};

} // namespace rlx_land
