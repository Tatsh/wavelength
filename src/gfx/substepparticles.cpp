#include "gfx/substepparticles.h"

namespace {

// The time of a system that has not been updated.
constexpr float kNever = -1e9f;

} // namespace

SubstepParticles::SubstepParticles(int nSteps, RndParticleSys *pSys) {
    mSys = pSys;
    mLastTime = kNever;
    SetSteps(nSteps);
}

void SubstepParticles::Reset() {
    mLastTime = kNever;
}

void SubstepParticles::SetSteps(int nSteps) {
    mSteps = nSteps;
    mInvSteps = 1.0f / static_cast<float>(nSteps);
}

void SubstepParticles::Update(float fFrame, float fTime) {
    mSys->SetFrame(fFrame);
    Transform &local = mSys->mLocalXfm;
    const Transform &world = mSys->mWorldXfm;
    if (mLastTime == kNever) {
        mLastPos = world.mTranslation;
        mLastTime = fTime;
        return;
    }
    const float fStep = (fTime - mLastTime) * mInvSteps;
    const Transform saved = local;
    Vector3 pos = world.mTranslation;
    float delta[] = {mLastPos.x - pos.x, mLastPos.y - pos.y, mLastPos.z - pos.z};
    mLastPos = pos;
    if (fStep != 0.0f) {
        for (float &component : delta) {
            component *= mInvSteps;
        }
        local.mBasisX = world.mBasisX;
        local.mBasisY = world.mBasisY;
        local.mBasisZ = world.mBasisZ;
        mSys->mDirty = 1;
        // Yes, the binary steps from the current position back toward the last one.
        for (int n = mSteps; n != 0; --n) {
            mLastTime += fStep;
            local.mTranslation = pos;
            mSys->mDirty = 1;
            mSys->UpdateWorldXfm(nullptr, 0);
            mSys->SetEmitFrame(mLastTime);
            pos.x += delta[0];
            pos.y += delta[1];
            pos.z += delta[2];
        }
        local = saved;
        mSys->mDirty = 1;
    }
    mLastTime = fTime;
}
