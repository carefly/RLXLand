#pragma once

#include <chrono>

namespace rlx_land {

class RLXStatus {
public:
    static RLXStatus& getInstance();

    bool load();
    bool enable();
    bool disable();

    void oneTick();

private:
    RLXStatus()                            = default;
    ~RLXStatus()                           = default;
    RLXStatus(const RLXStatus&)            = delete;
    RLXStatus& operator=(const RLXStatus&) = delete;

    std::chrono::steady_clock::time_point lastTpsCalculationTime_;
    int                                   tickCount_ = 0;
};

} // namespace rlx_land
