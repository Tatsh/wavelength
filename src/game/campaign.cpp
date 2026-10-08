#include "game/campaign.h"

#include "game/gamedb.h"

unsigned int g_dwUnlockAllParts = 0;

void Campaign::GetUnlockedParts(int nPart, std::vector<const char *> *pTypes) {
    AvatarPartSet::GetPartTypes(nPart, pTypes);
    if (g_dwUnlockAllParts == 0) {
        RemoveLockedItems(pTypes);
    }
}

void Campaign::GetUnlockedEmblems(std::vector<const char *> *pTypes) {
    AvatarPartSet::GetEmblemTypes(pTypes);
    if (g_dwUnlockAllParts == 0) {
        RemoveLockedItems(pTypes);
    }
}

void Campaign::RemoveLockedItems(std::vector<const char *> *pItems) {
    auto it = pItems->begin();
    while (it != pItems->end()) {
        if (IsUnlocked(*it, GameDb::kSkillAny)) {
            ++it;
        } else {
            it = pItems->erase(it);
        }
    }
}
