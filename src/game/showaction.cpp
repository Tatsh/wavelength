#include "game/showaction.h"

ShowAction::ShowAction(DataArray *pAction, bool bShow) {
    mDrawable = FindObject<Rnd::Drawable>(pAction, pAction->Sym(1));
    mShow = bShow;
}

void ShowAction::Exec() {
    mDrawable->SetShowing(mShow);
}
