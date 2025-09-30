#pragma once


namespace rlx_land {

class Land {

public:
    static Land& getInstance();

    /// @return True if the plugin is loaded successfully.
    bool load();

    /// @return True if the plugin is enabled successfully.
    bool enable();

    /// @return True if the plugin is disabled successfully.
    bool disable();
};

} // namespace rlx_land