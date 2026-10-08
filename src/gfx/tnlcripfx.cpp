#include "gfx/tnlcripfx.h"

#include <cmath>
#include <vector>

#include "game/gamedb.h"
#include "gfx/gfxmanager.h"
#include "gfx/gfxutil.h"
#include "gfx/playercamfx.h"
#include "gfx/tnlgeom.h"
#include "math/sine.h"
#include "rnd/manager.h"

namespace {

constexpr float kHalfPi = 1.57079637f;

// The ticks of one bar of the panels the wave bends.
constexpr float kBarTicks = 1920.0f;

// The tracks of a crippler with no wave.
constexpr char kNoTrack = -1;

// The rotation of the twist of a section about its Y axis, three rows of a basis.
struct Twist {
    Vector3 mRows[3];
};

// Apply three rows of a twist to the rows of a basis.
inline void TwistBasis(const Vector3 *pBase, const Twist &twist, Vector3 *pOut) {
    Vector3 rows[3];
    for (int i = 0; i < 3; ++i) {
        const Vector3 &axis = twist.mRows[i];
        rows[i] = Vector3{(pBase[0].x * axis.x) + (pBase[1].x * axis.y) + (pBase[2].x * axis.z),
                          (pBase[0].y * axis.x) + (pBase[1].y * axis.y) + (pBase[2].y * axis.z),
                          (pBase[0].z * axis.x) + (pBase[1].z * axis.y) + (pBase[2].z * axis.z),
                          axis.w};
    }
    for (int i = 0; i < 3; ++i) {
        pOut[i] = rows[i];
    }
}

// The last bar of the panels a wave bends, from which it counts down to the first.
inline int LastWaveBar(const TnlGeom *pGeom) {
    return (pGeom->mFirstBar + (pGeom->mNumBars - pGeom->mBarsBehind)) - 1;
}

} // namespace

float TnlCripFX::sFlyTime = 1000.0f;
float TnlCripFX::sFadeTime = 200.0f;
float TnlCripFX::sWaveTime = 1000.0f;
float TnlCripFX::sSpatialFreqs[2] = {0.00654498488f, 0.00654498488f};
float TnlCripFX::sTimeFreqs[2] = {0.0125663709f, 0.0125663709f};
float TnlCripFX::sWaveDeathLength = 100.0f;
float TnlCripFX::sWaveMagnitudes[2] = {0.25f, 0.0f};
float TnlCripFX::sTickVelocity = 1.0f;
float TnlCripFX::sStartHeight = 0.0f;
float TnlCripFX::sStartHeightVelocity = 9.99999975e-05f;
float TnlCripFX::sHeightAcceleration = -9.99999975e-05f;
float TnlCripFX::sImpactCamJiggle = 0.1f;
float TnlCripFX::sMaxTwist = 0.174532935f;
float TnlCripFX::sTwistFreq = 0.00654498488f;
Rnd::ParticleSys *TnlCripFX::sParticles = nullptr;
float TnlCripFX::sParticleFrame = 0.0f;

TnlCripFX::TnlCripFX(int nIndex) {
    mState = kStateIdle;
    mView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find("crippler.view"));
    mShadow = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find("crippler shadow.view"));
    if (nIndex != 0) {
        return;
    }
    sParticles = dynamic_cast<Rnd::ParticleSys *>(Rnd::TheManager.Find("crippler.part"));
    ScaleParticles(sParticles, TheGfxManager.mScrollSpeed);
    sParticleFrame = 0.0f;
    sParticles->Rnd::ParticleSys::SetFrameSelf(sParticleFrame);
    sParticles->FreeAllParticles();
    mView->RemoveAnim(sParticles);
}

TnlCripFX::~TnlCripFX() {
    sParticles = nullptr;
}

bool TnlCripFX::Poll(float flDelta, GfxTunnel *pTunnel) {
    if (mState == kStateFly) {
        mDelta = flDelta;
        const float flRise = sHeightAcceleration * flDelta;
        const float flClimb = mHeightVelocity * flDelta;
        mFlyTick = ((TheGameDb->mSongTime - mStartTime) * sTickVelocity) + TheGameDb->mSongTick;
        mHeightVelocity += flRise;
        mHeight += flClimb;
        TnlGeom *pGeom = pTunnel->mGeom;
        pGeom->BlendCell(&mXfm, true, false, pGeom->GetPlayer(mVictim)->mPosition, mFlyTick, 0.5f);
        const float flScale = TheGfxManager.mScrollSpeed;
        Vector3 *rows[] = {&mXfm.mBasisX, &mXfm.mBasisY, &mXfm.mBasisZ};
        for (Vector3 *pRow : rows) {
            pRow->x *= flScale;
            pRow->y *= flScale;
            pRow->z *= flScale;
        }
        mShadowPos = mXfm.mTranslation;
        mXfm.mTranslation.x = mShadowPos.x + (mXfm.mBasisZ.x * mHeight);
        mXfm.mTranslation.z = mShadowPos.z + (mXfm.mBasisZ.z * mHeight);
        mXfm.mTranslation.y = mShadowPos.y + (mXfm.mBasisZ.y * mHeight);
        return false;
    }
    if (mState != kStateWave) {
        return false;
    }

    const float flElapsed = TheGameDb->mSongTime - mStartTime;
    const float flPhaseA = flElapsed * sTimeFreqs[0];
    const float flPhaseB = flElapsed * sTimeFreqs[1];
    const float flReach = flPhaseA / sSpatialFreqs[0];
    const float flLife = 1.0f - (flElapsed / sWaveTime);
    mArrived = (mWaveTick - flReach) <= TheGameDb->mSongTick;
    if (flLife <= 0.0f) {
        Stop(pTunnel, false);
        return true;
    }

    TnlGeom *pGeom = pTunnel->mGeom;
    const bool bIdle = pTunnel->IsActivatorIdle(mVictim);
    const char nTrack = pGeom->GetPlayer(mVictim)->mTrack;
    mPrevWaveTrack = (nTrack != mWaveTrack) ? mWaveTrack : kNoTrack;
    for (int nBar = LastWaveBar(pGeom); nBar >= pGeom->mFirstBar; --nBar) {
        if (!bIdle) {
            pGeom->RestoreSections(mWaveTrack, nBar);
            continue;
        }
        if (nTrack != mWaveTrack) {
            pGeom->RestoreSections(mWaveTrack, nBar);
        }
        std::vector<TnlGeom::CrossSectionXfm> *pSections = pGeom->GetSections(nTrack, nBar, true);
        TnlGeom::PanelData *pPanel = pGeom->GetPanel(nTrack, nBar);
        if (pSections == nullptr) {
            continue;
        }
        const float flSectionTicks =
            kBarTicks / static_cast<float>(static_cast<unsigned int>(pSections->size() - 1));
        float flSectionTick = static_cast<float>(nBar) * kBarTicks;
        const TnlGeom::CrossSectionXfm *pBase = pPanel->mBaseSections.data();
        for (TnlGeom::CrossSectionXfm &section : *pSections) {
            const float flDistance = std::fabs(mWaveTick - flSectionTick);
            if (!(0.0f < (flDistance - flReach))) {
                const float flWaveA =
                    FastSin((flPhaseA - ((sSpatialFreqs[0] * flDistance) + kHalfPi)) + kHalfPi);
                const float flWaveB =
                    FastSin((flPhaseB - ((sSpatialFreqs[1] * flDistance) + kHalfPi)) + kHalfPi);
                const float flLift =
                    flLife * ((sWaveMagnitudes[0] * flWaveA) + (sWaveMagnitudes[1] * flWaveB));
                const Vector3 &axisZ = pBase->mXfm.mBasisZ;
                const float flTwistWave = FastSin(flSectionTick * sTwistFreq);
                const float flAngle = (flLife * sMaxTwist) * flTwistWave;
                const float flCos = SinApprox(flAngle + kHalfPi);
                const float flSin = SinApprox(flAngle);
                const Twist twist{{Vector3{flCos, 0.0f, -flSin},
                                   Vector3{0.0f, 1.0f, 0.0f},
                                   Vector3{flSin, 0.0f, flCos}}};
                section.mXfm.mTranslation.x = pBase->mXfm.mTranslation.x + (axisZ.x * flLift);
                section.mXfm.mTranslation.y = pBase->mXfm.mTranslation.y + (axisZ.y * flLift);
                section.mXfm.mTranslation.z = pBase->mXfm.mTranslation.z + (axisZ.z * flLift);
                TwistBasis(&pBase->mXfm.mBasisX, twist, &section.mXfm.mBasisX);
                TwistBasis(pBase->mCellBasis, twist, section.mCellBasis);
            }
            ++pBase;
            flSectionTick += flSectionTicks;
        }
    }
    mWaveTrack = nTrack;
    return true;
}

void TnlCripFX::Draw(GfxTunnel *pTunnel) {
    if (mState == kStateWave) {
        const float flElapsed = TheGameDb->mSongTime - mStartTime;
        if (flElapsed <= sFadeTime) {
            mView->SetFrame(flElapsed + sFlyTime);
            mView->SetLocalXfm(mXfm);
            mView->UpdateWorldXfm(nullptr, 0);
            mView->Draw();
        }
        return;
    }
    if (mState != kStateFly) {
        return;
    }
    const float flElapsed = TheGameDb->mSongTime - mStartTime;
    Transform shadow = mXfm;
    shadow.mTranslation = mShadowPos;
    mShadow->SetLocalXfm(shadow);
    mShadow->UpdateWorldXfm(nullptr, 0);
    mShadow->Draw();
    mView->SetFrame(flElapsed);
    mView->SetLocalXfm(mXfm);
    mView->UpdateWorldXfm(nullptr, 0);
    mView->Draw();
    sParticleFrame += mDelta;
    sParticles->Rnd::ParticleSys::SetFrameSelf(sParticleFrame);
    if (!(sFlyTime <= flElapsed)) {
        return;
    }
    const float flTick = mFlyTick;
    mState = kStateWave;
    mWaveTick = flTick;
    mStartTime += sFlyTime;
    const char nTrack = pTunnel->mGeom->GetPlayer(mVictim)->mTrack;
    mArrived = 0;
    mPrevWaveTrack = kNoTrack;
    mWaveTrack = nTrack;
    pTunnel->mCamFX->Kick(sImpactCamJiggle);
}

void TnlCripFX::DrawParticles() {
    sParticles->SetFrame(TheGameDb->mSongTime);
    sParticles->Draw();
}

void TnlCripFX::Start(GfxTunnel *pTunnel, char, char nVictim) {
    if (mState != kStateIdle) {
        Stop(pTunnel, true);
    }
    mVictim = nVictim;
    mState = kStateFly;
    mFlyTick = TheGameDb->mSongTick;
    mHeight = sStartHeight;
    mHeightVelocity = sStartHeightVelocity;
    mStartTime = TheGameDb->mSongTime;
    mDelta = 0.0f;
}

void TnlCripFX::Stop(GfxTunnel *pTunnel, bool bRestore) {
    if ((mState == kStateWave) && bRestore && (pTunnel != nullptr)) {
        TnlGeom *pGeom = pTunnel->mGeom;
        for (int nBar = LastWaveBar(pGeom); nBar >= pGeom->mFirstBar; --nBar) {
            pGeom->RestoreSections(mWaveTrack, nBar);
        }
    }
    mState = kStateIdle;
}

char TnlCripFX::ArrivedWaveTrack() const {
    if ((mState == kStateWave) && (mArrived != 0)) {
        return mWaveTrack;
    }
    return kNoTrack;
}

void TnlCripFX::GetWaveTracks(char *pnTrack, char *pnPrevTrack) const {
    if (mState == kStateWave) {
        *pnTrack = mWaveTrack;
        *pnPrevTrack = mPrevWaveTrack;
        return;
    }
    *pnPrevTrack = kNoTrack;
    *pnTrack = kNoTrack;
}
