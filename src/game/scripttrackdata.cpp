#include "game/scripttrackdata.h"

#include "game/tickobjvector.h"

ScriptTrackData::ScriptTrackData(const char *pszName) : mName(pszName) {
}

ScriptTrackData::~ScriptTrackData() {
    for (const auto &command : mCommands) {
        command.mValue->Release();
    }
}

void ScriptTrackData::AddCommand(int nTick, DataArray *pCommand) {
    TickObj<DataArray *> entry;
    entry.mPosition.mTick = nTick;
    entry.mValue = pCommand;
    mCommands.insert(UpperBoundByEntry(mCommands, entry), entry);
}

int ScriptTrackData::NumCommands() const {
    return static_cast<int>(mCommands.size());
}

TickObj<DataArray *> *ScriptTrackData::GetCommand(int nIndex) {
    return &mCommands[nIndex];
}
