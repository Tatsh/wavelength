#include "game/freestyletrack.h"

#include <algorithm>

#include "game/controllerdisplay.h"
#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "gfx/gfxmanager.h"
#include "math/vector3.h"
#include "msg/freestylepacket.h"
#include "netflow/nettransport.h"
#include "os/memfun1command.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"
#include "os/system.h"

namespace {

// The points of a bar are the configured freestyle points scaled by this factor of the part of
// the bar an active note sounded, and capped at the configured points.
constexpr float kPointsNumerator = 8.0f;
constexpr float kPointsDenominator = 5.0f;

// UpdatePoints() runs this many times a bar.
constexpr int kPointsUpdatesPerBar = 20;

// The shortest time between two updates SendUpdate() sends.
constexpr float kSendIntervalMs = 150.0f;

constexpr int kNoEnd = -1;

} // namespace

FreestyleTrack::FreestyleTrack(const SectionBoundaries *pSections,
                               PlayMap *pPlayMap,
                               const float *pfMsPerTick,
                               int nIndex,
                               int nNumBars,
                               int nTicksPerBar)
    : Track(nIndex), mTickOffset(0), mSections(pSections), mPlayMap(pPlayMap),
      mMsPerTick(pfMsPerTick), mNumBars(nNumBars), mTicksPerBar(nTicksPerBar), mButton(0),
      mLastSendMs(0.0f), mBarCommand(NewMemFun1Command(this, &FreestyleTrack::StartBar, true)),
      mPointsCommand(NewMemFunCommand(this, &FreestyleTrack::UpdatePoints)) {
}

FreestyleTrack::~FreestyleTrack() {
    FreestyleTrack::Stop();
}

void FreestyleTrack::Stop() {
}

void FreestyleTrack::SetPlayer(Player *pPlayer) {
    TheControllerDisplay->SetOption(pPlayer != nullptr);
    mNoteTimer.Reset();
    mNoteTimer.Start();
    mNoteTimer.Pause();
    if (mPlayer != nullptr) {
        StartBar(false);
        TheSongScheduler.Cancel(mBarCommand.Get());
    }
    Track::SetPlayer(pPlayer);
    if (pPlayer != nullptr) {
        pPlayer->LosePendingPoints();
        const Vector3 position{0.0f, 0.0f, 0.0f};
        TheGfxManager.SetFreestylePosition(pPlayer->GetIndex(), position);
        StartBar(true);
    }
}

void FreestyleTrack::HandleInput(Player *pPlayer, const PlayNoteEvent &event) {
    if (pPlayer != mPlayer) {
        return;
    }

    mButton = event.mButton;
    SendUpdate(pPlayer, event.mX, event.mY);
    if (IsActive()) {
        mNoteTimer.Resume();
        UpdatePoints();
    } else {
        mNoteTimer.Pause();
        TheSongScheduler.Cancel(mPointsCommand.Get());
    }
}

void FreestyleTrack::HandleInput(Player *pPlayer, const StickEvent<2> &event) {
    if (pPlayer != mPlayer) {
        return;
    }

    const float fX = event.mX;
    const float fY = event.mY;
    const Vector3 position{fX, 0.0f, fY};
    TheGfxManager.SetFreestylePosition(pPlayer->GetIndex(), position);
    SendUpdate(pPlayer, fX, fY);
}

void FreestyleTrack::HandleInput(Player *pPlayer, const StickEvent<6> &event) {
    if (pPlayer != mPlayer) {
        return;
    }

    const Vector3 position{event.mX, 0.0f, event.mY};
    TheGfxManager.SetFreestyleSecondPosition(pPlayer->GetIndex(), position);
}

void FreestyleTrack::UpdatePoints() {
    if (TheGameDb->mRuleSet != GameDb::kRuleSetGame || !IsActive()) {
        return;
    }
    Player *pPlayer = mPlayer;
    if (!TheGameDb->IsLocalPlayer(pPlayer->GetIndex())) {
        return;
    }
    if (TheSongScheduler.mTick / mTicksPerBar >= mNumBars) {
        return;
    }

    if (!TheGameDb->IsWinSequence()) {
        mNoteTimer.Pause();
        const float fSoundedMs = static_cast<float>(mNoteTimer.mCycles) * gSystemCycles2Ms;
        mNoteTimer.Resume();
        const float fBarMs = static_cast<float>(mTicksPerBar) * *mMsPerTick;
        const float fSounded = fSoundedMs / fBarMs;
        const int nMaxPoints = TheGameConfig->mFreestylePoints[TheGameDb->mSkillLevel];
        const int nPoints = static_cast<int>(fSounded * static_cast<float>(nMaxPoints) *
                                             kPointsNumerator / kPointsDenominator);
        pPlayer->SetPendingPoints(std::min(nPoints, nMaxPoints), false);
    }
    TheSongScheduler.PostIn(mPointsCommand.Get(), mTicksPerBar / kPointsUpdatesPerBar, false);
}

void FreestyleTrack::StartBar(bool bSchedule) {
    if (TheGameDb->mRuleSet != GameDb::kRuleSetGame) {
        return;
    }
    Player *pPlayer = mPlayer;
    if (!TheGameDb->IsLocalPlayer(pPlayer->GetIndex())) {
        return;
    }

    const int nBar = (TheSongScheduler.mTick + 1) / mTicksPerBar;
    if (nBar <= mNumBars && !TheGameDb->IsWinSequence()) {
        pPlayer->CommitPendingPoints(false);
    }
    mNoteTimer.Reset();
    mNoteTimer.Start();
    if (!IsActive()) {
        mNoteTimer.Pause();
    }
    if (bSchedule) {
        TheSongScheduler.PostAt(mBarCommand.Get(), ((nBar + 1) * mTicksPerBar) - 1, false);
    }
}

void FreestyleTrack::SendUpdate(Player *pPlayer, float fX, float fY) {
    if (TheGameDb->mCommunity != GameDb::kCommunityOnline) {
        return;
    }
    if (!TheGameDb->IsLocalPlayer(pPlayer->GetIndex())) {
        return;
    }

    const float fNowMs = SystemMs();
    if (fNowMs - mLastSendMs > kSendIntervalMs) {
        FreestylePacket packet(IsActive(), mButton, fX, fY);
        TheNetTransport->Send(packet);
        mLastSendMs = fNowMs;
    }
}

int FreestyleTrack::WrapTick(int nTick) {
    if (mPlayMap->mLooping) {
        return mPlayMap->MapTick(nTick);
    }
    return nTick % (mNumBars * mTicksPerBar);
}

int FreestyleTrack::SpanEnd(int nTick, int nStart, int nEnd) {
    int nSpanEnd;
    if ((nEnd == kNoEnd) || (nEnd < nStart)) {
        const int nPeriod = mNumBars * mTicksPerBar;
        nSpanEnd = (((nTick / nPeriod) * mNumBars) + mNumBars) * mTicksPerBar;
    } else {
        nSpanEnd = nTick + (nEnd - nStart);
    }

    if (mPlayMap->mLooping) {
        int nChangeTick;
        int nEndTick;
        int nNextStart;
        int nNextLength;
        int nIsLast;
        mPlayMap->GetSegment(nTick, &nChangeTick, &nEndTick, &nNextStart, &nNextLength, &nIsLast);
        if (nChangeTick < nSpanEnd) {
            nSpanEnd = nChangeTick;
        }
    }
    return nSpanEnd;
}
