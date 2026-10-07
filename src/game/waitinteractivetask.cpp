#include "game/waitinteractivetask.h"

#include <cstring>

#include "os/cycles.h"
#include "os/scheduler.h"
#include "script/scriptfunction.h"

namespace {

constexpr char kInactiveDelayCondition[] = "inactive_delay";
constexpr char kDelayCondition[] = "delay";
constexpr char kActiveIdleCondition[] = "active_idle";
constexpr char kHitCondition[] = "hit";
constexpr char kActiveMissCondition[] = "active_miss";
constexpr char kMissCondition[] = "miss";
constexpr char kPassCondition[] = "pass";
constexpr char kButtonMashCondition[] = "button_mash";
constexpr char kCaptureCondition[] = "capture";
constexpr char kNthCaptureCondition[] = "nth_capture";
constexpr char kAutocaptureCondition[] = "autocapture";
constexpr char kCaptureFailCondition[] = "capture_fail";
constexpr char kStreakCondition[] = "streak";
constexpr char kBlowStreakCondition[] = "blow_streak";
constexpr char kPowerupCondition[] = "powerup";
constexpr char kDoneAction[] = "done";

// Nodes of a condition.
constexpr int kConditionName = 0;
constexpr int kConditionValue = 1;
constexpr int kConditionRepeats = 2;

// Index of the first action after no value, one value, and two values.
constexpr int kActionsAfterName = 1;
constexpr int kActionsAfterValue = 2;
constexpr int kActionsAfterRepeats = 3;

// Index of the first condition of a step.
constexpr int kFirstCondition = 1;

constexpr int kInitialMissLane = 1;
constexpr int kNthCaptureFired = 99999;

} // namespace

WaitInteractiveTask::WaitInteractiveTask(DataArray *pStep, TutorialGameLogic *pLogic)
    : mLogic(pLogic), mStep(pStep), mPassCount(0), mMissCount(0), mHitCount(0), mCaptureCount(0),
      mNthCapture(0), mCaptureFailCount(0), mMissLane(kInitialMissLane), mPromptTime(0.0f),
      mActiveIdleTime(0.0f), mPowerupCount(0), mStreak(0), mAutocaptureCount(0), mMissStreak(0),
      mMashCount(0) {
}

void WaitInteractiveTask::OnStart() {
    GameCallback::Set(this);
    mMissCount = 0;
    mMissLane = kInitialMissLane;
    mHitCount = 0;
    mPassCount = 0;
    mCaptureCount = 0;
    mNthCapture = 0;
    mAutocaptureCount = 0;
    mCaptureFailCount = 0;
    mStreak = 0;
    mPowerupCount = 0;
    mPromptTime = TheSongScheduler.mTime;
    mActiveIdleTime = TheSongScheduler.mTime;
}

void WaitInteractiveTask::OnStop() {
    GameCallback::Set(nullptr);
}

void WaitInteractiveTask::OnPoll() {
    DataArray *pCondition = mStep->FindArray(kInactiveDelayCondition, false);
    if (pCondition != nullptr) {
        const float fNow = TheSongScheduler.mTime;
        if (mLogic->IsPhraseAbsent()) {
            mPromptTime = fNow;
        } else if (mPromptTime + pCondition->Float(kConditionValue) * kMillisecondsPerSecond <
                   fNow) {
            mPromptTime = fNow;
            RunActions(pCondition, kActionsAfterValue);
        }
    }

    pCondition = mStep->FindArray(kDelayCondition, false);
    if (pCondition != nullptr) {
        const float fNow = TheSongScheduler.mTime;
        if (mPromptTime + pCondition->Float(kConditionValue) * kMillisecondsPerSecond < fNow) {
            mPromptTime = fNow;
            RunActions(pCondition, kActionsAfterValue);
        }
    }

    pCondition = mStep->FindArray(kActiveIdleCondition, false);
    if (pCondition != nullptr && mLogic->IsPhraseAbsent()) {
        const float fNow = TheSongScheduler.mTime;
        if (mActiveIdleTime + pCondition->Float(kConditionValue) * kMillisecondsPerSecond < fNow) {
            mActiveIdleTime = fNow;
            RunActions(pCondition, kActionsAfterValue);
        }
    }
}

void WaitInteractiveTask::OnGemPass() {
    mHitCount = 0;
    mMissStreak = 0;
    ++mPassCount;
    EvaluateConditions();
}

void WaitInteractiveTask::OnGemMiss(int nLane) {
    mMissLane = nLane;
    ++mMissCount;
    mHitCount = 0;
    ++mMissStreak;
    mPassCount = 0;
    EvaluateConditions();
}

void WaitInteractiveTask::OnGemHit() {
    mMissCount = 0;
    mPassCount = 0;
    mMissStreak = 0;
    ++mHitCount;
    EvaluateConditions();
}

void WaitInteractiveTask::OnCapture(int nTrack) {
    mStreak = nTrack + 1;
    ++mCaptureCount;
    mMissCount = 0;
    ++mNthCapture;
    mPassCount = 0;
    EvaluateConditions();
}

void WaitInteractiveTask::OnAutocapture() {
    mMissCount = 0;
    mPassCount = 0;
    ++mAutocaptureCount;
    EvaluateConditions();
}

void WaitInteractiveTask::OnStreakBroken([[maybe_unused]] int nStreak) {
    mStreak = -mStreak;
    ++mCaptureFailCount;
    EvaluateConditions();
}

void WaitInteractiveTask::OnPowerup() {
    ++mPowerupCount;
    EvaluateConditions();
}

void WaitInteractiveTask::EvaluateConditions() {
    const int nSize = mStep->Size();
    for (int i = kFirstCondition; i < nSize; ++i) {
        EvaluateCondition(mStep->Array(i));
    }
}

void WaitInteractiveTask::EvaluateCondition(DataArray *pCondition) {
    const char *pszName = pCondition->Sym(kConditionName);
    int *pnCount = nullptr;

    if (strcmp(pszName, kHitCondition) == 0) {
        pnCount = &mHitCount;
    } else if (strcmp(pszName, kActiveMissCondition) == 0) {
        if (mMissCount != 0 && mMissLane == 0) {
            mMissCount = 0;
            RunActions(pCondition, kActionsAfterName);
        }
        return;
    } else if (strcmp(pszName, kMissCondition) == 0) {
        pnCount = &mMissCount;
    } else if (strcmp(pszName, kPassCondition) == 0) {
        pnCount = &mPassCount;
    } else if (strcmp(pszName, kButtonMashCondition) == 0) {
        if (mMissStreak < pCondition->Int(kConditionValue)) {
            return;
        }
        if (mMashCount < pCondition->Int(kConditionRepeats)) {
            ++mMashCount;
            return;
        }
        mMissStreak = 0;
        mMashCount = 0;
        RunActions(pCondition, kActionsAfterRepeats);
        return;
    } else if (strcmp(pszName, kCaptureCondition) == 0) {
        pnCount = &mCaptureCount;
    } else if (strcmp(pszName, kNthCaptureCondition) == 0) {
        if (mNthCapture == pCondition->Int(kConditionValue)) {
            mNthCapture = kNthCaptureFired;
            RunActions(pCondition, kActionsAfterValue);
        }
        return;
    } else if (strcmp(pszName, kAutocaptureCondition) == 0) {
        pnCount = &mAutocaptureCount;
    } else if (strcmp(pszName, kCaptureFailCondition) == 0) {
        pnCount = &mCaptureFailCount;
    } else if (strcmp(pszName, kStreakCondition) == 0) {
        pnCount = &mStreak;
    } else if (strcmp(pszName, kBlowStreakCondition) == 0) {
        if (mStreak < 0 && -mStreak == pCondition->Int(kConditionValue)) {
            mStreak = 0;
            RunActions(pCondition, kActionsAfterValue);
        }
        return;
    } else if (strcmp(pszName, kPowerupCondition) == 0) {
        pnCount = &mPowerupCount;
    } else {
        return;
    }

    if (*pnCount < pCondition->Int(kConditionValue)) {
        return;
    }
    *pnCount = 0;
    RunActions(pCondition, kActionsAfterValue);
}

void WaitInteractiveTask::RunActions(DataArray *pCondition, int nFirst) {
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
