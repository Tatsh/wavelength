#include "game/datafuncaction.h"

#include "script/scriptfunction.h"

DataFuncAction::DataFuncAction(DataArray *pAction) : mCommand(pAction) {
    pAction->AddRef();
}

void DataFuncAction::Exec() {
    (void)ScriptFunction::Dispatch(mCommand); // Yes, the binary discards the result.
}
