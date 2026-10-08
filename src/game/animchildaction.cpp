#include "game/animchildaction.h"

#include "os/debug.h"
#include "rnd/manager.h"

AnimChildAction::AnimChildAction(DataArray *pAction, bool bAdd) {
    mChild = dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find(pAction->Sym(1)));
    mParent = dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find(pAction->Sym(2)));
    if (mChild == nullptr || mParent == nullptr) {
        const char *pszMissing = mChild != nullptr ? pAction->Sym(2) : pAction->Sym(1);
        DebugWarn("Couldn't find '%s' object (file %s, line %d)",
                  pszMissing,
                  pAction->mFile,
                  pAction->mLine);
    }
    mAdd = bAdd;
}

void AnimChildAction::Exec() {
    if (mAdd != 0) {
        mParent->AddAnim(mChild);
    } else {
        mParent->RemoveAnim(mChild);
    }
}
