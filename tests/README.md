# RLXLand 集成测试

本文档描述了RLXLand项目的集成测试框架和使用方法。

## 目录结构

```
tests/
├── README.md                    # 本文档
├── run_tests.bat               # Windows批处理测试脚本
├── main.cpp                    # 测试主入口
├── catch2/                     # Catch2测试框架
│   └── catch.hpp               # Catch2单头文件
├── mocks/                      # 模拟类
│   ├── MockPlayer.h
│   └── MockPlayer.cpp
├── integration/                # 集成测试
│   └── land_management_test.cpp
├── fixtures/                   # 测试数据
│   └── test_land_data.json
├── utils/                      # 测试工具类
│   ├── TestHelper.h
│   ├── TestHelper.cpp
│   ├── TestDataLoader.h
│   ├── TestDataLoader.cpp
│   ├── TestEnvironment.h
│   └── TestEnvironment.cpp
└── reports/                    # 测试报告目录
    └── test_report.xml
```

## 快速开始

### 1. 环境准备

确保已安装以下依赖：
- xmake (构建系统)
- Visual Studio 2022 或更新版本
- Windows 10/11 (64位)

### 2. 运行测试

#### 方法一：使用批处理脚本（推荐）

```bash
# 基本用法
tests\run_tests.bat

# 调试构建，详细输出
tests\run_tests.bat --debug --verbose

# 清理并重新构建
tests\run_tests.bat --clean

# 查看帮助
tests\run_tests.bat --help
```

#### 方法二：手动运行

```bash
# 配置项目
xmake f -y -p windows -a x64 -m release --tests=y

# 构建测试
xmake build RLXLand_tests

# 运行测试
xmake run RLXLand_tests
```

## 测试模块

### 1. 土地管理集成测试 (`land_management_test.cpp`)

测试土地管理的核心功能：
- 土地创建和数据验证
- 成员管理（添加/移除成员）
- 数据加载和一致性检查
- 性能测试

**测试标签**: `[land]`, `[integration]`

### 测试用例说明

#### Basic Land Creation
- 验证土地数据创建的正确性
- 检查所有者权限验证
- 测试土地坐标和尺寸设置

#### Member Management
- 测试成员添加功能
- 测试成员移除功能
- 验证权限继承机制

#### Data Loading
- 测试从JSON文件加载测试数据
- 验证数据一致性
- 检查数据完整性

#### Performance Test
- 批量创建土地性能测试
- 数据完整性验证
- 执行时间监控

## 测试数据

测试数据存储在 `tests/fixtures/` 目录下：

- `test_land_data.json` - 土地测试数据

### 测试数据结构

```json
{
  "test_lands": [
    {
      "id": 1001,
      "owner_xuid": "test_owner_001",
      "owner_name": "TestOwnerOne",
      "x": 1000,
      "z": 1000,
      "dx": 50,
      "dz": 50,
      "dimension": 0,
      "permission_level": 3,
      "description": "Test land for basic functionality",
      "members": ["test_member_001", "test_member_002"]
    }
  ]
}
```

## 运行特定测试

### 按模块运行

```bash
# 只运行土地管理测试
xmake run RLXLand_tests -- "[land]"

# 只运行集成测试
xmake run RLXLand_tests -- "[integration]"
```

### 按测试名称运行

```bash
# 运行特定测试用例
xmake run RLXLand_tests -- "Basic Land Creation"

# 运行包含特定关键词的测试
xmake run RLXLand_tests -- "*Performance*"
```

## 测试报告

### 生成XML报告

```bash
xmake run RLXLand_tests -- --reporter=xml --out=tests/reports/test_report.xml
```

报告将保存在 `tests/reports/test_report.xml`

## 性能基准

当前性能基准（在标准开发机器上）：

| 测试类型 | 操作数量 | 时间阈值 |
|---------|---------|---------|
| 土地创建 | 100 | < 100ms |
| 数据加载 | 批量 | < 50ms |

## 故障排除

### 常见问题

1. **构建失败**
   - 检查Visual Studio版本
   - 确保xmake正确安装
   - 清理构建目录后重试

2. **测试运行失败**
   - 检查测试数据文件是否存在
   - 确保临时目录有写权限
   - 查看详细错误信息

3. **Catch2头文件问题**
   - 确保 `tests/catch2/catch.hpp` 文件存在
   - 检查文件路径是否正确

### 调试技巧

1. **启用详细输出**
   ```bash
   tests\run_tests.bat --verbose
   ```

2. **使用调试构建**
   ```bash
   tests\run_tests.bat --debug
   ```

3. **运行单个测试**
   ```bash
   xmake run RLXLand_tests -- "Specific Test Name"
   ```

## 扩展测试

### 添加新测试

1. 在 `tests/integration/` 目录下创建新的测试文件
2. 使用适当的测试标签
3. 添加测试数据到 `fixtures/` 目录
4. 更新本文档

### 测试命名规范

- 测试文件: `module_name_test.cpp`
- 测试用例: `Descriptive Test Name`
- 测试标签: `[module]`, `[integration]`, `[performance]`

### 代码风格

- 使用Catch2框架的BDD风格
- 每个测试用例应该独立
- 使用SECTION进行逻辑分组
- 添加适当的注释和文档

## 测试基础设施

### TestEnvironment
- 管理测试环境初始化和清理
- 设置测试数据路径
- 创建必要的目录结构

### TestDataLoader
- 加载和管理测试数据
- 创建动态测试数据
- 提供数据清理功能

### TestHelper
- 提供测试辅助函数
- 性能测量工具
- 文件操作辅助

### MockPlayer
- 模拟玩家对象
- 提供测试所需的玩家接口
- 支持位置和权限设置

## 联系方式

如有问题或建议，请：
1. 查看本文档的故障排除部分
2. 检查项目的GitHub Issues
3. 联系开发团队

---

*最后更新: 2024年1月*