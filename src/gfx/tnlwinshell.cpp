#include "gfx/tnlwinshell.h"

#include "game/gamedb.h"
#include "game/triggermgr.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "rnd/manager.h"
#include "synth/fxmidi.h"

namespace {

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(Rnd::TheManager.Find(pszName));
}

} // namespace

TnlWinShell::TnlWinShell() : mView(nullptr), mStartTick(kNoTick), mDuration(0.0f), mPath(kNoPath) {
    mView = FindObject<Rnd::View>("win_shell.view");
    FindObject<Rnd::View>("win_arms.view")->AddDraw(mView, nullptr);
    Reset();
}

TnlWinShell::~TnlWinShell() {
}

void TnlWinShell::LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *, int) {
    DataArray *pShell;
    FindConfigArray(pConfig, pDefaults, "win_shell", &pShell, true);
    pShell->FindFloat("duration", &mDuration, true);
    float flAnimLength;
    pShell->FindFloat("anim_length", &flAnimLength, true);
    if (mDuration == 0.0f) {
        mFrameRate = 1.0f;
    } else {
        mFrameRate = flAnimLength / mDuration;
    }
}

void TnlWinShell::Reset() {
    mView->SetShowing(0);
    mPath = kNoPath;
    mStartTick = kNoTick;
}

void TnlWinShell::Start(int nPath) {
    mPath = nPath;
    mStartTick = TheGameDb->mSongTick;
}

void TnlWinShell::Update() {
    if (mStartTick == kNoTick) {
        return;
    }
    const float flElapsed = TheGameDb->mSongTick - mStartTick;
    if (mDuration <= flElapsed) {
        if (mPath >= 0) {
            TheTriggerMgr.PathUnlockedEvent(mPath);
            FxMidi::PlayUnlock();
            TheGfxManager.ShowHud(false);
        }
        Reset();
        return;
    }
    mView->SetFrame(flElapsed * mFrameRate);
    mView->UpdateWorldXfm(nullptr, 0);
    mView->SetShowing(1);
}
