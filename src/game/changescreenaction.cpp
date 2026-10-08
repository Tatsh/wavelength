#include "game/changescreenaction.h"

#include "os/debug.h"
#include "ui/uimanager.h"

ChangeScreenAction::ChangeScreenAction(DataArray *pAction) {
    mScreen = TheUI.FindScreen(pAction->Sym(1), true);
    if (mScreen == nullptr) {
        DebugWarn("Couldn't find '%s' object (file %s, line %d)",
                  pAction->Sym(1),
                  pAction->mFile,
                  pAction->mLine);
    }
}

void ChangeScreenAction::Exec() {
    TheUI.GotoScreen(mScreen);
}
