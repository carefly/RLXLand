#pragma once
#include <string>


namespace rlx_land::test {

// 前向声明，避免包含真实的Player头文件
class Player;

class MockPlayer {
public:
    MockPlayer(const std::string& name, const std::string& xuid);

    // 添加默认构造函数
    MockPlayer() = default;

    // 添加拷贝构造函数和拷贝赋值操作符
    MockPlayer(const MockPlayer& other)            = default;
    MockPlayer& operator=(const MockPlayer& other) = default;

    // 添加移动构造函数和移动赋值操作符
    MockPlayer(MockPlayer&& other) noexcept            = default;
    MockPlayer& operator=(MockPlayer&& other) noexcept = default;

    // 模拟Player接口
    std::string getName() const { return m_name; }
    std::string getXuid() const { return m_xuid; }
    bool        isOperator() const { return m_isOperator; }

    // 测试辅助方法
    void setOperator(bool isOp) { m_isOperator = isOp; }
    void setPosition(int x, int y, int z);
    void setDimension(int dimension) { m_dimension = dimension; }

    // 获取位置信息
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getZ() const { return m_z; }
    int getDimension() const { return m_dimension; }

    // 获取模拟的Player指针（用于测试）
    // 注意：这里返回nullptr，因为我们只是模拟
    Player* getMockPlayerPtr() { return nullptr; }

private:
    std::string m_name;
    std::string m_xuid;
    bool        m_isOperator = false;
    int         m_x          = 0;
    int         m_y          = 64;
    int         m_z          = 0;
    int         m_dimension  = 0;
};

} // namespace rlx_land::test