#include "LandCommands.h"
#include "common/LeviLaminaAPI.h"
#include "common/PlayerInfoUtils.h"
#include "common/exceptions/LandExceptions.h"
#include "data/service/DataService.h"
#include "mod/town/Town.h"
#include "service/EconomyService.h"
#include "service/PermissionService.h"

#include <basetsd.h>
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

                int d           = sp->getDimensionId();
                auto [x, x_end] = std::minmax(ita->second.first, itb->second.first);
                auto [z, z_end] = std::minmax(ita->second.second, itb->second.second);
                int     area    = (x_end - x) * (z_end - z);
                int64_t pay     = EconomyService::getLandPurchaseCost(area); // 使用经济服务计算费用

                landBuyState.erase(xuid);
                landBuyA.erase(xuid);
                landBuyB.erase(xuid);

                // 检查玩家是否有足够的金钱
                if (!EconomyService::hasSufficientFunds(xuid, pay)) {
                    output.error("金钱不足，无法购买领地，需要 {} 金币", pay);
                    return;
                }

                // 扣除玩家金钱
                if (!EconomyService::deductLandPurchaseFee(xuid, pay)) {
                    output.error("扣款失败，请联系管理员");
                    return;
                }

                LandData data(x, z, x_end, z_end, xuid, d, DataService::getMaxId<LandData>() + 1);

                try {
                    // 创建PlayerInfo结构
                    auto playerInfo = PlayerInfoUtils::fromXuid(sp->getXuid());

                    // 使用统一的createItem方法创建土地
                    DataService::getInstance()->createItem<LandData>(data, playerInfo);
                    output.success(format("买入领地成功，领地面积为 {}，共花费 {} 元", area, pay));
                } catch (const RealmOutOfRangeException& e) {
                    // 如果创建失败，退还玩家金钱
                    EconomyService::addIncome(xuid, pay);
                    output.error(e.what());
                } catch (const RealmPermissionException& e) {
                    // 如果创建失败，退还玩家金钱
                    EconomyService::addIncome(xuid, pay);
                    output.error(e.what());
                } catch (const RealmConflictException& e) {
                    // 如果创建失败，退还玩家金钱
                    EconomyService::addIncome(xuid, pay);
                    output.error(e.what());
                } catch (const std::exception& e) {
                    // 如果创建失败，退还玩家金钱
                    EconomyService::addIncome(xuid, pay);
                    output.error(format("创建领地时发生错误: {}", e.what()));
                }
            } else if (LandCommandBasicOperation::sell == operation) {
                auto pos = sp->getPosition();
                auto li  = DataService::getInstance()->findLandAt((LONG64)pos.x, (LONG64)pos.z, sp->getDimensionId());

                if ((nullptr == li || !li->isOwner(xuid)) && !PermissionService::getInstance().isOperator(sp)) {
                    output.error("该位置不是你的领地");
                    return;
                }
                int64_t pay = EconomyService::getLandPurchaseCost(li->getArea()); // 使用经济服务计算费用
                // 增加玩家金钱
                EconomyService::addIncome(li->getOwnerXuid(), pay);
                // 使用新的坐标参数删除领地
                DataService::getInstance()->deleteItem<LandData>((LONG64)pos.x, (LONG64)pos.z, sp->getDimensionId());
                output.success(format("领地卖出成功，共获得 {} 元", pay));
            } else if (LandCommandBasicOperation::query == operation) {
                auto pos  = sp->getPosition();
                auto li   = DataService::getInstance()->findLandAt((int)pos.x, (int)pos.z, sp->getDimensionId());
                auto town = DataService::getInstance()->findTownAt((LONG64)pos.x, (LONG64)pos.z, sp->getDimensionId());

                std::string info;
                info += "当前位置: x=" + std::to_string((int)pos.x) + ", z=" + std::to_string((int)pos.z)
                      + ", dimension=" + std::to_string(sp->getDimensionId()) + "\n";

                if (nullptr == li) {
                    // 不在任何领地内
                    info += "§b[领地信息]§r 公共用地\n";
                } else {
                    // 在某个领地内
                    info += "§b[领地信息]§r " + li->getDescription() + "\n";
                    info += "  所有人: §6" + li->getOwnerName() + "§r\n";
                    info += "  所有者权限: §6" + showPerm(li->getPermission(), true) + "§r\n";
                    info += "  其他用户权限: §6" + showPerm(li->getPermission(), false) + "§r\n";
                    if (!li->getMembers().empty()) {
                        info += "  成员列表: §6" + li->getMembers() + "§r\n";
                    }
                }

                // 显示城镇信息
                if (town == nullptr) {
                    info += "§b[城镇信息]§r 公共区域（未被任何城镇管辖）\n";
                } else {
                    info += "§b[城镇信息]§r " + town->getTownName() + "\n";
                    info += "  镇长: §6" + town->getOwnerName() + "§r\n";
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
                try {
                    auto pos           = sp->getPosition();
                    auto currentPlayer = PlayerInfoUtils::fromXuid(sp->getXuid());

                    // 使用新的带验证的addItemMember接口
                    DataService::getInstance()->addItemMember<LandData>(
                        (int)pos.x,
                        (int)pos.z,
                        sp->getDimensionId(),
                        currentPlayer,
                        playerName
                    );

                    output.success(format("成功添加玩家 {} 为领地成员", playerName));

                } catch (const RealmPermissionException& e) {
                    output.error(e.what());
                } catch (const PlayerNotFoundException& e) {
                    output.error(e.what());
                } catch (const DuplicateException&) {
                    output.error("玩家 {} 已经是领地成员", playerName);
                }

            } else if (LandCommandTrustOperation::untrust == operation) {
                try {
                    auto pos           = sp->getPosition();
                    auto currentPlayer = PlayerInfoUtils::fromXuid(sp->getXuid());

                    // 使用新的带验证的removeItemMember接口
                    DataService::getInstance()->removeItemMember<LandData>(
                        (int)pos.x,
                        (int)pos.z,
                        sp->getDimensionId(),
                        currentPlayer,
                        playerName
                    );

                    output.success(format("成功删除领地成员 {}", playerName));

                } catch (const RealmPermissionException& e) {
                    output.error(e.what());
                } catch (const PlayerNotFoundException& e) {
                    output.error(e.what());
                } catch (const NotMemberException&) {
                    output.error(format("玩家 {} 不是领地成员", playerName));
                }
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

            if (LandCommandPermOperation::perm_operation == operation) {
                try {
                    auto pos           = sp->getPosition();
                    auto currentPlayer = PlayerInfoUtils::fromXuid(sp->getXuid());

                    DataService::getInstance()->modifyItemPermission<LandData>(
                        (int)pos.x,
                        (int)pos.z,
                        sp->getDimensionId(),
                        perm_num,
                        currentPlayer
                    );
                    output.success(format("领地权限更改为 {}", perm_num));

                } catch (const RealmPermissionException& e) {
                    output.error(e.what());
                } catch (const InvalidPermissionException& e) {
                    output.error(e.what());
                }
            }
        });
}

} // namespace rlx_land
