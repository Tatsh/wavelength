#include "game/dueltrackpitcher.h"

#include "game/gamedb.h"
#include "game/gem.h"
#include "game/player.h"
#include "gfx/gfxmanager.h"
#include "msg/editgempacket.h"
#include "netflow/nettransport.h"
#include "os/scheduler.h"
#include "synth/fxmidi.h"

namespace {

constexpr int kStepTicks = 120;
constexpr int kNoValue = -1;
constexpr int kNoGem = -1;

// The gems in a row on one button at which a press is refused.
constexpr int kMaxRepeats = 4;

// A refused press locks the pitcher for a quarter of a bar.
constexpr int kLockDivisor = 4;

constexpr int kNoPlayer = -1;
constexpr int kPatternStyle = 1;
constexpr int kGemStyle = 0;
constexpr int kGemFlags = 0;
constexpr int kRiffOffset = 0;

} // namespace

DuelTrackPitcher::DuelTrackPitcher(Track *pTrack,
                                   Receiver *pReceiver,
                                   PitchTrackRiffData *pRiffData,
                                   CatchTrackData *pGems,
                                   PlayMap *pPlayMap,
                                   DuelPatternTable *pPatterns,
                                   int nPhraseTicks,
                                   int nCatchSide,
                                   int nTicksPerBar,
                                   int nPhraseBars,
                                   int nWindowTicks,
                                   bool bEasiest)
    : mTrack(pTrack), mReceiver(pReceiver), mRiffData(pRiffData), mGems(pGems), mPlayMap(pPlayMap),
      mPatterns(pPatterns), mPhraseTicks(nPhraseTicks), mCatchSide(nCatchSide),
      mTicksPerBar(nTicksPerBar), mPhraseBars(nPhraseBars), mWindowTicks(nWindowTicks),
      mStepTicks(kStepTicks), mMuse(nullptr), mLastTick(kNoValue), mLastSlot(kNoValue),
      mRepeats(kNoValue), mLockedUntilTick(0), mEasiest(bEasiest ? 1 : 0) {
}

void DuelTrackPitcher::ShowPattern(int nStartBar) {
    const int nStartTick = nStartBar * mTicksPerBar;
    const int nSteps = (mTicksPerBar * mPhraseBars) / mStepTicks;
    const int nTrack = mTrack->mIndex;
    const auto pattern = mPatterns->Find(nStartTick);
    for (int nStep = 0; nStep < nSteps; ++nStep) {
        for (int nSlot = 0; nSlot < kDuelPatternLanes; ++nSlot) {
            if (pattern->mPattern.mAllowed[nStep][nSlot] != 0) {
                TheGfxManager.PlaceGem(nTrack,
                                       nSlot,
                                       kNoPlayer,
                                       static_cast<float>(nStep * mStepTicks + nStartTick),
                                       kPatternStyle,
                                       kGemFlags);
            }
        }
    }
}

void DuelTrackPitcher::Press(int nSlot) {
    const int nStep = (TheSongScheduler.mTick + mStepTicks / 2) / mStepTicks;
    const int nPhraseSteps = (mTicksPerBar * mPhraseBars) / mStepTicks;
    const int nTick = nStep * mStepTicks;
    const int nSongTick = mPlayMap->MapTick(nTick);
    if (nSongTick < 0) {
        return;
    }
    if (nTick < mLockedUntilTick) {
        FxMidi::PlaySound1();
        return;
    }

    const int nPhraseStep = nStep % nPhraseSteps;
    const int nCatchTick = nTick + mPhraseTicks;
    const int nCatchSongTick = mPlayMap->MapTick(nCatchTick);
    const int nExistingGem = mGems->FindGemAt(nCatchSongTick);
    if (mWindowTicks < nPhraseStep * mStepTicks) {
        return;
    }
    const auto pattern = mPatterns->Find(nSongTick);
    if (nExistingGem != kNoGem) {
        return;
    }

    if (pattern->mPattern.mAllowed[nPhraseStep][nSlot] == 0) {
        if (mEasiest) {
            return;
        }
        mReceiver->OnRejected(nTick, nSlot);
        mLockedUntilTick = nTick + mTicksPerBar / kLockDivisor;
        return;
    }

    if (nSlot == mLastSlot && nCatchSongTick == mLastTick + kStepTicks) {
        if (++mRepeats >= kMaxRepeats) {
            FxMidi::PlaySound1();
            return;
        }
    } else {
        mLastSlot = nSlot;
        mRepeats = 1;
    }
    mLastTick = nCatchSongTick;

    const float fTick = static_cast<float>(nTick);
    TheGfxManager.PlaceGem(
        mTrack->mIndex, nSlot, mTrack->mPlayer->GetIndex(), fTick, kGemStyle, kGemFlags);
    TheGfxManager.HitGem(mTrack->mPlayer->GetIndex(), mTrack->mIndex, nSlot, fTick);

    Muse *pCatchRiff = mRiffData->GetRiff(nCatchSongTick, nSlot);
    Muse *pRiff = mRiffData->GetRiff(nSongTick, nSlot);
    if (mMuse != nullptr) {
        mMuse->Stop();
    }
    pRiff->PlayFrom(&TheSongScheduler, kRiffOffset);
    mMuse = pRiff;

    const Gem gem{nSlot, nCatchSongTick, Ptr<Muse>(pCatchRiff)};
    mGems->AddGem(gem);
    TheGfxManager.PlaceGem(mCatchSide,
                           nSlot,
                           mTrack->mPlayer->GetIndex(),
                           static_cast<float>(nCatchTick),
                           kGemStyle,
                           kGemFlags);
    mReceiver->OnPlaced(nTick);

    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        EditGemPacket packet(mCatchSide, nSlot, nCatchSongTick, false);
        TheNetTransport->Send(packet);
    }
}

void DuelTrackPitcher::SetRiffData(PitchTrackRiffData *pRiffData) {
    mRiffData = pRiffData;
}
