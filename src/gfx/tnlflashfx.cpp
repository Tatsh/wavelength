#include "gfx/tnlflashfx.h"

#include "math/color.h"

TnlFlashFX::TnlFlashFX(Rnd::ParticleSys *pSys) : mSys(pSys), mParticle(nullptr), mShrink(0.0f) {
}

TnlFlashFX::~TnlFlashFX() {
    if (mParticle != nullptr) {
        mSys->FreeParticle(mParticle);
    }
}

Rnd::Particle *TnlFlashFX::Start(const Vector3 *pPos, float flSize, float flShrink) {
    if (mParticle != nullptr) {
        return nullptr;
    }
    mParticle = mSys->AllocParticle();
    mParticle->mCol = Color{1.0f, 1.0f, 1.0f, 1.0f};
    mParticle->mSize = flSize;
    mParticle->mPos = *pPos;
    mShrink = flShrink;
    return mParticle;
}

void TnlFlashFX::Stop() {
    if (mParticle != nullptr) {
        mSys->FreeParticle(mParticle);
        mParticle = nullptr;
    }
}

void TnlFlashFX::Poll(float flDelta) {
    if (mParticle == nullptr) {
        return;
    }
    mParticle->mSize -= flDelta * mShrink;
    if (mParticle->mSize <= 0.0f) {
        mSys->FreeParticle(mParticle);
        mParticle = nullptr;
    }
}
