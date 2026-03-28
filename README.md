# RLXLand

<div align="center">

**一个功能强大的 Minecraft 基岩版土地与城镇管理模组**

[![LeviLamina](https://img.shields.io/badge/LeviLamina-1.9.8-blue)](https://github.com/LiteLDev/LeviLamina)
[![License](https://img.shields.io/badge/License-CC0--1.0-lightgrey)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows-blue)](https://github.com/LiteLDev/LeviLamina)

</div>

---

## 📖 项目简介

RLXLand 是一个基于 **LeviLamina** 的 Minecraft 基岩版模组，提供完整的土地（Land）和城镇（Town）管理系统。玩家可以圈地保护自己的建筑、组建城镇、管理权限，并对接经济系统实现土地买卖功能。

### 核心特性

- **🏔️ 土地管理系统**
  - 使用 `/land a` 和 `/land b` 选择矩形区域
  - 支持土地购买、出售、转让
  - 细粒度权限控制（建造、破坏、容器、交互等）
  - 成员管理系统（添加/移除信任玩家）

- **🏘️ 城镇系统**
  - 管理员可创建城镇，指定镇长
  - 三级权限结构：服务器 → 城镇/野外 → 土地
  - 城镇成员可在城镇内圈地
  - 城镇默认权限设置

- **💰 经济集成**
  - 集成 RLXMoney 经济系统
  - 基于面积的土地定价
  - 可配置的初始金钱

- **⚡ 快速响应**
  - 即使在大地图上也能快速查询土地信息
  - 智能内存管理，不占用过多服务器资源
  - 操作流畅，不会造成服务器卡顿

- **🛡️ 权限系统**
  - 9 种权限节点（攻击、建造、破坏、交互等）
  - 位运算权限组合
  - 多级权限继承（OP > 土地所有者 > 土地成员 > 访客）

---

## 🚀 安装使用

### 安装步骤

1. 从 [Releases](https://github.com/carefly/RLXLand/releases) 下载最新版本
2. 将下载的文件解压到 LeviLamina 服务器的 `plugins/` 目录
3. 重启服务器即可

---

### 开发者构建

如果你想自行编译，需要以下环境：
- Windows 10+ / Windows Server 2022
- Visual Studio 2022
- xmake

```bash
# 克隆仓库
git clone https://github.com/carefly/RLXLand.git
cd RLXLand

# 配置并构建
xmake f -y -p windows -a x64 -m release
xmake
```

构建产物位于 `bin/` 目录。

---

## 🎮 使用指南

### 玩家命令

| 命令 | 说明 |
|------|------|
| `/land a` | 选择土地的第一个角点 |
| `/land b` | 选择土地的第二个角点 |
| `/land buy` | 购买选定区域作为土地 |
| `/land sell` | 出售当前所在的土地 |
| `/land query` | 查询当前所在土地信息 |
| `/land trust <玩家>` | 添加玩家为当前土地的成员 |
| `/land untrust <玩家>` | 从当前土地移除成员 |
| `/land perm <权限值>` | 设置当前土地的权限 |
| `/land exit` | 退出土地选择模式 |

### 管理员命令

#### 土地管理
- `/land create <xuid>` - 强制为玩家创建土地
- `/land delete <id>` - 删除指定土地

#### 城镇管理
| 命令 | 说明 |
|------|------|
| `/town create <town_name> [mayor]` | 创建城镇（可指定镇长） |
| `/town delete <town_name>` | 删除城镇 |
| `/town list` | 列出所有城镇 |
| `/town transfer <town_name> <new_mayor>` | 转让镇长权限 |

#### 城镇成员命令
| 命令 | 说明 |
|------|------|
| `/town add <player>` | 添加成员到当前城镇 |
| `/town remove <player>` | 从当前城镇移除成员 |
| `/town setperm <perm> <true/false>` | 设置城镇默认权限 |
| `/town info` | 查看当前所在城镇的信息 |

---

## ⚙️ 配置说明

配置文件位于模组目录下的 `config/land_config.json`：

```json
{
  "enableSidebar": true,              // 启用侧边栏显示
  "enableEconomy": true,              // 是否启用经济系统（需要安装 RLXMoney 插件）
  "playerInitialMoney": 100000        // 玩家初始金钱（已废弃，由 RLXMoney 插件管理）
}
```

### 经济系统说明

- **启用经济系统** (`enableEconomy: true`)：
  - 需要安装 [RLXMoney](https://github.com/carefly/RLXMoney) 插件
  - 土地购买需要花钱
  - 玩家初始余额由 RLXMoney 插件管理

- **禁用经济系统** (`enableEconomy: false`)：
  - 不需要安装 RLXMoney 插件
  - 土地免费申请
  - 适合纯生存或创意模式服务器

### 权限节点

| 权限值 | 权限节点 | 说明 |
|--------|----------|------|
| 1 | PERM_ATK | 攻击玩家 |
| 2 | PERM_USE_ON | 使用方块 |
| 4 | PERM_VILLAGER_ATK | 攻击村民 |
| 8 | PERM_BUILD | 建造（放置方块） |
| 16 | PERM_POPITEM | 物品展示框操作 |
| 32 | PERM_INTERWITHACTOR | 与实体交互 |
| 64 | PERM_ARMORSTANDER | 盔甲架操作 |
| 128 | PERM_FISHINGHOOK | 钓鱼竿使用 |
| 256 | PERM_FIRE | 火焰蔓延控制 |

权限可以通过位运算组合使用，例如 `perm = 8 | 16` 表示同时拥有建造和物品展示框权限。


---

## 📞 联系方式

如有问题或建议，欢迎通过以下方式联系：

- 提交 [Issue](https://github.com/carefly/RLXLand/issues)

---
