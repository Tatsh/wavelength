#include "game/waitinteractiveremixtask.h"

#include <cstring>

#include "os/scheduler.h"
#include "script/scriptfunction.h"

namespace {

constexpr char kIdleCondition[] = "idle";
constexpr char kIdle2Condition[] = "idle2";
constexpr char kPitchCondition[] = "pitch";
constexpr char kLoopOnCondition[] = "loop_on";
constexpr char kForwardSectionCondition[] = "forward_section";
constexpr char kBurnPatternCondition[] = "burn_pattern";
constexpr char kEraseCondition[] = "erase";
constexpr char kEraseSectionCondition[] = "erase_section";
constexpr char kRotateCondition[] = "rotate";
constexpr char kPatternMenuOpenCondition[] = "pattern_menu_open";
constexpr char kDoneAction[] = "done";

// Nodes of a condition.
constexpr int kConditionName = 0;
constexpr int kConditionValue = 1;

// Index of the first action after no value and after one value.
constexpr int kActionsAfterName = 1;
constexpr int kActionsAfterValue = 2;

// Index of the first condition of a step.
constexpr int kFirstCondition = 1;

constexpr int kFlagSet = 1;
constexpr float kMillisecondsPerSecond = 1000.0f;

} // namespace

WaitInteractiveRemixTask::WaitInteractiveRemixTask(DataArray *pStep, RemixTutorial *pTutorial)
    : mTutorial(pTutorial), mStep(pStep), mPitchCount(0), mIdleTime(0.0f), mIdle2Time(0.0f),
      mLoopOn(0), mForwardSection(0), mBurnPattern(0), mEraseCount(0), mRotation(0),
      mEraseSection(0), mPatternMenuOpen(0) {
}

void WaitInteractiveRemixTask::OnStart() {
    GameCallback::Set(this);
    mIdleTime = TheSongScheduler.mTime;
    mIdle2Time = TheSongScheduler.mTime;
}

void WaitInteractiveRemixTask::OnStop() {
    GameCallback::Set(nullptr);
}

void WaitInteractiveRemixTask::OnPoll() {
    DataArray *pCondition = mStep->FindArray(kIdleCondition, false);
    if (pCondition != nullptr) {
        const float fNow = TheSongScheduler.mTime;
        if (mIdleTime + pCondition->Float(kConditionValue) * kMillisecondsPerSecond < fNow) {
            mIdleTime = fNow;
            RunActions(pCondition, kActionsAfterValue);
        }
    }

    pCondition = mStep->FindArray(kIdle2Condition, false);
    if (pCondition != nullptr) {
        const float fNow = TheSongScheduler.mTime;
        if (mIdle2Time + pCondition->Float(kConditionValue) * kMillisecondsPerSecond < fNow) {
            mIdle2Time = fNow;
            RunActions(pCondition, kActionsAfterValue);
        }
    }
}

void WaitInteractiveRemixTask::OnPitch() {
    ++mPitchCount;
    mIdleTime = TheSongScheduler.mTime;
    mIdle2Time = TheSongScheduler.mTime;
    EvaluateConditions();
}

void WaitInteractiveRemixTask::OnLoop() {
    mLoopOn = kFlagSet;
    EvaluateConditions();
}

void WaitInteractiveRemixTask::OnSectionChange([[maybe_unused]] int nSection) {
    mForwardSection = kFlagSet;
    mIdleTime = TheSongScheduler.mTime;
    mIdle2Time = TheSongScheduler.mTime;
    EvaluateConditions();
}

void WaitInteractiveRemixTask::OnBurnPattern() {
    mBurnPattern = kFlagSet;
    EvaluateConditions();
}

void WaitInteractiveRemixTask::OnErase() {
    ++mEraseCount;
    EvaluateConditions();
}

void WaitInteractiveRemixTask::OnRotate(bool bRight) {
    mRotation = bRight ? mRotation + 1 : mRotation - 1;
    EvaluateConditions();
}

void WaitInteractiveRemixTask::OnEraseSection() {
    mEraseSection = kFlagSet;
    EvaluateConditions();
}

void WaitInteractiveRemixTask::OnPatternMenuOpen() {
    mPatternMenuOpen = kFlagSet;
    EvaluateConditions();
}

void WaitInteractiveRemixTask::EvaluateConditions() {
    const int nSize = mStep->Size();
    for (int i = kFirstCondition; i < nSize; ++i) {
        EvaluateCondition(mStep->Array(i));
    }
}

void WaitInteractiveRemixTask::EvaluateCondition(DataArray *pCondition) {
    const char *pszName = pCondition->Sym(kConditionName);
    int nFlag = 0;

    if (strcmp(pszName, kPitchCondition) == 0) {
        if (mPitchCount < pCondition->Int(kConditionValue)) {
            return;
        }
        mPitchCount = 0;
        RunActions(pCondition, kActionsAfterValue);
        return;
    }
    if (strcmp(pszName, kLoopOnCondition) == 0) {
        nFlag = mLoopOn;
    } else if (strcmp(pszName, kForwardSectionCondition) == 0) {
        nFlag = mForwardSection;
    } else if (strcmp(pszName, kBurnPatternCondition) == 0) {
        nFlag = mBurnPattern;
    } else if (strcmp(pszName, kEraseCondition) == 0) {
        // Neither `erase` nor `rotate` resets its count when it fires.
        if (mEraseCount >= pCondition->Int(kConditionValue)) {
            RunActions(pCondition, kActionsAfterValue);
        }
        return;
    } else if (strcmp(pszName, kEraseSectionCondition) == 0) {
        nFlag = mEraseSection;
    } else if (strcmp(pszName, kRotateCondition) == 0) {
        if (mRotation >= pCondition->Int(kConditionValue)) {
            RunActions(pCondition, kActionsAfterValue);
        }
        return;
    } else if (strcmp(pszName, kPatternMenuOpenCondition) == 0) {
        nFlag = mPatternMenuOpen;
    } else {
        return;
    }

    if (nFlag != 0) {
        RunActions(pCondition, kActionsAfterName);
    }
}

void WaitInteractiveRemixTask::RunActions(DataArray *pCondition, int nFirst) {
    for (int i = nFirst; i < pCondition->Size(); ++i) {
        if (pCondition->Type(i) == DataArray::kNodeSymbol) {
            if (strcmp(pCondition->Sym(i), kDoneAction) == 0) {
                Finish(true);
            }
        } else {
            ScriptFunction::Dispatch(pCondition->Array(i));
        }
    }
}
