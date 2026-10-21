#include "game/avatargemcatchaction.h"

#include "game/avatarpartset.h"
#include "game/gamedb.h"
#include "game/triggermgr.h"

namespace {

constexpr int kMinSizeWithLane = 3;

} // namespace

AvatarGemCatchAction::AvatarGemCatchAction(DataArray *pAction) {
    mCatch = pAction->Int(1) != 0;
    if (pAction->Size() >= kMinSizeWithLane) {
        mLane = pAction->Int(2);
    } else {
        mLane = kEventLane;
    }
}

void AvatarGemCatchAction::Exec() {
    AvatarPartSet *pAvatar = TheGameDb->GetAvatar(TheTriggerMgr.GetPlayer());
    if (mLane == kEventLane) {
        pAvatar->CatchGem(TheTriggerMgr.GetGemPos(), mCatch);
    } else {
        pAvatar->CatchGem(mLane, mCatch);
    }
}
