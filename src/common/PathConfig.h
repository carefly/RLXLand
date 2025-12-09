#pragma once
#include <string>


namespace rlx_land {

class PathConfig {
public:
    // 领地数据目录
    static const std::string LANDS_BASE_DIR;

    // 城镇数据目录
    static const std::string TOWNS_BASE_DIR;

    // 城镇数据文件
    static const std::string TOWNS_JSON_FILE;
};

} // namespace rlx_land