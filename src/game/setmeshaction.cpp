#include "game/setmeshaction.h"

SetMeshAction::SetMeshAction(DataArray *pAction) {
    mMesh = FindObject<Rnd::Mesh>(pAction, pAction->Sym(1));
    mFlags = 0;
    const char *pszMat;
    if (pAction->FindSymbol("mat", &pszMat, false)) {
        mFlags |= kFlagMat;
        mMat = FindObject<Rnd::Mat>(pAction, pszMat);
    }
}

void SetMeshAction::Exec() {
    if ((mFlags & kFlagMat) != 0) {
        mMesh->SetMat(mMat);
    }
}
