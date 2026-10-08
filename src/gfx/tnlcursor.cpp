#include "gfx/tnlcursor.h"

#include <cmath>

#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"

namespace {

constexpr float kNoTick = -1e9f;
constexpr char kNoTrack = -1;
constexpr float kOpaque = 1.0f;
constexpr float kTransparent = 0.0f;
constexpr float kMillisecondsPerSecond = 1000.0f;

} // namespace

TnlCursor::TnlCursor(GfxTunnel *pTunnel,
                     int nPlayer,
                     const char *pszColor,
                     char nExcludedType,
                     char nOtherExcludedType)
    : mPlayer(nPlayer), mTunnel(pTunnel), mFade(0.0f), mState(kStateIdle), mTrack(kNoTrack),
      mStartTick(kNoTick), mEndTick(kNoTick), mGems(nullptr), mTargetStartTick(0.0f),
      mTargetEndTick(0.0f), mTargetTrack(kNoTrack),
      mBeam(new TnlConnectorBeam(pTunnel, nPlayer, pszColor, nExcludedType, nOtherExcludedType)),
      mSeeker(new TnlSeeker(pTunnel->mGeom, nPlayer, pszColor)) {
}

TnlCursor::~TnlCursor() {
    delete mSeeker;
    delete mBeam;
}

void TnlCursor::LoadConfig(DataArray *pConfig,
                           DataArray *pDefaults,
                           GfxTunnel *pTunnel,
                           int nOption) {
    mBeam->LoadConfig(pConfig, pDefaults, pTunnel);
    mSeeker->LoadConfig(pConfig, pDefaults, pTunnel, nOption);
    FindConfigFloat(pConfig, pDefaults, "seeker_fade_speed", &TnlSeeker::sFadeSpeed, true);
    TnlSeeker::sFadeSpeed /= kMillisecondsPerSecond;
    mColor = *TheGfxManager.GetPlayerColor(mPlayer);
}

void TnlCursor::Reset() {
    mBeam->Reset();
    mSeeker->StopMultiplier();
    mTargetTrack = kNoTrack;
    mTargetStartTick = 0.0f;
    mTargetEndTick = 0.0f;
}

void TnlCursor::Refresh() {
    SetTarget(mTrack, nullptr, mStartTick, mStartTick);
}

void TnlCursor::SetTarget(char nTrack, TnlGems *pGems, float flStartTick, float flEndTick) {
    if (mState == kStateIdle && mFade == kTransparent) {
        mBeam->SetWindow(nTrack, pGems, flStartTick, flEndTick);
        mSeeker->SetWindow(nTrack, flStartTick, flEndTick);
        mGems = nullptr;
        mState = kStateFadingIn;
    } else {
        mSeeker->SetTarget(nTrack, flStartTick, flEndTick);
        mGems = pGems;
        mState = kStateFadingOut;
        mTrack = nTrack;
        mStartTick = flStartTick;
        mEndTick = flEndTick;
    }
    mTargetTrack = nTrack;
    mTargetStartTick = flStartTick;
    mTargetEndTick = flEndTick;
}

void TnlCursor::OnRangeChanged(const TnlTrackRange *pRange) {
    mBeam->OnRangeChanged(pRange);
    mSeeker->OnRangeChanged(pRange);
}

void TnlCursor::OnGemAdded(char nTrack, char nType, TnlGems *pGems, float flTick) {
    mBeam->OnGemAdded(nTrack, nType, pGems, flTick);
}

void TnlCursor::OnTicksChanged(char nTrack, TnlGems *pGems, float flStartTick, float flEndTick) {
    mBeam->OnTicksChanged(nTrack, pGems, flStartTick, flEndTick);
}

void TnlCursor::OnGemRemoved(const TnlGem *pGem) {
    mBeam->OnGemRemoved(pGem);
}

void TnlCursor::Pulse(float flTick) {
    mBeam->Pulse(flTick);
}

void TnlCursor::SetEnergy(float flEnergy) {
    mBeam->SetEnergy(flEnergy);
}

void TnlCursor::StartMultiplier(TnlGems *pGems, float flEndTick) {
    mBeam->StartMultiplier(pGems, flEndTick);
    mSeeker->StartMultiplier(flEndTick);
}

void TnlCursor::Poll(bool bCaptured, float, float flDelta) {
    const float flStep = std::fabs(flDelta);
    switch (mState) {
    case kStateFadingIn:
        mFade += flStep * TnlSeeker::sFadeSpeed;
        if (kOpaque <= mFade) {
            mFade = kOpaque;
            mState = kStateIdle;
        }
        break;
    case kStateFadingOut:
        mFade -= flStep * TnlSeeker::sFadeSpeed;
        if (mFade <= kTransparent) {
            mFade = kTransparent;
            mBeam->SetWindow(mTrack, mGems, mStartTick, mEndTick);
            mSeeker->SetWindow(mTrack, mStartTick, mEndTick);
            mState = kStateFadingIn;
            mGems = nullptr;
        }
        break;
    default:
        break;
    }
    mBeam->Poll(bCaptured, flStep, mFade);
    mSeeker->Poll(flStep, mFade);
}
