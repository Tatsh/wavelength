#include "game/playerprofile.h"

#include "game/gamedb.h"

unsigned int g_dwUnlockAllParts = 0;

void PlayerProfile::GetUnlockedParts(int nPart, std::vector<const char *> *pTypes) {
    AvatarPartSet::GetPartTypes(nPart, pTypes);
    if (g_dwUnlockAllParts == 0) {
        RemoveLockedItems(pTypes);
    }
}

void PlayerProfile::GetUnlockedEmblems(std::vector<const char *> *pTypes) {
    AvatarPartSet::GetEmblemTypes(pTypes);
    if (g_dwUnlockAllParts == 0) {
        RemoveLockedItems(pTypes);
    }
}

void PlayerProfile::RemoveLockedItems(std::vector<const char *> *pItems) {
    auto it = pItems->begin();
    while (it != pItems->end()) {
        if (IsUnlocked(*it, GameDb::kSkillAny)) {
            ++it;
        } else {
            it = pItems->erase(it);
        }
    }
}
