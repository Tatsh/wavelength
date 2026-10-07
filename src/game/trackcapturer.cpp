#include "game/trackcapturer.h"

#include "game/gamecallback.h"
#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "game/helptext.h"
#include "game/points.h"
#include "game/stats.h"
#include "gfx/gfxmanager.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"
#include "synth/fxmidi.h"

namespace {

constexpr int kNoBar = -1;

// The bars ahead of a bar a run must start within.
constexpr int kRunSearchBars = 6;

// The caught gems that fill the catch meter.
constexpr int kMeterFullGems = 20;
constexpr float kMeterFull = 1.0f;

// A press within a quarter bar of an active bar is not explained.
constexpr int kHintMarginDivisor = 4;

constexpr int kNoPowerup = 0;

} // namespace

TrackCapturer::Faker::Faker(TrackCapturer *pCapturer)
    : mCapturer(pCapturer), mCursor(pCapturer->mState->GetCursor()),
      mUpdateCmd(NewMemFunCommand(this, &Faker::Update)), mEnabled(1) {
}

void TrackCapturer::Faker::Start() {
    TheSongScheduler.Cancel(mUpdateCmd.Get());
    mCursor = mCapturer->mState->GetCursor(TheSongScheduler.mTick);
    if (mCursor.IsValid()) {
        TheSongScheduler.PostAt(mUpdateCmd.Get(), mCursor.GetTick(), false);
    }
}

void TrackCapturer::Faker::Stop() {
    TheSongScheduler.Cancel(mUpdateCmd.Get());
}

void TrackCapturer::Faker::Update() {
    const int nBar = mCursor.GetTick() / mCapturer->mTicksPerBar;
    if (mEnabled && mCapturer->mPlayer != nullptr && mCapturer->IsBarActive(nBar)) {
        mCapturer->Press(mCursor.GetLane());
    }
    (void)mCursor.Next(); // The binary discards the returned copy.
    if (mCursor.IsValid()) {
        TheSongScheduler.PostAt(mUpdateCmd.Get(), mCursor.GetTick(), false);
    }
}

TrackCapturer::TrackCapturer(Receiver *pReceiver,
                             CatchTrackState *pState,
                             const float *pMsPerTick,
                             PlayMap *pPlayMap,
                             int nReserved,
                             int,
                             int nTrack,
                             int nTicksPerBar,
                             int nStopPreviousNote)
    : mCatchReceiver(new CatchReceiver(this)), mReserved0C(nReserved),
      mCatcher(mCatchReceiver, pState, pMsPerTick, nTrack, nTicksPerBar), mNetFaker(nullptr),
      mFaker(nullptr), mPlayer(nullptr), mLastMuse(nullptr), mStopPreviousNote(nStopPreviousNote),
      mEndBar(pPlayMap->GetEndBar()), mStarted(0), mCaptureStartCmd(new SetCaptureStartCmd(this)),
      mGemsCaught(0), mRunStart(kNoBar), mRunEnd(kNoBar), mRunHidden(0) {
    mState = pState;
    mTicksPerBar = nTicksPerBar;
    mReceiver = pReceiver;
    mTrack = nTrack;
    mRunGems = 0;
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        mNetFaker = new NetFaker(this, 1);
    }
    if ((TheGameConfig->mFakeInput || TheGameDb->mTutorial) &&
        TheGameDb->mCommunity != GameDb::kCommunityOnline) {
        mFaker = new Faker(this);
        mFaker->Start();
        mFaker->mEnabled = TheGameConfig->mFakeInput;
    }
}

TrackCapturer::~TrackCapturer() {
    Stop();
    delete mCatchReceiver;
    if (mNetFaker != nullptr) {
        mNetFaker->Stop();
        delete mNetFaker;
    }
    if (mFaker != nullptr) {
        mFaker->Stop();
        delete mFaker;
    }
}

void TrackCapturer::Start() {
    if (mStarted) {
        return;
    }
    mStarted = 1;
    const int nBar = TheSongScheduler.mTick / mTicksPerBar;
    UpdateRun(nBar > kNoBar ? nBar : 0);
    mCatcher.Start();
    if (mNetFaker != nullptr) {
        mNetFaker->Start();
    }
    if (mFaker != nullptr) {
        mFaker->Start();
    }
}

void TrackCapturer::Stop() {
    if (!mStarted) {
        return;
    }
    mStarted = 0;
    if (mPlayer != nullptr) {
        HideRun();
    }
    if (mNetFaker != nullptr) {
        mNetFaker->Stop();
    }
    if (mFaker != nullptr) {
        mFaker->Stop();
    }
    TheSongScheduler.Cancel(mCaptureStartCmd.Get());
    mCatcher.Stop();
    mLastMuse = nullptr;
}

void TrackCapturer::Restart() {
    if (mStarted) {
        Stop();
        Start();
    }
}

void TrackCapturer::SetPlayer(Player *pPlayer) {
    if (mPlayer != nullptr) {
        HideRun();
    }
    Player *pPrevious = mPlayer;
    mPlayer = pPlayer;
    if (pPrevious != nullptr) {
        const int nTick = TheSongScheduler.mTick;
        if (pPrevious->GetCatching()) {
            EndRun(nTick, 0);
        }
    }
    UpdateMeter();
    ShowRun(1);
}

void TrackCapturer::Press(int nLane) {
    if (mStarted) {
        mCatcher.Catch(nLane);
    } else {
        ShowPressHint(TheSongScheduler.mTick);
    }
}

void TrackCapturer::SetFakeInput(int nEnabled) {
    mFaker->mEnabled = nEnabled;
}

void TrackCapturer::SetRunHidden(int nHidden) {
    mRunHidden = nHidden;
    UpdateMeter();
    ShowRun(1);
}

void TrackCapturer::RefreshRun() {
    UpdateRun(TheSongScheduler.mTick / mTicksPerBar);
}

bool TrackCapturer::GetRunStart(int nBar, int *pTick, int *pLane) {
    if (!(nBar < mEndBar)) {
        return true;
    }
    int nStart;
    int nEnd;
    if (!FindRun(nBar, nBar + kRunSearchBars, &nStart, &nEnd)) {
        return false;
    }
    const GemCursor cursor = mState->GetCursor(nStart * mTicksPerBar);
    *pTick = cursor.GetTick();
    *pLane = cursor.GetLane();
    return true;
}

bool TrackCapturer::IsBarActive(int nBar) {
    return mState->IsEnabled(nBar) && !mState->GetCapturedBy(nBar);
}

void TrackCapturer::HitGem(int nTick, const GemCursor &cursor, bool bRemote) {
    if (cursor.GetTick() < mPlayer->mLastHitTick) {
        MissPress(nTick, cursor.GetLane());
        return;
    }
    mPlayer->mLastHitTick = cursor.GetTick();
    if (mStopPreviousNote && mLastMuse != nullptr) {
        mLastMuse->Stop();
    }
    int nEarly = cursor.GetTick() - nTick;
    if (nEarly <= kNoBar) {
        nEarly = 0;
    }
    Muse *pMuse = cursor.GetMuse();
    pMuse->PlayFrom(&TheSongScheduler, -nEarly);
    mLastMuse = pMuse;

    const int nGemTick = cursor.GetTick();
    const int nBar = nGemTick / mTicksPerBar;
    const int nPlayer = mPlayer->GetIndex();
    if (mRunStart != kNoBar && !(nBar < mRunStart)) {
        if (!bRemote) {
            mPlayer->SetCatching(true);
            if (mGemsCaught == 0) {
                mReceiver->OnRunStart(cursor, mRunEnd);
                mPlayer->SetPendingPoints(GetPhrasePoints(cursor, mRunStart, mRunEnd, mTicksPerBar),
                                          true);
            }
        }
        ++mGemsCaught;
        UpdateMeter();
        if (IsLastGemOfBar(cursor)) {
            CompleteBar(cursor, bRemote);
        }
    }

    const float fTick = static_cast<float>(nGemTick);
    const int nLane = cursor.GetLane();
    TheGfxManager.ShowGemResult(mTrack, nLane, true, nPlayer, mState->GetPowerup(nBar), fTick);
    TheGfxManager.PlaceGem(mTrack, nLane, nPlayer, fTick, 0, 0);
    if (TheGameCallback != nullptr) {
        TheGameCallback->OnGemHit();
    }
}

void TrackCapturer::MissGem(int, const GemCursor &cursor, bool bRemote) {
    mLastMuse = nullptr;
    if (!bRemote && mPlayer != nullptr && !TheGameDb->IsLocalPlayer(mPlayer->GetIndex())) {
        return;
    }
    if (cursor.GetTick() >= 0) {
        EndRunIfStarted(cursor.GetTick(), bRemote);
    }
    if (mPlayer != nullptr && TheGameCallback != nullptr) {
        TheGameCallback->OnGemPass();
    }
}

GemCursor TrackCapturer::GetCursor() {
    return mState->GetCursor();
}

bool TrackCapturer::IsLastGemOfBar(const GemCursor &cursor) {
    GemCursor next(cursor);
    (void)next.Next(); // The binary discards the returned copy.
    if (!next.IsValid()) {
        return true;
    }
    return cursor.GetTick() / mTicksPerBar < next.GetTick() / mTicksPerBar;
}

int TrackCapturer::FindActiveBar(int nFrom, int nTo) {
    int nBar = nFrom > kNoBar ? nFrom : 0;
    const int nLimit = mEndBar < nTo ? mEndBar : nTo;
    do {
        if (IsBarActive(nBar)) {
            return nBar;
        }
        ++nBar;
    } while (nBar < nLimit);
    return kNoBar;
}

int TrackCapturer::FindRunEnd(int nStart, int nMaxBars) {
    (void)IsBarActive(nStart); // The binary discards the result.
    int nBar = nStart + 1;
    while (nBar < mEndBar) {
        if (!(nBar - nStart < nMaxBars) || !IsBarActive(nBar)) {
            return nBar;
        }
        ++nBar;
    }
    return nBar;
}

bool TrackCapturer::FindRun(int nFrom, int nTo, int *pStart, int *pEnd) {
    const int nLimit = mEndBar < nTo ? mEndBar : nTo;
    if (mEndBar < nFrom) {
        nFrom = mEndBar;
    }
    if (!(nFrom < nLimit)) {
        return false;
    }
    const int nMaxBars = TheGameConfig->mBarCaptureThreshold;
    const int nStart = FindActiveBar(nFrom, nLimit);
    *pStart = nStart;
    if (nStart == kNoBar) {
        return false;
    }
    *pEnd = FindRunEnd(nStart, nMaxBars);
    return true;
}

void TrackCapturer::UpdateRun(int nBar) {
    if (FindRun(nBar, nBar + kRunSearchBars, &mRunStart, &mRunEnd)) {
        mRunGems = 0;
        GemCursor cursor = mState->GetCursor(mRunStart * mTicksPerBar);
        const int nEndTick = mRunEnd * mTicksPerBar;
        while (cursor.IsValid() && cursor.GetTick() < nEndTick) {
            ++mRunGems;
            (void)cursor.Next(); // The binary discards the returned copy.
        }
        UpdateMeter();
    } else {
        mRunStart = kNoBar;
        mRunEnd = kNoBar;
        mCaptureStartCmd->mBar = nBar + 1;
        TheSongScheduler.Cancel(mCaptureStartCmd.Get());
        TheSongScheduler.PostIn(mCaptureStartCmd.Get(), mTicksPerBar, false);
    }
    ShowRun(0);
}

void TrackCapturer::EndRunIfStarted(int nTick, int nQuiet) {
    const int nBar = nTick / mTicksPerBar;
    if (mRunStart != kNoBar && !(nBar < mRunStart)) {
        EndRun(nTick, nQuiet);
    }
}

void TrackCapturer::EndRun(int nTick, int nQuiet) {
    if (!nQuiet) {
        mReceiver->OnRunLost();
        if (mPlayer != nullptr) {
            mPlayer->SetCatching(false);
            mPlayer->LosePendingPoints();
        }
    }
    mGemsCaught = 0;
    UpdateMeter();
    UpdateRun(nTick / mTicksPerBar + 1);
}

void TrackCapturer::CompleteBar(const GemCursor &cursor, int nQuiet) {
    (void)cursor.IsValid(); // The binary discards the result and the first tick.
    (void)cursor.GetTick();
    const int nBar = cursor.GetTick() / mTicksPerBar;
    if (!nQuiet) {
        const int nPowerup = mState->GetPowerup(nBar);
        if (nPowerup != kNoPowerup) {
            mPlayer->SetPowerup(nPowerup);
            TheStats->CatchPowerup(mPlayer->GetIndex(), TheSongScheduler.mTick, nPowerup);
            FxMidi::PlayPowerupCatchSound(nPowerup);
        }
    }
    const int nLastBar = mRunEnd - 1;
    if (nBar == nLastBar || mState->IsBarEmpty(nLastBar)) {
        if (TheGameConfig->mNoCapture) {
            EndRun(cursor.GetTick(), nQuiet);
        } else {
            Capture(cursor, nQuiet);
        }
    }
}

void TrackCapturer::Capture(const GemCursor &cursor, int nQuiet) {
    const int nBar = cursor.GetTick() / mTicksPerBar;
    if (!nQuiet) {
        mReceiver->OnCapture(cursor);
        mPlayer->SetCatching(false);
    }
    UpdateRun(nBar + 1);
    mGemsCaught = 0;
    mPlayer->SetPendingPoints(0, true);
    if (TheGameCallback != nullptr) {
        TheGameCallback->OnCapture(mPlayer->GetStreak());
    }
}

void TrackCapturer::UpdateMeter() {
    if (mPlayer == nullptr) {
        return;
    }
    float fLevel = static_cast<float>(mGemsCaught);
    if (mGemsCaught < kMeterFullGems) {
        fLevel /= static_cast<float>(kMeterFullGems);
    } else {
        fLevel = kMeterFull;
    }
    TheGfxManager.SetCatchMeter(mPlayer->GetIndex(), mRunHidden ? 0.0f : fLevel);
}

void TrackCapturer::HideRun() {
    TheGfxManager.ShowPhrase(mPlayer->GetIndex(), mTrack, false, 0.0f, 0.0f, 0, true);
}

void TrackCapturer::ShowRun(int nStyle) {
    if (mPlayer == nullptr) {
        return;
    }
    if (mRunStart == kNoBar || mRunHidden) {
        HideRun();
        return;
    }
    TheGfxManager.ShowPhrase(mPlayer->GetIndex(),
                             mTrack,
                             true,
                             static_cast<float>(mRunStart * mTicksPerBar),
                             static_cast<float>(mRunEnd * mTicksPerBar - 1),
                             nStyle,
                             true);
}

void TrackCapturer::ShowPressHint(int nTick) {
    FxMidi::PlaySound1();
    if (nTick <= 0) {
        return;
    }
    const int nMargin = mTicksPerBar / kHintMarginDivisor;
    const int nBar = nTick / mTicksPerBar;
    const int nPosition = nTick % mTicksPerBar;
    bool bShow = true;
    if (nPosition < nMargin && nBar > 0 && IsBarActive(nBar - 1)) {
        bShow = false;
    } else if (mTicksPerBar - nPosition < nMargin && nBar < mEndBar - 1 && IsBarActive(nBar + 1)) {
        bShow = false;
    }
    if (bShow) {
        (void)TheHelpText->ShowNotesAreEnergized(); // The binary discards the result.
    }
}

void TrackCapturer::MissPress(int nTick, int nLane) {
    if (nTick < 0) {
        return;
    }
    if (IsBarActive(nTick / mTicksPerBar)) {
        FxMidi::PlaySound0();
        TheGfxManager.ShowGemResult(
            mTrack, nLane, false, mPlayer->GetIndex(), 0, static_cast<float>(nTick));
        if (TheGameCallback != nullptr) {
            TheGameCallback->OnGemMiss(1);
        }
    } else {
        ShowPressHint(nTick);
        if (TheGameCallback != nullptr) {
            TheGameCallback->OnGemMiss(0);
        }
    }
    EndRunIfStarted(nTick, 0);
}
