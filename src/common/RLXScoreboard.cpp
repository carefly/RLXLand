#include "common/RLXScoreboard.h"

#ifndef TESTING
#include <ll/api/service/Bedrock.h>
#include <mc/network/packet/ScorePacketInfo.h>
#include <mc/network/packet/SetDisplayObjectivePacket.h>
#include <mc/network/packet/SetScorePacket.h>
#include <mc/world/level/Level.h>
#include <mc/world/scores/ScoreInfo.h>
#include <mc/world/scores/Scoreboard.h>
#include <mc/world/scores/ScoreboardId.h>

SetScorePacket::SetScorePacket() : mType{} {}
SetDisplayObjectivePacketPayload::SetDisplayObjectivePacketPayload() : mSortOrder{} {}
SetDisplayObjectivePacket::SetDisplayObjectivePacket() = default;

#endif

namespace rlx_land {

void RLXScoreboard::removeClientSidebar(Player* pl) {

    auto pkt = SetDisplayObjectivePacket();

    pkt.mDisplaySlotName      = "sidebar";
    pkt.mObjectiveName        = "";
    pkt.mObjectiveDisplayName = "";
    pkt.mCriteriaName         = "dummy";
    pkt.mSortOrder            = ObjectiveSortOrder::Ascending;
    pl->sendNetworkPacket(pkt);
}

void RLXScoreboard::setClientSidebar(
    const std::string&                              title,
    const std::vector<std::pair<std::string, int>>& data,
    Player*                                         player
) {
#ifndef TESTING
    if (!player) return;

    auto* serverPlayer = static_cast<ServerPlayer*>(player);
    if (!serverPlayer) return;

    RLXScoreboard::removeClientSidebar(player);

    auto pkt1 = SetDisplayObjectivePacket();

    pkt1.mDisplaySlotName      = "sidebar";
    pkt1.mObjectiveName        = "RLX_SIDEBAR_API";
    pkt1.mObjectiveDisplayName = title;
    pkt1.mCriteriaName         = "dummy";
    pkt1.mSortOrder            = ObjectiveSortOrder::Ascending;
    pkt1.sendTo(*player);


    std::vector<ScorePacketInfo> info;
    for (auto& [key, index] : data) {
        const ScoreboardId& id = ScoreboardId(index + 1145141919810);

        auto text      = key;
        auto scoreInfo = ScorePacketInfo();

        scoreInfo.mScoreboardId   = id;
        scoreInfo.mObjectiveName  = "RLX_SIDEBAR_API";
        scoreInfo.mIdentityType   = IdentityDefinition::Type::FakePlayer;
        scoreInfo.mScoreValue     = index;
        scoreInfo.mFakePlayerName = text;
        info.emplace_back(scoreInfo);
    }

    auto pkt2 = SetScorePacket();

    pkt2.mScoreInfo = info;
    pkt2.mType      = ScorePacketType::Change;
    pkt2.sendTo(*player);
#endif
}

} // namespace rlx_land
