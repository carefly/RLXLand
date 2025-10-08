#include "MockPlayer.h"

namespace rlx_land::test {

MockPlayer::MockPlayer(const std::string& name, const std::string& xuid) : m_name(name), m_xuid(xuid) {}

void MockPlayer::setPosition(int x, int y, int z) {
    m_x = x;
    m_y = y;
    m_z = z;
}

} // namespace rlx_land::test