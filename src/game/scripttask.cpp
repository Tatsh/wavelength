#include "game/scripttask.h"

#include "script/scriptfunction.h"

ScriptTask::ScriptTask(DataArray *pCommand) : mCommand(pCommand) {
}

void ScriptTask::OnStart() {
    ScriptFunction::Dispatch(mCommand);
    Finish(true);
}
