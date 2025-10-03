#include "LandEventHandlers.h"
#include "data/spatial/SpatialMap.h"
#include "mod/land/permissions/LandPermissionChecker.h"

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
#include <unordered_map>

using namespace std;

namespace rlx_land {

// 全局变量
unordered_map<string, Vec3> playerPosPre;

void LandEventHandlers::registerEventListeners() {
    auto& eventBus = ll::event::EventBus::getInstance();

    // 破坏方块事件
    ll::event::ListenerPtr playerDestroyBlockEventListener =
        eventBus.emplaceListener<ll::event::player::PlayerDestroyBlockEvent>(
            [](ll::event::player::PlayerDestroyBlockEvent& event) {
                event.setCancelled(!LandPermissionChecker::hasPerm(&event.self(), event.pos().center(), 8)
                ); // PERM_BUILD = 8
            }
        );

    // 放置方块事件
    ll::event::ListenerPtr PlayerPlaceBlockEventListener =
        eventBus.emplaceListener<ll::event::player::PlayerPlacingBlockEvent>(
            [](ll::event::player::PlayerPlacingBlockEvent& event) {
                event.setCancelled(!LandPermissionChecker::hasPerm(&event.self(), event.pos().center(), 8)
                ); // PERM_BUILD = 8
            }
        );

    // 方块交互事件
    ll::event::ListenerPtr BlockInteractedEventListener =
        eventBus.emplaceListener<ll::event::player::PlayerInteractBlockEvent>(
            [](ll::event::player::PlayerInteractBlockEvent& event) {
                auto&  p    = event.self();
                string xuid = p.getXuid();
                // 这里保留原来逻辑中与领地选择相关的处理，但先暂时跳过
                event.setCancelled(!LandPermissionChecker::hasPerm(&event.self(), event.clickPos(), 32)
                ); // PERM_INTERWITHACTOR = 32
            }
        );

    // 火焰蔓延事件
    ll::event::ListenerPtr FireSpreadEventListener =
        eventBus.emplaceListener<ll::event::world::FireSpreadEvent>([](ll::event::world::FireSpreadEvent& event) {
            auto pos = event.pos();
            auto dim = event.blockSource().getDimensionId();
            auto li  = LandMap::getInstance()->find((int)pos.x, (int)pos.z, (int)dim);

            if (li != NULL && (li->ld.perm & 256)) { // PERM_FIRE = 256
                event.setCancelled(false);
            } else {
                event.setCancelled(true);
            }
        });
}

// Hook函数定义（从原始代码中复制）
LL_TYPE_INSTANCE_HOOK(
    PlayerUseFrameHook1,
    HookPriority::Normal,
    ItemFrameBlock,
    // "?use@ItemFrameBlock@@UEBA_NAEAVPlayer@@AEBVBlockPos@@E@Z",
    &ItemFrameBlock::use,
    void,
    ::BlockEvents::BlockPlayerInteractEvent& eventData
) {
    auto p = &eventData.mPlayer;
    if (auto hitPos = eventData.mHit.get()) {
        Vec3 pos = *hitPos;
        if (LandPermissionChecker::hasPerm(p, pos, 16)) { // PERM_POPITEM = 16
            return origin(eventData);
        } else {
            return;
        }
    } else {
        return;
    }
}

LL_TYPE_INSTANCE_HOOK(
    UseFrameHook2,
    HookPriority::Normal,
    ItemFrameBlock,
    &ItemFrameBlock::$attack,
    bool,
    Player*         player,
    BlockPos const& pos
) {
    if (LandPermissionChecker::hasPerm(player, pos.center(), 16)) { // PERM_POPITEM = 16
        return origin(player, pos);
    } else {
        return false;
    }
}

LL_TYPE_INSTANCE_HOOK(
    PlayerPullFishingHook,
    HookPriority::Normal,
    FishingHook,
    &FishingHook::_pullCloser,
    void,
    Actor& inEntity,
    float  inSpeed
) {
    if (inEntity.isPlayer()) {
        auto* sp = static_cast<Player*>(&inEntity);

        if (LandPermissionChecker::hasPerm(sp, this->getPosition(), 128)) { // PERM_FISHINGHOOK = 128
            origin(inEntity, inSpeed);
        } else {
            return;
        }
    }
}

LL_TYPE_INSTANCE_HOOK(
    ArmorStandSwapItemHook,
    HookPriority::Normal,
    ArmorStand,
    &ArmorStand::_trySwapItem,
    bool,
    Player&                              player,
    ::SharedTypes::Legacy::EquipmentSlot slot
) {
    if (LandPermissionChecker::hasPerm(&player, this->getPosition(), 64)) { // PERM_AMRORSTANDER = 64
        return origin(player, slot);
    } else {
        return false;
    }
}

LL_STATIC_HOOK(
    StartDestroyBlockHook,
    HookPriority::Normal,
    &ServerPlayerBlockUseHandler::onStartDestroyBlock,
    void,
    ServerPlayer&   serverPlayer,
    const BlockPos& pos,
    int             face
) {
    if (!LandPermissionChecker::hasPerm(&static_cast<Player&>(serverPlayer), pos.center(), 2)) { // PERM_USE_ON = 2
        return;
    }
    return origin(serverPlayer, pos, face);
}

LL_TYPE_INSTANCE_HOOK(
    MobHurtEffectsHook,
    HookPriority::Normal,
    Mob,
    &Mob::getDamageAfterResistanceEffect,
    float,
    ::ActorDamageSource const& source,
    float                      damage
) {
    if (LandPermissionChecker::canHurt(*this, source)) {
        return origin(source, damage);
    } else {
        return 0.0f;
    }
}

void LandEventHandlers::hookAllFunctions() {
    // Hook函数注册
    UseFrameHook2::hook();
    StartDestroyBlockHook::hook();
    PlayerUseFrameHook1::hook();
    PlayerPullFishingHook::hook();
    ArmorStandSwapItemHook::hook();
    MobHurtEffectsHook::hook();
}

} // namespace rlx_land
