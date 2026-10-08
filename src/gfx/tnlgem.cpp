#include "gfx/tnlgem.h"

#include "gfx/tnlgems.h"

namespace {

// The player slots of PlayerColor().
enum PlayerSlot {
    kPlayerSlotGreen = 0,
    kPlayerSlotPurple = 1,
    kPlayerSlotRed = 2,
    kPlayerSlotYellow = 3,
};

constexpr float kPurpleRed = 0.65f;

} // namespace

float TnlGem::sParticleHeight = 0.0f;
float TnlGem::sSpriteHeight = 0.9f;
float TnlGem::sFlareFlashSize = 1.0f;
float TnlGem::sFlareFlashRate = 0.006f;
float TnlGem::sMeshFlashSize = 1.0f;
float TnlGem::sMeshFlashRate = 0.006f;

Color TnlGem::PlayerColor(char nColor) {
    switch (nColor) {
    case kPlayerSlotGreen:
        return Color{0.0f, 1.0f, 0.0f, 1.0f};
    case kPlayerSlotPurple:
        return Color{kPurpleRed, 0.0f, 1.0f, 1.0f};
    case kPlayerSlotRed:
        return Color{1.0f, 0.0f, 0.0f, 1.0f};
    case kPlayerSlotYellow:
        return Color{1.0f, 1.0f, 0.0f, 1.0f};
    default:
        return Color{1.0f, 1.0f, 1.0f, 1.0f};
    }
}

void TnlGem::Update(TnlGems *pGems, GfxTunnel *pTunnel, const TnlTrackRange *pRange, float flTick) {
    TnlGeom *pGeom = pTunnel->mGeom;
    if ((static_cast<unsigned char>(mType) & kTypeSprite) == 0) {
        bool bPlace;
        if (mMeshGroup == nullptr) {
            mMeshGroup = pGems->GetMeshGroup(mType);
            mMeshGroup->Bind(this);
            bPlace = true;
        } else {
            bPlace = (pRange != nullptr && pRange->Contains(mTrack, mTick));
        }
        if (bPlace) {
            const float flLateral = Lateral();
            const char nTrack = mTrack;
            Transform &xfm = *mTransform;
            pGeom->CellXfm(nTrack, &xfm, false, pTunnel->IsTrackOpen(nTrack), mTick, flLateral);
            if (mParticle != nullptr) {
                const Vector3 raise{xfm.mBasisZ.x * sParticleHeight,
                                    xfm.mBasisZ.y * sParticleHeight,
                                    xfm.mBasisZ.z * sParticleHeight};
                mParticle->mPos.x = xfm.mTranslation.x + raise.x;
                mParticle->mPos.z = xfm.mTranslation.z + raise.z;
                mParticle->mPos.y = xfm.mTranslation.y + raise.y;
            }
        }
        MeshGroup *pGroup = mMeshGroup;
        while (pGroup != nullptr && !(flTick + pGroup->mLookahead < mTick)) {
            pGroup = pGroup->mNext;
        }
        if (pGroup != nullptr && pGroup != mMeshGroup) {
            pGroup->TakeGem(mMeshGroup, this);
            mMeshGroup = pGroup;
        }
        return;
    }

    if (mSpriteGroup == nullptr) {
        mSpriteGroup = pGems->GetSpriteGroup(mType);
        mParticle = mSpriteGroup->mParticles->AllocParticle();
        if (mParticle == nullptr) {
            mSpriteGroup = nullptr;
            return;
        }
        const float flLateral = Lateral();
        const char nTrack = mTrack;
        Transform xfm;
        pGeom->PlaceCell(
            nTrack, &xfm, false, pTunnel->IsTrackOpen(nTrack), mTick, flLateral, sSpriteHeight);
        if ((mFlags & kFlagPlayerColor) != 0) {
            mParticle->mCol = PlayerColor(static_cast<char>(mFlags & kFlagColorMask));
        } else {
            mParticle->mCol = mSpriteGroup->mParticles->mStartColorLow;
        }
        mParticle->mPos = xfm.mTranslation;
        mParticle->mSize = mSpriteGroup->mParticles->mSizeLow;
    } else if (pRange != nullptr && pRange->Contains(mTrack, mTick)) {
        const float flLateral = Lateral();
        const char nTrack = mTrack;
        Transform xfm;
        pGeom->PlaceCell(
            nTrack, &xfm, false, pTunnel->IsTrackOpen(nTrack), mTick, flLateral, sSpriteHeight);
        mParticle->mPos = xfm.mTranslation;
    }
}

void TnlGem::Release(GfxTunnel *pTunnel) {
    if (mMeshGroup != nullptr) {
        if (pTunnel != nullptr) {
            pTunnel->OnGemRemoved(this);
        }
        mMeshGroup->Unbind(this);
        mMeshGroup = nullptr;
    } else if (mSpriteGroup != nullptr) {
        // Yes, the original leaves mParticle pointing at the freed particle.
        mSpriteGroup->mParticles->FreeParticle(mParticle);
        mSpriteGroup = nullptr;
    } else if (pTunnel != nullptr && (static_cast<unsigned char>(mType) & kTypeSprite) == 0) {
        pTunnel->OnGemRemoved(this);
    }
}

void TnlGem::Flash(GfxTunnel *pTunnel) {
    if (mMeshGroup != nullptr) {
        (void)pTunnel->Flash(&mTransform->mTranslation, sMeshFlashSize, sMeshFlashRate);
    } else if (mSpriteGroup != nullptr) {
        (void)pTunnel->Flash(&mParticle->mPos,
                             mParticle->mSize * sFlareFlashSize,
                             mParticle->mSize * sFlareFlashRate);
    }
}
