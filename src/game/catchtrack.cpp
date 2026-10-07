#include "game/catchtrack.h"

#include <algorithm>

#include "game/gamecallback.h"
#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "game/gamelogic.h"
#include "game/helptext.h"
#include "game/stats.h"
#include "gfx/gfxmanager.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"

namespace {

constexpr int kNoTick = -1;
constexpr int kFirstBar = 0;
constexpr int kFirstTick = 0;

// The values of the nClear argument of the display's redraws.
constexpr int kKeepGems = 0;
constexpr int kClearGems = 1;

// The guide ticker plays only with one controller.
constexpr int kGuideTickerPads = 1;

// UpdateHint() runs this many bars after a press.
constexpr int kHintDelayBars = 5;

// Bars at the start of each section that are not enabled. An insane solo game has none.
inline int CheckpointBars() {
    if (TheGameDb->mSkillLevel == GameDb::kSkillInsane &&
        TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        return 0;
    }
    return TheGameConfig->mCheckpointBars;
}

} // namespace

void CatchTrack::DeactivateCmd::Execute() {
    mTrack->mLogic->EndPhrase(mTrack);
    for (BackMusic *pMusic : mTrack->mBackMusic) {
        pMusic->Stop();
    }
}

CatchTrack::CatchTrack(GameLogic *pLogic,
                       CatchTrackData *pData,
                       const float *pfMsPerTick,
                       PlayMap *pPlayMap,
                       SectionBoundaries *pSections,
                       int nIndex,
                       int nIntroBars,
                       int nNumBars,
                       int nTicksPerBar,
                       int nFlags)
    : Track(nIndex), mPlayMap(pPlayMap), mLogic(pLogic),
      mState(pData, nNumBars, nTicksPerBar, pPlayMap), mSections(pSections),
      mTicksPerBar(nTicksPerBar), mNumBars(nNumBars),
      mDisplay(&mState, pSections, nIndex, nIntroBars, nNumBars, nTicksPerBar, pPlayMap),
      mMusic(&mState, nTicksPerBar, nNumBars, pPlayMap, nFlags),
      mGuideTicker(&mState, nTicksPerBar), mReceiver(new CaptureReceiver(this)),
      mDeactivateCommand(new DeactivateCmd(this)),
      mHintCommand(NewMemFunCommand(this, &CatchTrack::UpdateHint)),
      mShowHints(!TheGameDb->mTutorial), mCapturer(mReceiver,
                                                   &mState,
                                                   pfMsPerTick,
                                                   pPlayMap,
                                                   pSections,
                                                   nNumBars,
                                                   nIndex,
                                                   nTicksPerBar,
                                                   nFlags),
      mStarted(0), mEnabled(0) {
}

CatchTrack::~CatchTrack() {
    CatchTrack::Stop();
    delete mReceiver;
}

void CatchTrack::AddBackMusic(BackMusic *pMusic) {
    mBackMusic.push_back(pMusic);
}

void CatchTrack::Start() {
    if (mStarted != 0) {
        return;
    }

    mStarted = 1;
    mDisplay.Start();
    mMusic.Start();
    for (BackMusic *pMusic : mBackMusic) {
        pMusic->Stop();
    }
    if (TheGameDb->GetNumPads() != kGuideTickerPads || TheGameConfig->mGuideTicks == 0) {
        return;
    }
    mGuideTicker.Start();
    if (mPlayer != nullptr && TheGameDb->IsLocalPlayer(mPlayer->GetIndex())) {
        mGuideTicker.MuteForTwoBars();
    } else {
        mGuideTicker.Mute();
    }
}

void CatchTrack::Stop() {
    if (mStarted == 0) {
        return;
    }

    mStarted = 0;
    mDisplay.Stop();
    mMusic.Stop();
    mGuideTicker.Stop();
    mCapturer.Stop();
    TheSongScheduler.Cancel(mDeactivateCommand.Get());
    TheSongScheduler.Cancel(mIdleCommand.Get());
    if (mShowHints != 0) {
        TheSongScheduler.Cancel(mHintCommand.Get());
    }
    for (BackMusic *pMusic : mBackMusic) {
        pMusic->Stop();
    }
}

void CatchTrack::SetPlayer(Player *pPlayer) {
    mCapturer.SetPlayer(pPlayer);
    Track::SetPlayer(pPlayer);
    if (pPlayer == nullptr) {
        mGuideTicker.Mute();
        if (mShowHints != 0) {
            TheSongScheduler.Cancel(mHintCommand.Get());
        }
        return;
    }
    if (TheGameDb->IsLocalPlayer(pPlayer->GetIndex())) {
        mGuideTicker.MuteForTwoBars();
        ScheduleHint();
    } else {
        mGuideTicker.Mute();
    }
}

void CatchTrack::Loop(int nBar) {
    mDisplay.Redraw(nBar, kClearGems);
    mCapturer.Restart();
}

void CatchTrack::SetPowerup(int nBar, int nPowerup) {
    mState.SetPowerup(nBar, nPowerup);
}

void CatchTrack::SetBarPowerup(int nBar, int nPowerup) {
    mState.SetWrittenPowerup(nBar, nPowerup);
}

void CatchTrack::Enable(int nBar) {
    mEnabled = 1;
    for (int i = mPlayMap->mLooping ? kFirstBar : nBar; i < mNumBars; ++i) {
        const int nSectionStart = mSections->SectionStart(mSections->SectionAt(i));
        const bool bCheckpoint = (nSectionStart > 0) && ((i - nSectionStart) < CheckpointBars());
        const bool bEmpty = mState.IsWrittenBarEmpty(i);
        mState.SetWrittenEnabled(i, !bCheckpoint && !bEmpty);
    }
    mDisplay.Redraw(nBar, kKeepGems);
    mCapturer.Start();
}

void CatchTrack::HandleInput(Player *pPlayer, const PlayNoteEvent &event) {
    if (event.mState == 0) {
        return;
    }

    if (pPlayer != mPlayer) {
        (void)TheHelpText->ShowCatchBehind(event.mPlayer); // Yes, the binary discards the result.
        return;
    }
    if (mShowHints != 0) {
        ScheduleHint();
    }
    if (TheGameDb->IsLocalPlayer(pPlayer->GetIndex())) {
        mGuideTicker.MuteForTwoBars();
    }
    mCapturer.Press(event.mButton);
}

void CatchTrack::Restart(Player *pPlayer, int nTick) {
    mState.SetEnabled(kFirstBar, mNumBars, true);
    mState.SetCapturedBy(kFirstBar, mNumBars, pPlayer);
    mDisplay.Seek(nTick);
}

bool CatchTrack::HasPhraseAt(int nBar) {
    return mCapturer.IsBarActive(nBar);
}

bool CatchTrack::FindPhrase(int nBar, int *pTick, int *pLane) {
    return mCapturer.GetRunStart(nBar, pTick, pLane);
}

int CatchTrack::GetNextGemTick(int nTick, int *pLane) {
    const GemCursor cursor = mState.GetCursor(nTick);
    if (!cursor.IsValid()) {
        return kNoTick;
    }
    if (pLane != nullptr) {
        *pLane = cursor.GetLane();
    }
    return cursor.GetTick();
}

void CatchTrack::Capture(int nBar, Player *pPlayer) {
    CaptureBars(nBar, pPlayer, false);
    mCapturer.RefreshRun();
}

bool CatchTrack::Autocatch(int nBar, Player *pPlayer) {
    const bool bOpen = mState.IsEnabled(nBar) && (mState.GetCapturedBy(nBar) == nullptr);
    if (!bOpen) {
        const int nNext = nBar + 1;
        if (nBar >= mNumBars - 1 || !mState.IsEnabled(nNext) ||
            mState.GetCapturedBy(nNext) != nullptr) {
            return false;
        }
    }

    if (TheGameDb->IsLocalPlayer(pPlayer->GetIndex())) {
        pPlayer->AddScore(TheGameConfig->mAutocatcherPoints[TheGameDb->mSkillLevel]);
    }
    pPlayer->SetPendingPoints(0, true);
    pPlayer->SetCatching(false);
    CaptureBars(nBar, pPlayer, true);
    mCapturer.RefreshRun();
    mMusic.Refresh();
    if (TheGameCallback != nullptr) {
        TheGameCallback->OnAutocapture();
    }
    return true;
}

void CatchTrack::CaptureBars(int nBar, Player *pPlayer, bool bAuto) {
    int nEndBar = nBar + GetStrandBars() + 1;
    if (!mSections->IsPastEnd(nBar) &&
        nEndBar >= mSections->SectionEnd(mSections->SectionAt(nBar))) {
        nEndBar += CheckpointBars();
    }
    TheStats->CaptureTrack(mIndex, TheSongScheduler.mTick, nEndBar - nBar);
    ActivateBars(nBar, nEndBar, pPlayer, bAuto, false);
    mLogic->CapturePhrase(this, pPlayer, nEndBar, bAuto);
}

void CatchTrack::Freestyle(int nBar, int nBars, Player *pPlayer) {
    ActivateBars(nBar, nBar + nBars + 1, pPlayer, false, true);
    mCapturer.RefreshRun();
    mMusic.Refresh();
}

void CatchTrack::ActivateBars(
    int nStartBar, int nEndBar, Player *pPlayer, bool bAuto, bool bFreestyle) {
    nEndBar = std::min(nEndBar, mPlayMap->GetEndBar());
    mState.SetCapturedBy(nStartBar, nEndBar, pPlayer);
    for (BackMusic *pMusic : mBackMusic) {
        pMusic->Start(mPlayMap);
    }
    TheSongScheduler.Cancel(mDeactivateCommand.Get());
    if (TheGameConfig->mNoDeactivate == 0) {
        const int nLastTick = (nEndBar * mTicksPerBar) - 1;
        if (nLastTick >= TheSongScheduler.mTick) {
            TheSongScheduler.PostAt(mDeactivateCommand.Get(), nLastTick, false);
        }
    }
    TheGfxManager.ShowCapture(pPlayer->GetIndex(),
                              mIndex,
                              bAuto,
                              bFreestyle,
                              static_cast<float>(nStartBar * mTicksPerBar),
                              static_cast<float>(nEndBar * mTicksPerBar));
    mDisplay.RedrawRange(nStartBar, nEndBar, kKeepGems);
}

void CatchTrack::AssignBars(int nBar, int nBars, Player *pPlayer) {
    const int nEndBar = nBar + nBars;
    for (int i = nBar; i < nEndBar; ++i) {
        mState.SetWrittenCapturedBy(i, pPlayer);
    }
}

void CatchTrack::UpdateHint() {
    if (mPlayer == nullptr || !mLogic->HasPhraseThisBar()) {
        return;
    }

    const int nBar = TheSongScheduler.mTick / mTicksPerBar;
    bool bCatchable = false;
    if (mState.IsEnabled(nBar) && mState.GetCapturedBy(nBar) == nullptr) {
        bCatchable = !mState.IsBarEmpty(nBar);
    }
    TheHelpText->ShowEnergizeNotes(bCatchable);
}

void CatchTrack::ScheduleHint() {
    if (mShowHints == 0) {
        return;
    }

    TheSongScheduler.Cancel(mHintCommand.Get());
    const int nTick = std::max(TheSongScheduler.mTick, kFirstTick) + kHintDelayBars * mTicksPerBar;
    if (nTick <= mNumBars * mTicksPerBar) {
        TheSongScheduler.PostAt(mHintCommand.Get(), nTick, false);
    }
}

int CatchTrack::GetStrandBars() {
    return TheGameConfig->mStrandBars[TheGameDb->GetNumPlayers() - 1];
}

void CatchTrack::OnCapture(const GemCursor &cursor) {
    const int nBar = cursor.GetTick() / mTicksPerBar;
    Player *pPlayer = mPlayer;
    pPlayer->CommitPendingPoints(true);
    CaptureBars(nBar, pPlayer, false);
}

void CatchTrack::OnRunLost() {
    Player *pPlayer = mPlayer;
    const int nStreak = (pPlayer != nullptr) ? pPlayer->GetStreak() : 0;
    mLogic->MissPhrase(this);
    if (pPlayer != nullptr && TheGameCallback != nullptr) {
        TheGameCallback->OnStreakBroken(nStreak);
    }
}

void CatchTrack::OnRunStart(const GemCursor &cursor, int nEndBar) {
    mLogic->ContinueStreak(this, cursor.GetTick() / mTicksPerBar, nEndBar);
}

void CatchTrack::SetAutopilot(bool bAutopilot) {
    mCapturer.SetFakeInput(bAutopilot);
}

void CatchTrack::SetNoSeeker(bool bNoSeeker) {
    mCapturer.SetRunHidden(bNoSeeker);
}
