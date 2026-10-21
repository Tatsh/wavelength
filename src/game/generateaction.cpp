#include "game/generateaction.h"

GenerateAction::GenerateAction(DataArray *pAction) {
    mGenerator = FindObject<RndGenerator>(pAction, pAction->Sym(1));
}

void GenerateAction::Exec() {
    mGenerator->Generate(mGenerator->GetFilteredFrame());
}
