#include "TownCommands.h"
#include "common/JsonLoader.h"
#include "common/LeviLaminaAPI.h"
#include "common/exceptions/LandExceptions.h"
#include "data/DataManager.h"
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

namespace rlx_land {

void TownCommands::registerCommands() {
    using ll::command::CommandRegistrar;
    auto& townCommand = CommandRegistrar::getInstance().getOrCreateCommand("town", "城镇系统");

    // 注册腐竹专用命令，使用GameDirectors权限，普通玩家看不到
    auto& opCommand = CommandRegistrar::getInstance()
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
                if (DataManager::getInstance()->findTownByName(townName)) {
                    output.error("城镇名称已存在: " + townName);
                    return;
                }

                // 获取镇长XUID
                std::string mayorXuid = xuid; // 默认为当前操作者
                if (!playerName.empty()) {
                    mayorXuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
                    if (mayorXuid.empty()) {
                        output.error("找不到玩家: " + playerName);
                        return;
                    }
                }

                // 获取坐标参数或使用默认值
                int x, z, dx, dz;
                if (param.X != 0 || param.Z != 0 || param.DX != 0 || param.DZ != 0) {
                    x  = min(param.X, param.DX);
                    z  = min(param.Z, param.DZ);
                    dx = max(param.X, param.DX);
                    dz = max(param.Z, param.DZ);
                } else {
                    // 使用默认值（以玩家位置为中心的100x100区域）
                    x  = (int)sp->getPosition().x - 50;
                    z  = (int)sp->getPosition().z - 50;
                    dx = x + 100;
                    dz = z + 100;
                }

                // 创建城镇数据
                TownData data;
                data.id          = DataManager::getInstance()->getTownMaxId() + 1;
                data.name        = townName;
                data.mayorXuid   = mayorXuid;
                data.memberXuids = {};
                data.perm        = 0; // 默认权限
                data.x           = x;
                data.z           = z;
                data.dx          = dx;
                data.dz          = dz;
                data.d           = sp->getDimensionId();
                data.description = "城镇 " + townName;

                // 创建城镇
                DataManager::getInstance()->createTown(data);

                std::string mayorName = LeviLaminaAPI::getPlayerNameByXuid(mayorXuid);
                output.success("创建城镇成功: " + townName + "，镇长: " + mayorName);
                break;
            }
            case TownCommandBasicOperation::remove: {
                if (townName.empty()) {
                    output.error("请指定要删除的城镇名称");
                    return;
                }

                auto town = DataManager::getInstance()->findTownByName(townName);
                if (!town) {
                    output.error("找不到城镇: " + townName);
                    return;
                }

                DataManager::getInstance()->deleteTown(town->td);

                output.success("删除城镇: " + townName);
                break;
            }
            case TownCommandBasicOperation::list: {
                auto towns = DataManager::getInstance()->getAllTowns();

                std::string townList = "城镇列表:\n";
                for (auto town : towns) {
                    townList += "- " + town->td.name + " (镇长: " + town->mayorName + ")\n";
                }
                output.success(townList);
                break;
            }
            case TownCommandBasicOperation::transfer: {
                if (townName.empty() || playerName.empty()) {
                    output.error("请指定城镇名称和新镇长");
                    return;
                }

                auto town = DataManager::getInstance()->findTownByName(townName);
                if (!town) {
                    output.error("找不到城镇: " + townName);
                    return;
                }

                auto newXuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
                if (newXuid.empty()) {
                    output.error("找不到玩家: " + playerName);
                    return;
                }

                DataManager::getInstance()->transferTownMayor(town, newXuid);

                output.success("转让城镇 " + townName + " 给 " + playerName);
                break;
            }
            }
        });

    // 成员管理命令（镇长专用）
    townCommand.overload<TownMemberCommand>()
        .required("Operation")
        .required("PlayerName")
        .execute(
            [](CommandOrigin const& origin, CommandOutput& output, TownMemberCommand const& param, Command const&) {
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
                auto pos  = sp->getPosition();
                auto town = TownMap::getInstance()->find((LONG64)pos.x, (LONG64)pos.z, sp->getDimensionId());

                if (!town) {
                    output.error("您不在任何城镇内");
                    return;
                }

                if (!town->isMayor(xuid) && !rlx_land::PermissionService::getInstance().isOperator(sp)) {
                    output.error("您不是所在城镇的镇长或腐竹");
                    return;
                }

                auto memberXuid = LeviLaminaAPI::getXuidByPlayerName(playerName);
                if (memberXuid.empty()) {
                    output.error("找不到玩家: " + playerName);
                    return;
                }

                switch (operation) {
                case TownCommandMemberOperation::member_add: {

                    auto memberName = LeviLaminaAPI::getPlayerNameByXuid(memberXuid);

                    try {
                        DataManager::getInstance()->addTownMember(town, memberName);
                    } catch (const PlayerNotFoundException&) {

                        output.error("找不到玩家: " + playerName);
                    } catch (const DuplicateException&) {
                        output.error("玩家已经是成员: " + playerName);
                    } catch (const std::exception&) {
                        output.error("添加成员失败: " + playerName);
                    }

                    output.success("成功添加成员: " + playerName);
                    break;
                }
                case TownCommandMemberOperation::member_del: {
                    auto memberName = LeviLaminaAPI::getPlayerNameByXuid(memberXuid);

                    try {
                        DataManager::getInstance()->removeTownMember(town, memberName);
                    } catch (const PlayerNotFoundException&) {
                        output.error("找不到玩家: " + playerName);
                    } catch (const NotMemberException&) {
                        output.error("玩家不是成员: " + playerName);
                    } catch (const std::exception&) {
                        output.error("删除成员失败: " + playerName);
                    }

                    output.success("成功删除成员: " + playerName);
                    break;
                }
                }
            }
        );

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
            auto pos  = sp->getPosition();
            auto town = TownMap::getInstance()->find((LONG64)pos.x, (LONG64)pos.z, sp->getDimensionId());

            TownInformation* currentTown = nullptr;
            if (town && (town->isMayor(xuid) || rlx_land::PermissionService::getInstance().isOperator(sp))) {
                currentTown = town;
            }

            if (!currentTown) {
                output.error("您不是所在城镇的镇长或腐竹");
                return;
            }

            switch (operation) {
            case TownCommandPermOperation::perm: {
                DataManager::getInstance()->modifyTownPerm(currentTown, perm);
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
                auto town = TownMap::getInstance()->find((LONG64)pos.x, (LONG64)pos.z, sp->getDimensionId());

                if (town == nullptr) {
                    output.success("当前位置不在任何城镇内");
                    return;
                }

                std::string info  = "当前位置所在城镇信息:\n";
                info             += "城镇名称: " + town->td.name + "\n";
                info             += "镇长: " + town->mayorName + "\n";
                info             += "成员: " + town->getMembers() + "\n";
                info             += "权限值: " + std::to_string(town->td.perm) + "\n";
                output.success(info);
                break;
            }
            }
        });
}

} // namespace rlx_land
