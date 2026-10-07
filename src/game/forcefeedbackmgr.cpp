#include "game/forcefeedbackmgr.h"

#include "game/forcefeedbackmgrbeatcmd.h"
#include "game/forcefeedbackmgrcontroller.h"
#include "game/forcefeedbackmgrmotoreffect.h"
#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "os/joypad.h"
#include "os/scheduler.h"

ForceFeedbackMgr::ForceFeedbackMgr()
    : mBeatCmd(new BeatCmd), mTickDuration(nullptr), mEnabled(0), mPaused(0) {
    mControllers.reserve(kPadCount);
}

ForceFeedbackMgr::~ForceFeedbackMgr() {
    for (auto *pController : mControllers) {
        delete pController;
    }
}

void ForceFeedbackMgr::Start(const float *pTickDuration, int nTicksPerBar) {
    const int nPlayers = TheGameDb->GetNumPads();
    if (nPlayers > kMaxPlayers || TheGameDb->GetDemo() != 0 ||
        TheGameConfig->mForceFeedbackEnabled == 0) {
        return;
    }

    mPaused = 0;
    mEnabled = 1;
    for (unsigned i = 0; i < mControllers.size(); ++i) {
        mControllers[i]->CancelAll();
    }
    for (int nPad = 0; nPad < kPadCount; ++nPad) {
        JoypadSetVibration(nPad, 0, 0);
    }

    const auto nCount = static_cast<unsigned>(nPlayers);
    while (mControllers.size() < nCount) {
        mControllers.push_back(new Controller(static_cast<int>(mControllers.size()), nTicksPerBar));
    }
    while (mControllers.size() > nCount) {
        delete mControllers.back();
        mControllers.pop_back();
    }

    for (unsigned i = 0; i < mControllers.size(); ++i) {
        mControllers[i]->mVibration = TheGameDb->GetOptions()->mForceFeedback;
    }
    mTickDuration = pTickDuration;
}

void ForceFeedbackMgr::StopAll() {
    StopMetronome();
    for (unsigned i = 0; i < mControllers.size(); ++i) {
        mControllers[i]->CancelAll();
    }
    mEnabled = 0;
}

void ForceFeedbackMgr::StartMetronome(int nPeriod, int nTick) {
    if (mEnabled == 0) {
        return;
    }
    StopMetronome();
    mBeatCmd->mPeriod = nPeriod;
    for (unsigned i = 0; i < mControllers.size(); ++i) {
        mControllers[i]->mEnabled = 1;
    }

    const int nRemainder = nTick % nPeriod;
    if (nRemainder != 0) {
        nTick += nPeriod - nRemainder;
    }
    const float fLead = static_cast<float>(TheGameConfig->mBeatLeadMs);
    float fTime = static_cast<float>(nTick) * *mTickDuration - fLead;
    while (fTime < TheSongScheduler.mTime) {
        nTick += nPeriod;
        fTime = static_cast<float>(nTick) * *mTickDuration - fLead;
    }
    TheSongScheduler.PostAtTime(mBeatCmd.Get(), fTime, false);
}

void ForceFeedbackMgr::StopMetronome() {
    TheSongScheduler.Cancel(mBeatCmd.Get());
}

void ForceFeedbackMgr::SetBeatEnabled(int nPad, bool bEnabled) {
    if (mEnabled != 0) {
        mControllers[nPad]->mEnabled = bEnabled ? 1 : 0;
    }
}

void ForceFeedbackMgr::SetVibration(int nController, int nSmallMotor, int nBigMotor) {
    Controller *pController = mControllers[nController];
    pController->mSmallMotor = nSmallMotor;
    pController->mBigMotor = nBigMotor;
    if (mPaused == 0 && pController->mVibration != 0) {
        JoypadSetVibration(nController, nSmallMotor, nBigMotor);
    }
}

void ForceFeedbackMgr::PlayBumpEffect(int nController) {
    if (mEnabled != 0) {
        PlayEffect(nController, kEffectBump);
    }
}

void ForceFeedbackMgr::PlayCrippleEffect(int nController) {
    if (mEnabled != 0) {
        PlayEffect(nController, kEffectCripple);
    }
}

void ForceFeedbackMgr::PlayAutocatchEffect(int nController) {
    if (mEnabled != 0) {
        PlayEffect(nController, kEffectAutocatch);
    }
}

void ForceFeedbackMgr::PlayEffect(int nController, int nEffect) {
    Controller *pController = mControllers[nController];
    pController->CancelAll();
    pController->mEffects[nEffect]->Start();
}

void ForceFeedbackMgr::Pause() {
    if (mEnabled != 0 && mPaused == 0) {
        mPaused = 1;
        for (int nPad = 0; nPad < kPadCount; ++nPad) {
            JoypadSetVibration(nPad, 0, 0);
        }
    }
}

void ForceFeedbackMgr::Resume() {
    if (mEnabled == 0 || mPaused == 0) {
        return;
    }
    mPaused = 0;
    for (unsigned i = 0; i < mControllers.size(); ++i) {
        Controller *pController = mControllers[i];
        pController->mVibration = TheGameDb->GetOptions()->mForceFeedback;
        if (pController->mVibration != 0) {
            JoypadSetVibration(
                static_cast<int>(i), pController->mSmallMotor, pController->mBigMotor);
        }
    }
}

ForceFeedbackMgr *ForceFeedbackMgr::shared() {
    static ForceFeedbackMgr sInstance;
    return &sInstance;
}

ForceFeedbackMgr *TheForceFeedbackMgr = ForceFeedbackMgr::shared();
