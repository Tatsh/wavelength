#include "game/lyric.h"

#include "game/triggermgr.h"
#include "gfx/gfxmanager.h"
#include "os/debug.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"
#include "script/scriptfunction.h"

namespace {

constexpr char kToggleCommandName[] = "lyric";

} // namespace

Lyric::Lyric(int nTicksPerBar, int nLoopBars)
    : mTicksPerBar(nTicksPerBar), mLoopBars(nLoopBars), mCurrent(),
      mAdvanceCommand(NewMemFunCommand(this, &Lyric::Advance)), mShowing(false) {
    ScriptFunction::Register(ToggleShowing, kToggleCommandName, this);
}

Lyric::~Lyric() {
    Stop();
    ScriptFunction::Unregister(ToggleShowing);
}

void Lyric::AddLyric(int nTick, const char *pszText) {
    if (nTick / mTicksPerBar < mLoopBars) {
        mLyrics.insert(LyricData{nTick, String(pszText)});
    }
}

void Lyric::Start() {
    TheTriggerMgr.LyricEvent("");
    if (!mLyrics.empty()) {
        mCurrent = mLyrics.begin();
        TheSongScheduler.PostAt(mAdvanceCommand.Get(), mCurrent->mTick, false);
    }
}

void Lyric::Stop() {
    TheSongScheduler.Cancel(mAdvanceCommand.Get());
}

void Lyric::Advance() {
    const char *pszText = mCurrent->mText.c_str();
    TheTriggerMgr.LyricEvent(pszText);
    if (mShowing) {
        TheGfxManager.SetLyricText(pszText, true);
    }

    ++mCurrent;
    int nLoop = TheSongScheduler.mTick / mTicksPerBar / mLoopBars;
    if (mCurrent == mLyrics.end()) {
        mCurrent = mLyrics.begin();
        ++nLoop;
        while (mCurrent->mTick < 0) {
            ++mCurrent;
        }
    }

    TheSongScheduler.PostAt(
        mAdvanceCommand.Get(), nLoop * mLoopBars * mTicksPerBar + mCurrent->mTick, false);
}

void Lyric::ToggleShowing([[maybe_unused]] DataArray *pCommand, void *pUserData) {
    auto *pLyric = static_cast<Lyric *>(pUserData);
    pLyric->mShowing = !pLyric->mShowing;
    DebugPrint("CHEAT: lyrics %s\n", pLyric->mShowing ? "showing" : "not showing");
    if (!pLyric->mShowing) {
        TheGfxManager.SetLyricText("", true);
    }
}
