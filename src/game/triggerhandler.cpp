#include "game/triggerhandler.h"

#include <cstring>

#include "game/triggermgr.h"
#include "os/debug.h"

namespace {

constexpr char kConditionsTag[] = "conditions";
constexpr char kActionsTag[] = "actions";
constexpr char kRealtimeTag[] = "realtime";

constexpr char kNegatePrefix = '!';

} // namespace

TriggerHandler::TriggerHandler(DataArray *pTrigger) {
    for (int i = 1; i < pTrigger->Size(); ++i) {
        if (std::strcmp(pTrigger->Array(i)->Sym(0), kConditionsTag) == 0) {
            AddConditions(pTrigger->Array(i));
        }
    }
    AddActions(pTrigger->FindArray(kActionsTag, true));
}

TriggerHandler::~TriggerHandler() {
    for (std::list<TriggerCondition *> &group : mConditions) {
        for (TriggerCondition *pCondition : group) {
            delete pCondition;
        }
    }
    for (TriggerAction *pAction : mActions) {
        delete pAction;
    }
}

void TriggerHandler::AddActions(DataArray *pActions) {
    float flDelay = 0.0f;
    int nClock = TriggerMgr::kClockGame;
    for (int i = 1; i < pActions->Size(); ++i) {
        const int nType = pActions->Type(i);
        if (nType == DataArray::kNodeFloat || nType == DataArray::kNodeInt) {
            flDelay = pActions->Float(i);
        } else if (nType == DataArray::kNodeSymbol) {
            if (std::strcmp(pActions->Sym(i), kRealtimeTag) == 0) {
                nClock = TriggerMgr::kClockReal;
            } else {
                DebugWarn("Don't recognize '%s' in actions (file %s, line %d, node %d)",
                          pActions->Sym(i),
                          pActions->mFile,
                          pActions->mLine,
                          i);
            }
        } else {
            TriggerAction *pAction = TriggerAction::New(pActions->Array(i));
            pAction->mDelay = flDelay;
            pAction->mClock = nClock;
            mActions.push_back(pAction);
            flDelay = 0.0f; // The clock is not reset, and applies to every later action.
        }
    }
}

void TriggerHandler::AddConditions(DataArray *pConditions) {
    mConditions.push_back(std::list<TriggerCondition *>());
    int nNegate = 0;
    for (int i = 1; i < pConditions->Size(); ++i) {
        if (pConditions->Type(i) == DataArray::kNodeSymbol) {
            const char *pszSymbol = pConditions->Sym(i);
            if (pszSymbol[0] != kNegatePrefix) {
                DebugWarn("Don't recognize '%s' (file %s, line %d, node %d)",
                          pszSymbol,
                          pConditions->mFile,
                          pConditions->mLine,
                          i);
            }
            nNegate = 1;
        } else {
            TriggerCondition *pCondition = TriggerCondition::New(pConditions->Array(i));
            pCondition->mNegate = nNegate;
            mConditions.back().push_back(pCondition);
            nNegate = 0;
        }
    }
}

void TriggerHandler::Fire() {
    if (!mConditions.empty()) {
        auto group = mConditions.begin();
        for (; group != mConditions.end(); ++group) {
            auto condition = group->begin();
            for (; condition != group->end(); ++condition) {
                const bool bHolds = (*condition)->Test();
                if ((*condition)->mNegate != 0 ? bHolds : !bHolds) {
                    break;
                }
            }
            if (condition == group->end()) {
                break;
            }
        }
        if (group == mConditions.end()) {
            return;
        }
    }
    for (TriggerAction *pAction : mActions) {
        pAction->Start();
    }
}
