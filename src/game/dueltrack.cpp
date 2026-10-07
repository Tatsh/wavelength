#include "game/dueltrack.h"

#include "game/duellogic.h"
#include "game/forcefeedbackmgr.h"
#include "game/gamecallback.h"
#include "game/gamedb.h"
#include "game/points.h"
#include "gfx/gfxmanager.h"
#include "msg/capturepacket.h"
#include "netflow/nettransport.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"
#include "synth/fxmidi.h"

namespace {

constexpr int kNoBar = -1;
constexpr int kNoPad = -1;
constexpr int kNoPlayer = -1;

constexpr int kPhraseBonus = 10;

// OnPhraseTick() first runs seven eighths into the first phrase.
constexpr int kFirstPhraseEighths = 7;
constexpr int kEighths = 8;

// The catch meter is full at this many gems caught.
constexpr int kFullMeterGems = 20;
constexpr float kFullMeter = 1.0f;

// A refused press is marked for a quarter of a bar.
constexpr int kRejectDivisor = 4;

// Yes, the binary reports lane 1 for every press that caught no gem.
constexpr int kCallbackMissLane = 1;

constexpr int kPitchShade = 64;
constexpr int kCatchShade = 128;
constexpr int kBarCameraPlayer = 0;
constexpr bool kBarVisible = false;
constexpr int kGemResultFlags = 0;
constexpr int kCaptureStyle = 0;
constexpr bool kCaptureQuiet = false;
constexpr bool kClearAllGems = false;
constexpr int kPhraseStyle = 0;
constexpr bool kPhraseSlide = true;
constexpr float kNoPhraseTick = 0.0f;

} // namespace

DuelTrack::DuelTrack(DuelLogic *pLogic,
                     Player *pPlayer,
                     CatchTrackData *pCatchGems,
                     CatchTrackData *pPitchGems,
                     PitchTrackRiffData *pRiffData,
                     DuelPatternTable *pPatterns,
                     const float *pTickDuration,
                     PlayMap *pPlayMap,
                     [[maybe_unused]] SectionBoundaries *pSections,
                     int nSide,
                     int nOpponentSide,
                     [[maybe_unused]] int nLeadInBars,
                     int nNumBars,
                     int nTicksPerBar,
                     int nPhraseBars,
                     bool bEasiest)
    : Track(nSide), mSidePlayer(pPlayer), mCatchReceiver(new CatchReceiver(this)),
      mPitchReceiver(new PitchReceiver(this)), mPatterns(pPatterns), mPlayMap(pPlayMap),
      mLogic(pLogic), mNumBars(nNumBars), mTicksPerBar(nTicksPerBar), mPhraseBars(nPhraseBars),
      mFirstPhraseTick(nTicksPerBar * nPhraseBars * kFirstPhraseEighths / kEighths),
      mCatchState(pCatchGems, nNumBars, nTicksPerBar, pPlayMap),
      mPitchState(pPitchGems, nNumBars, nTicksPerBar, pPlayMap), mMode(kModeIdle),
      mGemCatcher(mCatchReceiver, &mCatchState, pTickDuration, nSide, nTicksPerBar),
      mPitcher(this,
               mPitchReceiver,
               pRiffData,
               pPitchGems,
               pPlayMap,
               pPatterns,
               nTicksPerBar * nPhraseBars,
               nOpponentSide,
               nTicksPerBar,
               nPhraseBars,
               mFirstPhraseTick,
               bEasiest),
      mCatchMissed(0), mCatchMade(0), mCatchPoints(0),
      mPhraseCommand(NewMemFunCommand(this, &DuelTrack::OnPhraseTick)), mCaughtGems(0),
      mPhraseGems(0), mCatchBar(kNoBar), mCatchEndBar(kNoBar), mRejectEndTick(0),
      mPhraseBonus(kPhraseBonus), mPitchedGems(0), mMuse(nullptr), mNetFaker(nullptr),
      mIgnoreMisses(0) {
    // Yes, the binary sets mPitchPoints only once the first gem of a phrase is placed.
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        mNetFaker = new NetFaker(this);
    }
}

DuelTrack::~DuelTrack() {
    TheSongScheduler.Cancel(mPhraseCommand.Get());
    delete mCatchReceiver;
    delete mPitchReceiver;
    if (mNetFaker != nullptr) {
        mNetFaker->Stop();
        delete mNetFaker;
    }
}

void DuelTrack::Start() {
    TheSongScheduler.PostAt(mPhraseCommand.Get(), mFirstPhraseTick, false);
    if (mNetFaker != nullptr) {
        mNetFaker->Start();
    }
}

void DuelTrack::Stop() {
    if (mNetFaker != nullptr) {
        mNetFaker->Stop();
    }
}

void DuelTrack::ShowBars(int nMode, int nStartBar, int nEndBar) {
    const bool bPlayable = nMode == kModePitch || nMode == kModeCatch;
    const int nShade = nMode == kModePitch ? kPitchShade : kCatchShade;
    for (int nBar = nStartBar; nBar < nEndBar; ++nBar) {
        // Yes, the binary shows every bar with the instrument of the first.
        TheGfxManager.SetBar(mIndex,
                             kBarCameraPlayer,
                             kNoPlayer,
                             bPlayable,
                             kBarVisible,
                             nShade,
                             static_cast<signed char>(mLogic->GetBarInstrument(nStartBar)),
                             static_cast<float>(nBar * mTicksPerBar),
                             static_cast<float>(mTicksPerBar));
    }
}

void DuelTrack::ShowPhrase(int nMode, int nStartBar, int nEndBar) {
    ShowBars(nMode, nStartBar, nEndBar);
    if (nMode == kModePitch) {
        mPitcher.ShowPattern(nStartBar);
    }
}

void DuelTrack::SetMode(int nMode, int nBar) {
    mMode = nMode;
    mCatchMissed = 0;
    mCatchMade = 0;
    if (TheGameDb->IsLocalPlayer(mSidePlayer->GetIndex())) {
        mSidePlayer->SetCatching(nMode == kModeCatch);
    }

    if (nMode == kModeCatch) {
        mCatchBar = nBar;
        mCatchEndBar = nBar + mPhraseBars;
        mGemCatcher.Reset();
        mCatchState.SetEnabled(mCatchBar, mCatchEndBar, true);
        const int nEndTick = mCatchEndBar * mTicksPerBar;
        mPhraseGems = 0;
        mCaughtGems = 0;
        GemCursor cursor = mCatchState.GetCursor(mCatchBar * mTicksPerBar);
        if (!mIgnoreMisses && (!cursor.IsValid() || cursor.GetTick() >= nEndTick)) {
            mCatchPoints = mPhraseBonus;
            if (TheGameDb->mCommunity != GameDb::kCommunityOnline ||
                TheGameDb->IsLocalPlayer(mSidePlayer->GetIndex())) {
                mLogic->OnCatchMade(this, mSidePlayer, false);
                mCatchMade = 1;
                if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
                    CapturePacket packet(mIndex, nBar, false);
                    TheNetTransport->Send(packet);
                }
            }
        } else {
            mCatchPoints =
                GetPhrasePoints(cursor, mCatchBar, mCatchEndBar, mTicksPerBar) + mPhraseBonus;
            while (cursor.IsValid() && cursor.GetTick() < nEndTick) {
                ++mPhraseGems;
                (void)cursor.Next();
            }
        }
    } else if (nMode == kModePitch) {
        mPitcher.ShowPattern(nBar);
    }

    const int nPad = mSidePlayer->GetPadNum();
    if (nMode != kModeIdle) {
        ShowBars(nMode, nBar, nBar + mPhraseBars);
    }
    if (nPad != kNoPad) {
        TheForceFeedbackMgr->SetBeatEnabled(nPad, nMode != kModeIdle);
    }
}

void DuelTrack::CountGem() {
    ++mPhraseGems;
}

void DuelTrack::DropBars(int nStartBar, int nEndBar) {
    mCatchState.ClearBars(nStartBar, nEndBar);
}

void DuelTrack::Rewind() {
    mGemCatcher.Reset();
    const GemCursor cursor = mCatchState.GetCursor(TheSongScheduler.mTick);
    if (mNetFaker != nullptr) {
        mNetFaker->SetCursor(cursor);
    }
}

void DuelTrack::CancelPhrase() {
    TheSongScheduler.Cancel(mPhraseCommand.Get());
}

void DuelTrack::HandleInput(Player *pPlayer, const PlayNoteEvent &event) {
    if (pPlayer != mPlayer || event.mState == 0) {
        return;
    }
    if (mMode == kModePitch) {
        if (mCatchMissed) {
            FxMidi::PlaySound1();
        } else {
            mPitcher.Press(event.mButton);
        }
    } else if (mMode == kModeCatch) {
        if (mCatchMade) {
            FxMidi::PlaySound1();
        } else {
            mGemCatcher.Catch(event.mButton);
        }
    }
}

void DuelTrack::HandleInput([[maybe_unused]] Player *pPlayer,
                            [[maybe_unused]] const StickEvent<2> &event) {
}

void DuelTrack::HandleInput([[maybe_unused]] Player *pPlayer,
                            [[maybe_unused]] const StickEvent<6> &event) {
}

bool DuelTrack::IsLastGemOfBar(const GemCursor &cursor) {
    GemCursor next(cursor);
    (void)next.Next();
    if (!next.IsValid()) {
        return true;
    }
    return cursor.GetTick() / mTicksPerBar < next.GetTick() / mTicksPerBar;
}

void DuelTrack::HitGem(int nTick, const GemCursor &cursor, bool bRemote) {
    if (bRemote && mCatchMissed) {
        return;
    }
    Player *pPlayer = mPlayer;
    int nLateTicks = cursor.GetTick() - nTick;
    if (nLateTicks < 0) {
        nLateTicks = 0;
    }
    if (mMuse != nullptr) {
        mMuse->Stop();
    }
    mMuse = cursor.GetMuse();
    mMuse->PlayFrom(&TheSongScheduler, -nLateTicks);

    const int nGemTick = cursor.GetTick();
    const float fGemTick = static_cast<float>(nGemTick);
    const int nBar = nGemTick / mTicksPerBar;
    const int nLane = cursor.GetLane();
    TheGfxManager.ShowGemResult(
        mIndex, nLane, true, pPlayer->GetIndex(), kGemResultFlags, fGemTick);
    TheGfxManager.RemoveGem(mIndex, nLane, fGemTick);
    if (mCatchMissed || mCatchBar == kNoBar || nBar < mCatchBar) {
        return;
    }

    if (!bRemote) {
        pPlayer->SetCatching(true);
    }
    ++mCaughtGems;
    ShowCatchProgress();
    if (IsLastGemOfBar(cursor)) {
        CheckCatchMade(cursor, bRemote);
    }
}

void DuelTrack::ShowCatchProgress() {
    const float fLevel = mCaughtGems < kFullMeterGems ?
                             static_cast<float>(mCaughtGems) / static_cast<float>(kFullMeterGems) :
                             kFullMeter;
    TheGfxManager.SetCatchMeter(mSidePlayer->GetIndex(), fLevel);
}

void DuelTrack::MissGem(int nTick, const GemCursor &cursor, [[maybe_unused]] bool bRemote) {
    if (nTick < 0 || mIgnoreMisses || !TheGameDb->IsLocalPlayer(mSidePlayer->GetIndex())) {
        return;
    }
    CheckCatchMissed(cursor.GetTick());
}

void DuelTrack::OnPressMissed(int nTick, int nLane) {
    if (nTick < 0) {
        return;
    }
    (void)(nTick / mTicksPerBar); // Yes, the binary divides and discards the bar.
    FxMidi::PlaySound0();
    TheGfxManager.ShowGemResult(
        mIndex, nLane, false, mPlayer->GetIndex(), kGemResultFlags, static_cast<float>(nTick));
    if (TheGameCallback != nullptr) {
        TheGameCallback->OnGemMiss(kCallbackMissLane);
    }
    if (!mCatchMissed) {
        CheckCatchMissed(nTick);
    }
}

void DuelTrack::OnPressRejected(int nTick) {
    FxMidi::PlaySound0();
    mRejectEndTick = nTick + mTicksPerBar / kRejectDivisor;
    TheGfxManager.ClearGems(
        mIndex, kClearAllGems, static_cast<float>(nTick), static_cast<float>(mRejectEndTick));
}

void DuelTrack::OnGemPlaced(int nTick) {
    if (++mPitchedGems == 1) {
        mLogic->RewindCatcherTrack();
        mPitchPoints = 0;
    }
    mPitchPoints += GetGemPoints(nTick);
}

void DuelTrack::CheckCatchMissed(int nTick) {
    const int nBar = nTick / mTicksPerBar;
    if (mCatchBar == kNoBar || nBar < mCatchBar) {
        return;
    }
    MissCatch();
}

void DuelTrack::MissCatch() {
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline &&
        TheGameDb->IsLocalPlayer(mPlayer->GetIndex())) {
        CapturePacket packet(mIndex, mCatchBar, false);
        packet.mValid = false;
        TheNetTransport->Send(packet);
    }
    mLogic->OnCatchMissed(this, mSidePlayer, mCatchBar);
    mCaughtGems = 0;
    mCatchMissed = 1;
    mCatchBar = kNoBar;
    ShowCatchProgress();
    TheGfxManager.ShowPhrase(mSidePlayer->GetIndex(),
                             mIndex,
                             false,
                             kNoPhraseTick,
                             kNoPhraseTick,
                             kPhraseStyle,
                             kPhraseSlide);
}

void DuelTrack::CheckCatchMade(const GemCursor &cursor, bool bRemote) {
    if (bRemote) {
        return;
    }
    (void)cursor.IsValid(); // Yes, the binary discards this call's result.
    (void)cursor.GetTick(); // Yes, the binary discards this call's result.
    const int nBar = cursor.GetTick() / mTicksPerBar;
    if (mCaughtGems == mPhraseGems) {
        MakeCatch(nBar);
    }
}

void DuelTrack::MakeCatch(int nBar) {
    const int nCatchTick = mCatchBar * mTicksPerBar;
    const int nCatchEndTick = (mCatchBar + mPhraseBars) * mTicksPerBar;
    // Yes, the binary multiplies the end tick by the bar length a second time.
    TheGfxManager.ShowCapture(mPlayer->GetIndex(),
                              mIndex,
                              kCaptureStyle,
                              kCaptureQuiet,
                              static_cast<float>(nCatchTick),
                              static_cast<float>(nCatchEndTick * mTicksPerBar));
    mLogic->OnCatchMade(this, mSidePlayer, true);
    mCatchBar = kNoBar;
    mCaughtGems = 0;
    mCatchMade = 1;
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline &&
        TheGameDb->IsLocalPlayer(mPlayer->GetIndex())) {
        CapturePacket packet(mIndex, nBar, false);
        TheNetTransport->Send(packet);
    }
}

void DuelTrack::OnPhraseTick() {
    mPitchedGems = 0;
    if (TheSongScheduler.mTick / mTicksPerBar < mNumBars) {
        TheSongScheduler.PostIn(mPhraseCommand.Get(), mTicksPerBar * mPhraseBars, false);
    }
}

void DuelTrack::SetRiffData(PitchTrackRiffData *pRiffData) {
    mPitcher.SetRiffData(pRiffData);
}

GemCursor DuelTrack::GetCursor() {
    return mCatchState.GetCursor();
}

bool DuelTrack::IsBarActive([[maybe_unused]] int nBar) {
    return true;
}
