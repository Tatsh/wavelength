#include "game/generateaction.h"

GenerateAction::GenerateAction(DataArray *pAction) {
    mGenerator = FindObject<Rnd::Generator>(pAction, pAction->Sym(1));
}

void GenerateAction::Exec() {
    mGenerator->Generate(mGenerator->mFilteredFrame);
}
