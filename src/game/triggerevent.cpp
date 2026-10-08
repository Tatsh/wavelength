#include "game/triggerevent.h"

#include <cstring>

#include "os/debug.h"

namespace {

// NTSC-U/C: 0x003afc48
const char *const kTypeNames[TriggerEvent::kNumTypes] = {
    "beat",
    "time",
    "component_select",
    "component_select_start",
    "component_focus",
    "screen_change",
    "button",
    "begin",
    "end",
    "stage_complete",
    "score",
    "hit",
    "miss",
    "pass",
    "gem",
    "new_bar",
    "phrase_end",
    "phrase_capture",
    "phrase_miss",
    "new_track",
    "powerup_capture",
    "powerup_deploy",
    "new_leader",
    "health",
    "dying_change",
    "boss_journey",
    "lyric",
    "path_unlocked",
};

} // namespace

int TriggerEvent::FindType(DataArray *pArray, int nNode) {
    const char *pszName = pArray->Sym(nNode);
    for (int i = 0; i < kNumTypes; ++i) {
        if (std::strcmp(pszName, kTypeNames[i]) == 0) {
            return i;
        }
    }
    DebugWarn("Don't recognize %s event (file %s, line %d)", pszName, pArray->mFile, pArray->mLine);
    return kNumTypes;
}

void TriggerEvent::Fire() {
    for (TriggerHandler *pHandler : mHandlers) {
        pHandler->Fire();
    }
}

void TriggerEvent::Clear() {
    for (TriggerHandler *pHandler : mHandlers) {
        delete pHandler;
    }
    mHandlers.clear();
}

void TriggerEvent::AddHandler(DataArray *pTrigger) {
    mHandlers.push_back(new TriggerHandler(pTrigger));
}
