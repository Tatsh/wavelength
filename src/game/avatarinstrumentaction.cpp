#include "game/avatarinstrumentaction.h"

#include "game/avatarpartset.h"
#include "game/gamedb.h"
#include "game/triggermgr.h"

AvatarInstrumentAction::AvatarInstrumentAction(DataArray *pAction) : mPose(pAction->Sym(1)) {
}

void AvatarInstrumentAction::Exec() {
    TheGameDb->GetAvatar(TheTriggerMgr.mPlayer)->SetPose(mPose);
}
