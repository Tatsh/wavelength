#include "gfx/gfxtunnelpanel.h"

#include "game/gamedb.h"
#include "gfx/tnlgeom.h"

namespace {

// The ticks of one bar.
constexpr float kTicksPerBar = 1920.0f;

// The tick of a reveal that has not been scheduled.
constexpr float kNeverTick = 1e9f;

// The ticks a reveal sweeps the panel along its bar.
constexpr int kRevealTicks = 480;

// Added to the elapsed ticks of a reveal before they are truncated.
constexpr float kRounding = 0.499f;

// The section a finished reveal starts the panel at.
constexpr char kFirstSection = 0;

// In a remix, the bars of this track set the checkpoint materials of the bars of the ring.
constexpr int kCheckpointTrack = 0;

} // namespace

GfxTunnelPanel::GfxTunnelPanel(GfxTunnel *pTunnel,
                               int nTrack,
                               const GfxTunnel::BarData *pBar,
                               const Color *pColor) {
    mTunnel = pTunnel;
    mTrack = nTrack;
    mTick = pBar->mTick;
    mOwner = pBar->mOwner;
    mFlags = pBar->mFlags;
    mDeferred = 0;
    mBar = static_cast<int>(pBar->mTick / kTicksPerBar);
    mCheckpointMat = nullptr;
    mTransparent = 0;
    mPickable = 0;
    mRevealTicks = 0;
    mReserved38 = 0;
    mInSong = pBar->mInSong;
    mColor = pColor;
    mRevealTick = kNeverTick;
    ResolveState();
}

void GfxTunnelPanel::SetDeferred(int bDeferred, float flRevealTick) {
    mDeferred = bDeferred;
    mRevealTick = flRevealTick;
}

void GfxTunnelPanel::ResolveState() {
    if ((mFlags & GfxTunnel::kBarFlagFinish) != 0 && TheGameDb->mRuleSet != GameDb::kRuleSetRemix) {
        mTransparent = 1;
        mPickable = 0;
        mCheckpointMat = mTunnel->mCheckpointFinishMat;
        return;
    }
    if ((mFlags & GfxTunnel::kBarFlagCheckpoint) != 0 &&
        TheGameDb->mRuleSet != GameDb::kRuleSetRemix) {
        mTransparent = 1;
        mPickable = 0;
        mCheckpointMat = mTunnel->mCheckpointMat;
        return;
    }
    if (mOwner >= 0 || mInSong == 0) {
        mPickable = 0;
        mTransparent = 1;
        mCheckpointMat = nullptr;
    } else {
        mCheckpointMat = nullptr;
        mPickable = 1;
        mTransparent = 0;
    }
}

void GfxTunnelPanel::Apply() {
    TnlGeom *pGeom = mTunnel->mGeom;
    pGeom->SetPanelState(
        mTrack, mBar, static_cast<char>(mTransparent), mCheckpointMat, mPickable, true, mColor);
    if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix && mTrack == kCheckpointTrack) {
        Rnd::Mat *pMat = nullptr;
        if ((mFlags & GfxTunnel::kBarFlagCheckpoint) != 0) {
            pMat = mTunnel->mCheckpointMat;
        }
        pGeom->SetBarValue(mBar, pMat);
    }
}

void GfxTunnelPanel::Refresh() {
    if (mRevealTicks != 0) {
        mTunnel->mGeom->SetPanelStartSection(mTrack, mBar, kFirstSection);
        Apply();
    }
}

bool GfxTunnelPanel::Update(GfxTunnel *pTunnel, float flTick) {
    if (mDeferred != 0) {
        if (pTunnel->IsTrackCaptured(static_cast<char>(mTrack))) {
            return true;
        }
        Apply();
        return false;
    }
    if (mRevealTick <= flTick && mRevealTicks == 0) {
        if ((mFlags & GfxTunnel::kBarFlagNoReveal) != 0) {
            return false;
        }
        mReserved38 = 0;
        mRevealTicks = kRevealTicks;
    }
    if (mRevealTicks == 0) {
        return true;
    }
    const int nElapsed = static_cast<int>((flTick - mRevealTick) + kRounding);
    TnlGeom *pGeom = mTunnel->mGeom;
    if (mRevealTicks < nElapsed) {
        pGeom->SetPanelStartSection(mTrack, mBar, kFirstSection);
        Apply();
        return false;
    }
    if (mOwner < 0) {
        return true;
    }
    const int nSection =
        (((pGeom->mSectionsPerBar - 1) * nElapsed) + mRevealTicks - 1) / mRevealTicks;
    pGeom->SetPanelStartSection(mTrack, mBar, static_cast<char>(nSection));
    return true;
}
