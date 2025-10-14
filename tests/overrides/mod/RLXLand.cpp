#include "mod/RLXLand.h"

namespace rlx_land {

RLXLand& RLXLand::getInstance() {
    static RLXLand instance;
    return instance;
}

} // namespace rlx_land