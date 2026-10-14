#include "game/changelyricaction.h"

#include "game/triggermgr.h"

ChangeLyricAction::ChangeLyricAction(DataArray *pAction) {
    mText = FindObject<RndText>(pAction, pAction->Sym(1));
}

void ChangeLyricAction::Exec() {
    mText->SetText(TheTriggerMgr.mLyric.c_str());
}
