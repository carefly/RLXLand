#include "plugin/RLXStatus.h"
#include "common/RLXScoreboard.h"
#include "data/service/DataService.h"
#include "mod/RLXLand.h"

#include <chrono>
#include <format>
#include <ll/api/service/Bedrock.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/Level.h>
#include <string>

namespace rlx_land {

RLXStatus& RLXStatus::getInstance() {
    static RLXStatus instance;
    return instance;
}

bool RLXStatus::load() { return true; }

bool RLXStatus::enable() {
    RLXLand::getInstance().getSelf().getLogger().info("load status");

    // 初始化TPS计算时间
    lastTpsCalculationTime_ = std::chrono::steady_clock::now();
    tickCount_              = 0;

    return true;
}

bool RLXStatus::disable() { return true; }

void RLXStatus::oneTick() {
    auto now = std::chrono::steady_clock::now();

    // 更新tick计数
    tickCount_++;

    // 定期计算TPS
    auto timeSinceLastTpsCalculation =
        std::chrono::duration_cast<std::chrono::seconds>(now - lastTpsCalculationTime_).count();
    if (timeSinceLastTpsCalculation >= 1) {
        double tps = static_cast<double>(tickCount_) / static_cast<double>(timeSinceLastTpsCalculation);

        auto level = ll::service::getLevel();
        if (!level.has_value()) return;

        std::string tps_value;
        if (tps >= 18.0) {
            tps_value = "§a" + std::to_string((int)tps);
        } else if (tps >= 15.0) {
            tps_value = "§e" + std::to_string((int)tps);
        } else {
            tps_value = "§c" + std::to_string((int)tps);
        }

        level->forEachPlayer([&](Player& player) {
            auto pos = player.getPosition();

            auto li = DataService::getInstance()->findLandAt((int)pos.x, (int)pos.z, (int)player.getDimensionId());
            auto town =
                DataService::getInstance()->findTownAt((LONG64)pos.x, (LONG64)pos.z, (int)player.getDimensionId());

            std::string land_value;
            if (li) {
                land_value = "§6" + li->getOwnerName() + "§r领地(§d" + std::to_string(li->getPermission()) + "§r)";
            } else {
                land_value = "§r空地";
            }

            std::string town_value;
            if (town) {
                town_value = "§b" + town->getTownName() + "§r(§d" + std::to_string(town->getPermission()) + "§r)";
            } else {
                town_value = "§7野外";
            }

            auto data = std::vector<std::pair<std::string, int>>{
                {std::format("§rTPS {}", tps_value), 0},
                {land_value, 1},
                {town_value, 2},
            };

            RLXScoreboard::setClientSidebar("RLX", data, &player);
            return true;
        });

        // 重置计数器
        tickCount_              = 0;
        lastTpsCalculationTime_ = now;
    }
}

} // namespace rlx_land
