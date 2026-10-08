#include "game/changelyricaction.h"

#include "game/triggermgr.h"
#include "os/hxstr.h"

ChangeLyricAction::ChangeLyricAction(DataArray *pAction) {
    mText = FindObject<Rnd::Text>(pAction, pAction->Sym(1));
}

void ChangeLyricAction::Exec() {
    mText->SetText(HxStr(TheTriggerMgr.mLyric.c_str()));
}
