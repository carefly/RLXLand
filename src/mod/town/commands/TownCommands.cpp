#include "TownCommands.h"
#include "common/JsonLoader.h"
#include "common/LeviLaminaAPI.h"
#include "common/PlayerInfoUtils.h"
#include "common/exceptions/LandExceptions.h"
#include "data/service/DataService.h"
#include "service/PermissionService.h"

#include <basetsd.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>
#include <ll/api/service/Bedrock.h>
#include <mc/server/ServerPlayer.h>

#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandRawText.h>

#include <string>

using namespace std;

namespace rlx_town {

void TownCommands::registerCommands() {
    using ll::command::CommandRegistrar;
    auto& townCommand = CommandRegistrar::getInstance(false).getOrCreateCommand("town", "城镇系统");

    // 注册腐竹专用命令，使用GameDirectors权限，普通玩家看不到
    auto& opCommand = CommandRegistrar::getInstance(false)
                          .getOrCreateCommand("townop", "腐竹城镇管理系统", CommandPermissionLevel::GameDirectors);

    // 基础命令（腐竹专用）
    opCommand.overload<TownBasicCommand>()
        .required("Operation")
        .optional("TownName")
        .optional("PlayerName")
        .optional("X")
        .optional("Z")
        .optional("DX")
        .optional("DZ")
        .execute([](CommandOrigin const& origin, CommandOutput& output, TownBasicCommand const& param, Command const&) {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->isType(ActorType::Player)) {
                output.error("Only players can do");
                return;
            }

            auto* sp   = static_cast<Player*>(entity);
            auto  xuid = sp->getXuid();

            // 检查是否为OP（腐竹）
            if (!rlx_land::PermissionService::getInstance().isOperator(sp)) {
                output.error("只有腐竹可以执行此操作");
                return;
            }

            auto operation  = param.Operation;
            auto townName   = param.TownName.mText;
            auto playerName = param.PlayerName.mText;

            switch (operation) {
            case TownCommandBasicOperation::create: {
                if (townName.empty()) {
                    output.error("请指定城镇名称");
                    return;
                }

                // 检查城镇名称是否已存在
                bool townExists = false;
                for (auto town : rlx_land::DataService::getInstance()->getAllItems<TownData>()) {
                    if (town->getTownName() == townName) {
                        townExists = true;
                        break;
                    }
                }
                if (townExists) {
                    output.error("城镇名称已存在: " + townName);
                    return;
                }

                // 获取镇长XUID
                std::string mayorXuid = xuid; // 默认为当前操作者
                if (!playerName.empty()) {
                    mayorXuid = rlx_land::LeviLaminaAPI::getXuidByPlayerName(playerName);
                    if (mayorXuid.empty()) {
                        output.error("找不到玩家: " + playerName);
                        return;
                    }
                }

                // 获取坐标参数或使用默认值
                int x, z, x_end, z_end;
                if (param.X != 0 || param.Z != 0 || param.DX != 0 || param.DZ != 0) {
                    auto [x_min, x_max] = std::minmax(param.X, param.DX);
                    auto [z_min, z_max] = std::minmax(param.Z, param.DZ);
                    x                   = x_min;
                    z                   = z_min;
                    x_end               = x_max;
                    z_end               = z_max;
                } else {
                    // 使用默认值（以玩家位置为中心的100x100区域）
                    x     = (int)sp->getPosition().x - 50;
                    z     = (int)sp->getPosition().z - 50;
                    x_end = x + 100;
                    z_end = z + 100;
                }

                // 创建城镇数据
                TownData data(
                    x,
                    z,
                    x_end,
                    z_end,
                    townName,
                    mayorXuid,
                    sp->getDimensionId(),
                    rlx_land::DataService::getMaxId<TownData>() + 1
                );

                try {
                    // 创建PlayerInfo结构
                    auto playerInfo = rlx_land::PlayerInfoUtils::fromXuid(sp->getXuid());

                    // 使用统一的createItem方法创建城镇
                    rlx_land::DataService::getInstance()->createItem<TownData>(data, playerInfo);

                    std::string mayorName = rlx_land::LeviLaminaAPI::getPlayerNameByXuid(mayorXuid);
                    output.success("创建城镇成功: " + townName + "，镇长: " + mayorName);
                } catch (const rlx_land::RealmOutOfRangeException& e) {
                    output.error(e.what());
                } catch (const rlx_land::RealmPermissionException& e) {
                    output.error(e.what());
                } catch (const rlx_land::RealmConflictException& e) {
                    output.error(e.what());
                } catch (const std::exception& e) {
                    output.error(format("创建城镇时发生错误: {}", e.what()));
                }
                return;
            }
            case TownCommandBasicOperation::remove: {
                if (townName.empty()) {
                    output.error("请指定要删除的城镇名称");
                    return;
                }

                TownInformation* town = nullptr;
                for (auto t : rlx_land::DataService::getInstance()->getAllItems<TownData>()) {
                    if (t->getTownName() == townName) {
                        town = t;
                        break;
                    }
                }
                if (!town) {
                    output.error("找不到城镇: " + townName);
                    return;
                }

                // 使用城镇的坐标删除城镇
                rlx_land::DataService::getInstance()
                    ->deleteItem<TownData>(town->getX(), town->getZ(), town->getDimension());

                output.success("删除城镇: " + townName);
                break;
            }
            case TownCommandBasicOperation::list: {
                auto towns = rlx_land::DataService::getInstance()->getAllItems<TownData>();

                std::string townList = "城镇列表:\n";
                for (auto town : towns) {
                    townList += "- " + town->getTownName() + " (镇长: " + town->getOwnerName() + ")\n";
                }
                output.success(townList);
                break;
            }
            case TownCommandBasicOperation::transfer: {
                if (townName.empty() || playerName.empty()) {
                    output.error("请指定城镇名称和新镇长");
                    return;
                }

                TownInformation* town = nullptr;
                for (auto t : rlx_land::DataService::getInstance()->getAllItems<TownData>()) {
                    if (t->getTownName() == townName) {
                        town = t;
                        break;
                    }
                }
                if (!town) {
                    output.error("找不到城镇: " + townName);
                    return;
                }

                auto newXuid = rlx_land::LeviLaminaAPI::getXuidByPlayerName(playerName);
                if (newXuid.empty()) {
                    output.error("找不到玩家: " + playerName);
                    return;
                }

                // 使用新的坐标参数转让镇长
                rlx_land::DataService::getInstance()
                    ->transferTownMayor(town->getX(), town->getZ(), town->getDimension(), newXuid);

                output.success("转让城镇 " + townName + " 给 " + playerName);
                break;
            }
            }
        });

    // 成员管理命令（镇长专用）
    townCommand.overload<TownMemberCommand>()
        .required("Operation")
        .required("PlayerName")
        .execute([](CommandOrigin const&     origin,
                    CommandOutput&           output,
                    TownMemberCommand const& param,
                    Command const&) {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->isType(ActorType::Player)) {
                output.error("Only players can do");
                return;
            }

            auto* sp         = static_cast<Player*>(entity);
            auto  xuid       = sp->getXuid();
            auto  operation  = param.Operation;
            auto  playerName = param.PlayerName.mText;

            // 检查是否为OP或镇长
            // 获取玩家所在位置的Town
            auto pos = sp->getPosition();
            auto town =
                rlx_land::DataService::getInstance()->findTownAt((LONG64)pos.x, (LONG64)pos.z, sp->getDimensionId());

            if (!town) {
                output.error("您不在任何城镇内");
                return;
            }

            if (!town->isOwner(xuid) && !rlx_land::PermissionService::getInstance().isOperator(sp)) {
                output.error("您不是所在城镇的镇长或腐竹");
                return;
            }

            auto memberXuid = rlx_land::LeviLaminaAPI::getXuidByPlayerName(playerName);
            if (memberXuid.empty()) {
                output.error("找不到玩家: " + playerName);
                return;
            }

            switch (operation) {
            case TownCommandMemberOperation::member_add: {

                auto memberName = rlx_land::LeviLaminaAPI::getPlayerNameByXuid(memberXuid);

                try {
                    auto currentPlayer = rlx_land::PlayerInfoUtils::fromXuid(sp->getXuid());

                    rlx_land::DataService::getInstance()->addItemMember<TownData>(
                        (int)pos.x,
                        (int)pos.z,
                        sp->getDimensionId(),
                        currentPlayer,
                        memberName
                    );
                } catch (const rlx_land::PlayerNotFoundException&) {
                    output.error("找不到玩家: " + playerName);
                } catch (const rlx_land::DuplicateException&) {
                    output.error("玩家已经是成员: " + playerName);
                } catch (const std::exception&) {
                    output.error("添加成员失败: " + playerName);
                }

                output.success("成功添加成员: " + playerName);
                break;
            }
            case TownCommandMemberOperation::member_del: {
                auto memberName = rlx_land::LeviLaminaAPI::getPlayerNameByXuid(memberXuid);

                try {
                    auto currentPlayer = rlx_land::PlayerInfoUtils::fromXuid(sp->getXuid());

                    rlx_land::DataService::getInstance()->removeItemMember<TownData>(
                        (int)pos.x,
                        (int)pos.z,
                        sp->getDimensionId(),
                        currentPlayer,
                        memberName
                    );
                } catch (const rlx_land::PlayerNotFoundException&) {
                    output.error("找不到玩家: " + playerName);
                } catch (const rlx_land::NotMemberException&) {
                    output.error("玩家不是成员: " + playerName);
                } catch (const std::exception&) {
                    output.error("删除成员失败: " + playerName);
                }

                output.success("成功删除成员: " + playerName);
                break;
            }
            }
        });

    // 权限设置命令（镇长专用）
    townCommand.overload<TownPermCommand>()
        .required("Operation")
        .required("Perm")
        .execute([](CommandOrigin const& origin, CommandOutput& output, TownPermCommand const& param, Command const&) {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->isType(ActorType::Player)) {
                output.error("Only players can do");
                return;
            }

            auto* sp        = static_cast<Player*>(entity);
            auto  xuid      = sp->getXuid();
            auto  operation = param.Operation;
            auto  perm      = param.Perm;

            // 检查是否为OP或镇长
            // 获取玩家所在位置的Town
            auto pos = sp->getPosition();
            auto town =
                rlx_land::DataService::getInstance()->findTownAt((LONG64)pos.x, (LONG64)pos.z, sp->getDimensionId());

            TownInformation* currentTown = nullptr;
            if (town && (town->isOwner(xuid) || rlx_land::PermissionService::getInstance().isOperator(sp))) {
                currentTown = town;
            }

            if (!currentTown) {
                output.error("您不是所在城镇的镇长或腐竹");
                return;
            }

            switch (operation) {
            case TownCommandPermOperation::perm: {
                auto currentPlayer = rlx_land::PlayerInfoUtils::fromXuid(sp->getXuid());
                rlx_land::DataService::getInstance()->modifyItemPermission<TownData>(
                    currentTown->getX(),
                    currentTown->getZ(),
                    currentTown->getDimension(),
                    perm,
                    currentPlayer
                );
                output.success("设置权限为: " + std::to_string(perm));
                break;
            }
            }
        });

    // 信息查看命令（所有玩家）
    townCommand.overload<TownInfoCommand>()
        .required("Operation")
        .execute([](CommandOrigin const& origin, CommandOutput& output, TownInfoCommand const& param, Command const&) {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->isType(ActorType::Player)) {
                output.error("Only players can do");
                return;
            }

            auto* sp        = static_cast<Player*>(entity);
            auto  xuid      = sp->getXuid();
            auto  operation = param.Operation;

            switch (operation) {
            case TownCommandOperation::info: {
                auto pos  = sp->getPosition();
                auto town = rlx_land::DataService::getInstance()
                                ->findTownAt((LONG64)pos.x, (LONG64)pos.z, sp->getDimensionId());

                if (town == nullptr) {
                    output.success("当前位置不在任何城镇内");
                    return;
                }

                std::string info  = "当前位置所在城镇信息:\n";
                info             += "城镇名称: " + town->getTownName() + "\n";
                info             += "镇长: " + town->getOwnerName() + "\n";
                info             += "成员: " + town->getMembers() + "\n";
                info             += "权限值: " + std::to_string(town->getPermission()) + "\n";
                output.success(info);
                break;
            }
            }
        });
}

} // namespace rlx_town
