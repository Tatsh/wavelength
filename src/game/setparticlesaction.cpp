#include "game/setparticlesaction.h"

#include "game/triggermgr.h"

namespace {

// Interpolate a range from where it was towards where the action moves it.
inline float Lerp(float flFrom, float flTo, float flT) {
    return ((flTo - flFrom) * flT) + flFrom;
}

// Interpolate a corner, which arrives exactly at either end.
inline Vector3 LerpCorner(const Vector3 &from, const Vector3 &to, float flT) {
    if (flT == 0.0f) {
        return from;
    }
    if (flT == 1.0f) {
        return to;
    }
    const float flFrom = 1.0f - flT;
    Vector3 corner = to;
    corner.x = (to.x * flT) + (from.x * flFrom);
    corner.y = (to.y * flT) + (from.y * flFrom);
    corner.z = (to.z * flT) + (from.z * flFrom);
    return corner;
}

void ReadCorner(DataArray *pCorner, Vector3 &corner) {
    const float flX = pCorner->Float(0);
    const float flY = pCorner->Float(1);
    corner.z = pCorner->Float(2);
    corner.x = flX;
    corner.y = flY;
}

} // namespace

SetParticlesAction::SetParticlesAction(DataArray *pAction) {
    mParticleSys = FindObject<Rnd::ParticleSys>(pAction, pAction->Sym(1));
    mDuration = pAction->Float(2);
    mFlags = 0;
    if (pAction->FindInt("max_particles", &mMaxParticles, false)) {
        mFlags |= kFlagMaxParticles;
    }
    if (pAction->FindVector("life", &mLife, false)) {
        mFlags |= kFlagLife;
    }
    if (pAction->FindVector("speed", &mSpeed, false)) {
        mFlags |= kFlagSpeed;
    }
    if (pAction->FindVector("size", &mSize, false)) {
        mFlags |= kFlagSize;
    }
    if (pAction->FindVector("emit_rate", &mEmitRate, false)) {
        mFlags |= kFlagEmitRate;
    }
    DataArray *pPos = pAction->FindArray("pos", false);
    if (pPos != nullptr) {
        ReadCorner(pPos->Array(1), mPos[kCornerLow]);
        ReadCorner(pPos->Array(2), mPos[kCornerHigh]);
        mFlags |= kFlagPos;
    }
    const char *pszMat;
    if (pAction->FindSymbol("mat", &pszMat, false)) {
        mFlags |= kFlagMat;
        mMat = FindObject<Rnd::Mat>(pAction, pszMat);
    }
}

void SetParticlesAction::Exec() {
    mStartTime = TheTriggerMgr.mTime[TheTriggerMgr.mClock];
    mStartLife = mParticleSys->mLife;
    mStartSpeed = mParticleSys->mSpeed;
    mStartSize.x = mParticleSys->mSizeLow;
    mStartSize.y = mParticleSys->mSizeHigh;
    mStartEmitRate.x = mParticleSys->mEmitRateLow;
    mStartEmitRate.y = mParticleSys->mEmitRateHigh;
    mStartPos[kCornerLow] = mParticleSys->mPosLow;
    mStartPos[kCornerHigh] = mParticleSys->mPosHigh;
    if ((mFlags & kFlagMaxParticles) != 0) {
        mParticleSys->SetPoolSize(mMaxParticles);
    }
    if ((mFlags & kFlagMat) != 0) {
        mParticleSys->SetMat(mMat);
    }
    TheTriggerMgr.AddRunning(this);
}

bool SetParticlesAction::Poll() {
    float flT = 1.0f;
    if (mDuration != 0.0f) {
        flT = (TheTriggerMgr.mTime[TheTriggerMgr.mClock] - mStartTime) / mDuration;
        if (1.0f < flT) {
            flT = 1.0f;
        }
    }
    if ((mFlags & kFlagLife) != 0) {
        mParticleSys->mLife.x = Lerp(mStartLife.x, mLife.x, flT);
        mParticleSys->mLife.y = Lerp(mStartLife.y, mLife.y, flT);
    }
    if ((mFlags & kFlagSpeed) != 0) {
        mParticleSys->mSpeed.x = Lerp(mStartSpeed.x, mSpeed.x, flT);
        mParticleSys->mSpeed.y = Lerp(mStartSpeed.y, mSpeed.y, flT);
    }
    if ((mFlags & kFlagSize) != 0) {
        mParticleSys->mSizeLow = Lerp(mStartSize.x, mSize.x, flT);
        mParticleSys->mSizeHigh = Lerp(mStartSize.y, mSize.y, flT);
    }
    if ((mFlags & kFlagEmitRate) != 0) {
        mParticleSys->mEmitRateLow = Lerp(mStartEmitRate.x, mEmitRate.x, flT);
        mParticleSys->mEmitRateHigh = Lerp(mStartEmitRate.y, mEmitRate.y, flT);
    }
    if ((mFlags & kFlagPos) != 0) {
        const Vector3 low = LerpCorner(mStartPos[kCornerLow], mPos[kCornerLow], flT);
        const Vector3 high = LerpCorner(mStartPos[kCornerHigh], mPos[kCornerHigh], flT);
        mParticleSys->mPosLow = low;
        mParticleSys->mPosHigh = high;
    }
    return flT == 1.0f;
}
