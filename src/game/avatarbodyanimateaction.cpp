#include "game/avatarbodyanimateaction.h"

#include "game/avatarpartset.h"
#include "game/gamedb.h"
#include "game/triggermgr.h"

namespace {

constexpr int kMinSizeWithFlags = 3;

} // namespace

AvatarBodyAnimateAction::AvatarBodyAnimateAction(DataArray *pAction) : mAnim(pAction->Sym(1)) {
    if (pAction->Size() < kMinSizeWithFlags) {
        mFlags = 0;
    } else {
        mFlags = pAction->Int(2) != 0;
    }
}

void AvatarBodyAnimateAction::Exec() {
    TheGameDb->GetAvatar(TheTriggerMgr.mPlayer)->SetBaseAnim(mAnim, mFlags);
}
