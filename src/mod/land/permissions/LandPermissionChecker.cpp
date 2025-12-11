#include "LandPermissionChecker.h"
#include "data/service/DataService.h"
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

#include <string>

using namespace std;

namespace rlx_land {

std::string LandPermissionChecker::showPerm(int perm, bool isOwner) {
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

void LandPermissionChecker::NoticePerm(Player* p, string ownerName, int perm) {
    p->sendMessage(format("§c这是§r{}§c的领地，需要§6{}({})§c权限", ownerName, showPerm(perm), (int)perm));
}

// 完整的三层权限继承机制实现
bool LandPermissionChecker::hasPerm(Player* p, Vec3 pos, int perm) {
    // 空指针检查
    if (nullptr == p) {
        return false;
    }

    // 腐竹权限检查（全局最高权限）
    if (rlx_land::PermissionService::getInstance().isOperator(p)) {
        return true;
    }

    // 检查玩家当前位置是否在领地内
    auto land = DataService::getInstance()->findLandAt((int)pos.x, (int)pos.z, (int)p->getDimensionId());

    // 情况1: 玩家在领地内（最高优先级）
    if (nullptr != land) {
        // 检查玩家是否是领地有权限者或者拥有相应权限
        if (land->hasBasicPermission(p->getXuid()) || (land->getPermission() & perm)) {
            return true;
        } else {
            // 领地内但没有权限
            NoticePerm(p, land->getOwnerName(), perm);
            return false;
        }
    }
    // 情况2: 玩家不在任何领地内，检查是否在城镇内
    else {
        // 查找当前位置是否在某个城镇内
        auto town = DataService::getInstance()->findTownAt((LONG64)pos.x, (LONG64)pos.z, (int)p->getDimensionId());

        // 情况2.1: 玩家在城镇内
        if (nullptr != town) {
            // 检查玩家是否是城镇的有权限者
            if (town->hasBasicPermission(p->getXuid())) {
                // 城镇成员拥有所有权限
                return true;
            } else {
                // 玩家是城镇访客，检查城镇访客权限设置
                if (town->getPermission() & perm) {
                    return true;
                } else {
                    // 城镇访客没有相应权限
                    NoticePerm(p, town->getTownName(), perm);
                    return false;
                }
            }
        }
        // 情况2.2: 玩家在野外(Wilderness)
        else {
            // 在野外默认拥有所有权限
            return true;
        }
    }
}

bool LandPermissionChecker::canHurt(Actor& actor, ActorDamageSource const& source) {
    auto& hurtActor  = actor;
    auto  type       = hurtActor.getEntityTypeId();
    auto  pos        = hurtActor.getPosition();
    int   typeMasked = (int)type & (~(int)ActorType::TypeMask);
    auto  li         = DataService::getInstance()->findLandAt((int)pos.x, (int)pos.z, (int)hurtActor.getDimensionId());

    if ((int)ActorType::Monster == (typeMasked & 0xFFF)) return true;

    Player* sp = nullptr;
    if (source.isEntitySource()) {
        Actor* act = nullptr;
        if (source.isChildEntitySource()) {
            act = ll::service::getLevel()->fetchEntity(source.getEntityUniqueID(), false);
        } else {
            act = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
        }
        if (NULL != act && act->isPlayer()) {
            sp = (Player*)act;
        }
    }

    if (li != NULL && ((int)ActorType::VillagerBase == typeMasked)
        && !(li->getPermission() & LandPerm::PERM_VILLAGER_ATK)) {

        if (nullptr != sp) {
            if (rlx_land::PermissionService::getInstance().isOperator(sp)) return true;
            sp->sendMessage(format("领地内的村民受到保护"));
        }

        return false;
    }

    if (nullptr == sp) {
        return true;
    } else {
        return hasPerm(sp, pos, LandPerm::PERM_ATK);
    }
}

} // namespace rlx_land
