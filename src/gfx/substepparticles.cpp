#include "gfx/substepparticles.h"

namespace {

// The time of a system that has not been updated.
constexpr float kNever = -1e9f;

// The components of a position.
constexpr int kNumComponents = 3;

// The rows of a transform.
constexpr int kNumBasisRows = 3;
constexpr int kTranslationRow = 3;

} // namespace

SubstepParticles::SubstepParticles(int nSteps, Rnd::ParticleSys *pSys) {
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
    float (&local)[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount] = mSys->mLocalXfm;
    const float (&world)[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount] = mSys->mWorldXfm;
    if (mLastTime == kNever) {
        mLastPos = Vector3{world[kTranslationRow][0],
                           world[kTranslationRow][1],
                           world[kTranslationRow][2],
                           world[kTranslationRow][kVec3PaddingFloat]};
        mLastTime = fTime;
        return;
    }
    const float fStep = (fTime - mLastTime) * mInvSteps;
    float saved[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
    for (int i = 0; i < Rnd::kXfmRowCount; ++i) {
        for (int j = 0; j < Rnd::kXfmRowFloatCount; ++j) {
            saved[i][j] = local[i][j];
        }
    }
    float pos[Rnd::kXfmRowFloatCount];
    for (int j = 0; j < Rnd::kXfmRowFloatCount; ++j) {
        pos[j] = world[kTranslationRow][j];
    }
    float delta[] = {mLastPos.x - pos[0], mLastPos.y - pos[1], mLastPos.z - pos[2]};
    mLastPos = Vector3{pos[0], pos[1], pos[2], pos[kVec3PaddingFloat]};
    if (fStep != 0.0f) {
        for (float &component : delta) {
            component *= mInvSteps;
        }
        for (int i = 0; i < kNumBasisRows; ++i) {
            for (int j = 0; j < Rnd::kXfmRowFloatCount; ++j) {
                local[i][j] = world[i][j];
            }
        }
        mSys->mDirty = 1;
        // Yes, the binary steps from the current position back toward the last one.
        for (int n = mSteps; n != 0; --n) {
            mLastTime += fStep;
            for (int j = 0; j < Rnd::kXfmRowFloatCount; ++j) {
                local[kTranslationRow][j] = pos[j];
            }
            mSys->mDirty = 1;
            mSys->UpdateWorldXfm(nullptr, 0);
            mSys->SetFrameSelf(mLastTime);
            for (int i = 0; i < kNumComponents; ++i) {
                pos[i] += delta[i];
            }
        }
        for (int i = 0; i < Rnd::kXfmRowCount; ++i) {
            for (int j = 0; j < Rnd::kXfmRowFloatCount; ++j) {
                local[i][j] = saved[i][j];
            }
        }
        mSys->mDirty = 1;
    }
    mLastTime = fTime;
}
