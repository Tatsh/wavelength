#include "script/scriptfunction.h"

#include "os/debug.h"

// The vector is constructed and destroyed by one routine at 0x002986b0, which the image's static
// construction and destruction entries at 0x00298760 and 0x00298780 call.
// NTSC-U/C: 0x002986b0, PAL: 0x002a22c0
// NTSC-U/C: 0x00298760, PAL: 0x002a2370
// NTSC-U/C: 0x00298780, PAL: 0x002a2390
// NTSC-U/C: 0x005152e8
std::vector<ScriptFunction::Entry> ScriptFunction::sFunctions;

void ScriptFunction::Register(Handler pfnHandler, const char *pszName, void *pUserData) {
    Entry entry{String(pszName), pfnHandler, pUserData};
    sFunctions.push_back(entry);
}

void ScriptFunction::Unregister(Handler pfnHandler) {
    for (auto it = sFunctions.begin(); it != sFunctions.end(); ++it) {
        if (it->mHandler == pfnHandler) {
            sFunctions.erase(it);
            return;
        }
    }
}

bool ScriptFunction::Dispatch(DataArray *pCommand) {
    const char *pszName = pCommand->Sym(0);
    for (const Entry &entry : sFunctions) {
        if (entry.mName == pszName) {
            entry.mHandler(pCommand, entry.mUserData);
            return true;
        }
    }
    DebugPrint("Error: function %s has not been registered\n", pszName);
    return false;
}
