#pragma once
#include <string>


namespace rlx_land {

class PathConfig {
public:
    // 基础数据目录
    static const std::string BASE_DATA_DIR;
    
    // 领地数据目录
    static const std::string LANDS_BASE_DIR;
    
    // 城镇数据目录
    static const std::string TOWNS_BASE_DIR;
    
    // 城镇数据文件
    static const std::string TOWNS_JSON_FILE;
    
    // 经济数据目录
    static const std::string ECONOMY_BASE_DIR;
    
    // 经济数据文件
    static const std::string ECONOMY_JSON_FILE;
    
    // 确保目录存在
    static void ensureDataDirectoriesExist();
};

} // namespace rlx_land