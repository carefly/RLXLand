#include "LandCommands.h"
#include "common/LeviLaminaAPI.h"
#include "common/exceptions/LandExceptions.h"
#include "data/land/LandCore.h"
#include "data/service/DataService.h"
#include "data/spatial/SpatialMap.h"
#include "mod/town/Town.h"
#include "mod/town/permissions/TownPermissionChecker.h"
#include "service/PermissionService.h"


#include <basetsd.h>
#include <cstddef>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>
#include <ll/api/event/entity/ActorHurtEvent.h>
#include <ll/api/event/player/PlayerClickEvent.h>
#include <ll/api/event/player/PlayerDestroyBlockEvent.h>
#include <ll/api/event/player/PlayerInteractBlockEvent.h>
#include <ll/api/event/player/PlayerPlaceBlockEvent.h>
#include <ll/api/event/world/FireSpreadEvent.h>
#include <ll/api/service/Bedrock.h>
#include <mc/network/ServerPlayerBlockUseHandler.h>
#include <mc/server/ServerPlayer.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/block/block_events/BlockPlayerInteractEvent.h>

#include <ll/api/memory/Hook.h>
#include <mc/legacy/ActorUniqueID.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/ArmorStand.h>
#include <mc/world/actor/FishingHook.h>
#include <mc/world/actor/Mob.h>
#include <mc/world/events/PlayerEventListener.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/BlockType.h>
#include <mc/world/level/block/ItemFrameBlock.h>

#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandRawText.h>

#include <algorithm>
#include <string>
#include <unordered_map>


using namespace std;

namespace rlx_land {

#define LAND_RANGE 1000000

enum LandPerm : int {
    PERM_NULL           = 0,
    PERM_ATK            = 1,
    PERM_USE_ON         = 2,
    PERM_VILLAGER_ATK   = 4,
    PERM_BUILD          = 8,
    PERM_POPITEM        = 16,
    PERM_INTERWITHACTOR = 32,
    PERM_AMRORSTANDER   = 64,
    PERM_FISHINGHOOK    = 128,
    PERM_FIRE           = 256
};

static std::string showPerm(int perm, bool isOwner = false) {
    std::string permStr;

    // 如果是所有者，只需要显示特殊权限
    if (isOwner) {
        permStr = "所有基础权限 ";

        // 只显示所有者的特殊权限
        if (perm & LandPerm::PERM_VILLAGER_ATK) permStr += "攻击村民 ";
        if (perm & LandPerm::PERM_FIRE) permStr += "火焰蔓延 ";
    } else {
        // 非所有者，根据设置的权限显示
        if (perm & LandPerm::PERM_ATK) permStr += "攻击 ";
        if (perm & LandPerm::PERM_USE_ON) permStr += "使用方块 ";
        if (perm & LandPerm::PERM_VILLAGER_ATK) permStr += "攻击村民 ";
        if (perm & LandPerm::PERM_BUILD) permStr += "建造 ";
        if (perm & LandPerm::PERM_POPITEM) permStr += "物品框操作 ";
        if (perm & LandPerm::PERM_INTERWITHACTOR) permStr += "实体交互 ";
        if (perm & LandPerm::PERM_AMRORSTANDER) permStr += "盔甲架操作 ";
        if (perm & LandPerm::PERM_FISHINGHOOK) permStr += "钓鱼竿 ";
        if (perm & LandPerm::PERM_FIRE) permStr += "火焰蔓延 ";
    }

    if (permStr.empty()) permStr = "冒险 ";

    // 删除末尾的空格
    if (!permStr.empty() && permStr.back() == ' ') {
        permStr.pop_back();
    }

    return permStr;
}

enum LandBuyState : char {
    N = 0,
    A = 1,
    B = 2,
};

unordered_map<string, LandBuyState>   landBuyState;
unordered_map<string, pair<int, int>> landBuyA;
unordered_map<string, pair<int, int>> landBuyB;

void LandCommands::registerCommands() {
    using ll::command::CommandRegistrar;
    auto& landCommad = CommandRegistrar::getInstance().getOrCreateCommand("land", "领地");
    landCommad.overload<LandBasicCommad>()
        .required("Operation")
        .execute([](CommandOrigin const& origin, CommandOutput& output, LandBasicCommad const& param, Command const&) {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->isType(ActorType::Player)) {
                output.error("Only players can do");
                return;
            }
            auto* sp        = static_cast<Player*>(entity);
            auto  xuid      = sp->getXuid();
            auto  operation = param.Operation;

            if (LandCommandBasicOperation::a == operation) {
                landBuyState[xuid] = LandBuyState::A;
                output.success("点击方块选择领地A点");
            } else if (LandCommandBasicOperation::b == operation) {
                landBuyState[xuid] = LandBuyState::B;
                output.success("点击方块选择领地B点");
            } else if (LandCommandBasicOperation::exit == operation) {
                landBuyState.erase(xuid);
                landBuyA.erase(xuid);
                landBuyB.erase(xuid);
                output.success("退出领地选择模式");
            } else if (LandCommandBasicOperation::buy == operation) {
                auto ita = landBuyA.find(xuid);
                auto itb = landBuyB.find(xuid);

                if (ita == landBuyA.end()) output.error("land A 为空");
                if (itb == landBuyB.end()) output.error("land B 为空");

                if (ita == landBuyA.end() || itb == landBuyB.end()) return;

                int x, z, dx, dz, d;
                d  = sp->getDimensionId();
                x  = min(ita->second.first, itb->second.first);
                dx = max(ita->second.first, itb->second.first);
                z  = min(ita->second.second, itb->second.second);
                dz = max(ita->second.second, itb->second.second);

                int area = (dx - x) * (dz - z);

                // 检查是否超出LAND_RANGE范围
                if (x >= LAND_RANGE || x <= -LAND_RANGE || dx >= LAND_RANGE || dx <= -LAND_RANGE || z >= LAND_RANGE
                    || z <= -LAND_RANGE || dz >= LAND_RANGE || dz <= -LAND_RANGE) {
                    output.error(format("领地坐标超出范围，坐标范围不能超过 +/-{}", LAND_RANGE));
                    return;
                }

                if ((abs(x) > LAND_RANGE) || (abs(z) > LAND_RANGE)) {
                    output.error(format("领地不能超过 {} 格", LAND_RANGE));
                    return;
                }

                // 添加Town权限检查逻辑（符合原始设计意图）
                bool canClaim = true;
                // 检查整个领地区域是否可以圈地（确保所有坐标点都有权限）
                for (int xi = x; xi <= dx; xi++) {
                    for (int zi = z; zi <= dz; zi++) {
                        if (!TownPermissionChecker::canClaimLand(sp, xi, zi, d)) {
                            canClaim = false;
                            break;
                        }
                    }
                    if (!canClaim) break;
                }

                if (!canClaim) {
                    output.error("您没有在此区域圈地的权限");
                    return;
                }

                landBuyState.erase(xuid);
                landBuyA.erase(xuid);
                landBuyB.erase(xuid);

                for (int xi = x; xi <= dx; xi++)
                    for (int zi = z; zi <= dz; zi++) {
                        auto li = LandMap::getInstance()->find(xi, zi, d);
                        if (NULL != li) {
                            output.error(format("领地冲突 x:{} z:{} 领地所有者 {} 请重新圈地", xi, zi, li->ownerName));
                            return;
                        }
                    }

                int pay = area;
                // auto res = RLXMoney::getInstance().addMoney(xuid, -pay);
                // if (!res) {
                //     output.error("钱不够了，缺钱可以找腐竹免费领取");
                //     return;
                // }

                LandData data;
                data.x           = x;
                data.z           = z;
                data.dx          = dx;
                data.dz          = dz;
                data.ownerXuid   = xuid;
                data.d           = d;
                data.perm        = 0;
                data.description = "";
                data.memberXuids = {};
                data.id          = DataService::getMaxId<LandData, LandInformation>() + 1;
                // 删除townIds字段，因为Land不需要和任何Town关联
                DataService::getInstance()->createItem<LandData, LandInformation>(data);

                output.success(format("买入领地成功，领地面积为 {}，共花费 {} 元", area, pay));
            } else if (LandCommandBasicOperation::sell == operation) {
                auto pos = sp->getPosition();
                auto li  = LandMap::getInstance()->find((LONG64)pos.x, (LONG64)pos.z, sp->getDimensionId());

                if ((nullptr == li || !li->isOwner(xuid)) && !PermissionService::getInstance().isOperator(sp)) {
                    output.error("该位置不是你的领地");
                    return;
                }
                int pay = (li->ld.dx - li->ld.x) * (li->ld.dz - li->ld.z);
                // RLXMoney::getInstance().addMoney(li->ld.ownerXuid, pay);
                DataService::getInstance()->deleteItem<LandData, LandInformation>(li->ld);
                output.success(format("领地卖出成功，共获得 {} 元", pay));
            } else if (LandCommandBasicOperation::query == operation) {
                auto pos  = sp->getPosition();
                auto li   = LandMap::getInstance()->find((int)pos.x, (int)pos.z, sp->getDimensionId());
                auto town = Town::getInstance().getTownAt(pos, sp->getDimensionId());

                std::string info;
                info += "当前位置: x=" + std::to_string((int)pos.x) + ", z=" + std::to_string((int)pos.z)
                      + ", dimension=" + std::to_string(sp->getDimensionId()) + "\n";

                if (nullptr == li) {
                    // 不在任何领地内
                    info += "§b[领地信息]§r 公共用地\n";
                } else {
                    // 在某个领地内
                    info += "§b[领地信息]§r " + li->ld.description + "\n";
                    info += "  所有人: §6" + li->ownerName + "§r\n";
                    info += "  所有者权限: §6" + showPerm(li->ld.perm, true) + "§r\n";
                    info += "  其他用户权限: §6" + showPerm(li->ld.perm, false) + "§r\n";
                    if (!li->getMembers().empty()) {
                        info += "  成员列表: §6" + li->getMembers() + "§r\n";
                    }
                }

                // 显示城镇信息
                if (town == nullptr) {
                    info += "§b[城镇信息]§r 公共区域（未被任何城镇管辖）\n";
                } else {
                    info += "§b[城镇信息]§r " + town->td.name + "\n";
                    info += "  镇长: §6" + town->mayorName + "§r\n";
                    if (!town->getMembers().empty()) {
                        info += "  成员列表: §6" + town->getMembers() + "§r\n";
                    }
                }

                output.success(info);
            }
        });
    landCommad.overload<LandTrustCommand>()
        .required("Operation")
        .required("Name")
        .execute([](CommandOrigin const& origin, CommandOutput& output, LandTrustCommand const& param, Command const&) {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->isType(ActorType::Player)) {
                output.error("Only players can do");
                return;
            }
            auto* sp         = static_cast<Player*>(entity);
            auto  xuid       = sp->getXuid();
            auto  operation  = param.Operation;
            auto  playerName = param.Name.mText;

            if (LandCommandTrustOperation::trust == operation) {

                auto pos = sp->getPosition();
                auto li  = LandMap::getInstance()->find((int)pos.x, (int)pos.z, sp->getDimensionId());

                if (li == NULL || (!li->isOwner(xuid) && !PermissionService::getInstance().isOperator(sp))) {
                    output.error("你不是领地主人");
                    return;
                }


                auto memberXuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
                if ("" == memberXuid) {
                    output.error(format("找不到玩家 {}， 请检查玩家ID拼写", playerName));
                    return;
                }

                auto memberName = LeviLaminaAPI::getPlayerNameByXuid(memberXuid);

                try {
                    DataService::getInstance()->addItemMember<LandData, LandInformation>(li, memberName);
                } catch (PlayerNotFoundException&) {
                    output.error("玩家 {} 不存在", memberName);
                } catch (DuplicateException&) {
                    output.error("玩家 {} 已经是领地成员", memberName);
                }

                output.success(format("成功添加玩家 {} 为领地成员", playerName));

            } else if (LandCommandTrustOperation::untrust == operation) {
                auto pos = sp->getPosition();
                auto li  = LandMap::getInstance()->find((int)pos.x, (int)pos.z, sp->getDimensionId());

                if (li == NULL || (!li->isOwner(xuid) && !PermissionService::getInstance().isOperator(sp))) {
                    output.error("你不是领地主人");
                    return;
                }

                auto memberXuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
                if ("" == memberXuid) {
                    output.error(format("找不到玩家 {}， 请检查玩家ID拼写", playerName));
                    return;
                }

                try {
                    DataService::getInstance()->removeItemMember<LandData, LandInformation>(li, playerName);
                } catch (PlayerNotFoundException&) {
                    output.error("玩家 {} 不存在", playerName);
                } catch (NotMemberException&) {
                    output.error(format("玩家 {} 不是领地成员", playerName));
                }

                output.success(format("成功删除领地成员 {}", playerName));
            }
        });

    landCommad.overload<LandPermCommand>()
        .required("Operation")
        .required("Perm")
        .execute([](CommandOrigin const& origin, CommandOutput& output, LandPermCommand const& param, Command const&) {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->isType(ActorType::Player)) {
                output.error("Only players can do");
                return;
            }
            auto* sp        = static_cast<Player*>(entity);
            auto  xuid      = sp->getXuid();
            auto  operation = param.Operation;
            auto  perm_num  = param.Perm;

            if (LandCommandPermOperation::perm == operation) {
                auto pos = sp->getPosition();
                auto li  = LandMap::getInstance()->find((int)pos.x, (int)pos.z, sp->getDimensionId());

                if (li == NULL || (!li->isOwner(xuid) && !PermissionService::getInstance().isOperator(sp))) {
                    output.error("你不是领地主人");
                    return;
                }

                if (perm_num < 0) {
                    output.error("perm 不能小于0");
                    return;
                }

                DataService::getInstance()->modifyItemPermission<LandData, LandInformation>(li, perm_num);
                output.success(format("领地权限更改为 {}", perm_num));
            }
        });
}

} // namespace rlx_land
