#include "gfx/tnlstreakgemfx.h"

#include <cstring>

#include "game/gamedb.h"
#include "gfx/gfxmanager.h"
#include "gfx/tnlgem.h"
#include "os/string.h"
#include "rnd/font.h"
#include "rnd/manager.h"
#include "rnd/mesh.h"

namespace {

// The tick of an arrow that has no gem.
constexpr float kNoTick = -1e9f;

// The colour letter of the materials of a solo game.
constexpr char kSoloColor = 's';

// A shown arrow vanishes at once at this rate.
constexpr float kVanishRate = -1.0f;

} // namespace

float TnlStreakGemFX::sBlinkRate = 0.001f;
char TnlStreakGemFX::sPendingText[16];
unsigned char TnlStreakGemFX::sShownMask = 0;
LinearInterpolator TnlStreakGemFX::sDistanceFade(0.0f, 1.0f, 120.0f, 600.0f);

TnlStreakGemFX::TnlStreakGemFX(int nTrack, int nPlayer) {
    mAlpha = 0.0f;
    mBlinkRate = 0.0f;
    mSlot = 0;
    mMovePending = 0;
    mTrack = nTrack;
    mBit = static_cast<unsigned char>(1 << nTrack);
    mTick = kNoTick;
    mView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find("streakarrow.view"));
    mText = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find("streakarrow.txt"));
    Rnd::Mesh *pMesh = dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find("streakarrow.mesh"));
    const char nColor = (TheGameDb->mCommunity == GameDb::kCommunitySolo) ?
                            kSoloColor :
                            TheGameDb->GetPlayerColor(nPlayer)[0];
    mMat =
        dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(FormatString("streakarrow_%c.mat", nColor)));
    mFontMat = dynamic_cast<Rnd::Mat *>(
        Rnd::TheManager.Find(FormatString("streakarrow font_%c.mat", nColor)));
    pMesh->SetMat(mMat);
    Rnd::Font *pFont = mText->GetFont();
    pFont->SetAtlas(
        mFontMat, pFont->mChars, pFont->mRows, pFont->mCols, pFont->mSize, pFont->mSpace);
}

TnlStreakGemFX::~TnlStreakGemFX() = default;

void TnlStreakGemFX::Hide(bool bNow) {
    if (!bNow) {
        mBlinkRate = -sBlinkRate;
        return;
    }
    sShownMask &= static_cast<unsigned char>(~mBit);
    mBlinkRate = kVanishRate;
}

void TnlStreakGemFX::Show(float flTick, int nSlot) {
    mMovePending = 1;
    mTick = flTick;
    mSlot = nSlot;
    if (0.0f <= mBlinkRate) {
        mBlinkRate = -sBlinkRate;
    }
}

void TnlStreakGemFX::SetText(const char *pszText) {
    if (sShownMask == 0) {
        mText->SetText(pszText);
        return;
    }
    std::strncpy(sPendingText, pszText, sizeof(sPendingText) - 1);
    sPendingText[sizeof(sPendingText) - 1] = '\0';
}

void TnlStreakGemFX::Refresh(GfxTunnel *pTunnel) {
    if ((sShownMask & mBit) == 0) {
        return;
    }
    if (pTunnel->mChangedRange.Contains(static_cast<char>(mTrack), mTick)) {
        Place(pTunnel);
    }
}

void TnlStreakGemFX::Poll(float flDelta, GfxTunnel *pTunnel) {
    mAlpha += flDelta * mBlinkRate;
    if (!(mAlpha <= 0.0f)) {
        sShownMask |= mBit;
        if (1.0f < mAlpha) {
            mAlpha = 1.0f;
        }
        return;
    }
    sShownMask &= static_cast<unsigned char>(~mBit);
    int bMove = mMovePending;
    if (sPendingText[0] != '\0') {
        if (sShownMask == 0) {
            mText->SetText(sPendingText);
            sPendingText[0] = '\0';
        } else {
            bMove = 0;
        }
    }
    if (!bMove) {
        mAlpha = 0.0f;
        return;
    }
    const float flStep = sBlinkRate * flDelta;
    mAlpha = (flStep < 1.0f) ? flStep : 1.0f;
    mMovePending = 0;
    mBlinkRate = sBlinkRate;
    Place(pTunnel);
}

void TnlStreakGemFX::Draw() {
    if (mAlpha == 0.0f) {
        return;
    }
    const float flAlpha = sDistanceFade.Eval(mTick - TheGameDb->mSongTick) * mAlpha;
    if (!(0.0f < flAlpha)) {
        return;
    }
    mFontMat->SetAlpha(flAlpha);
    Color emissive = mMat->mEmissive;
    emissive.a = flAlpha;
    mMat->SetEmissive(emissive);
    mMat->SetAlpha(flAlpha);
    mView->SetLocalXfm(mXfm);
    mView->UpdateWorldXfm(nullptr, 0);
    mView->Draw();
}

void TnlStreakGemFX::Place(GfxTunnel *pTunnel) {
    const float flLateral = (static_cast<float>(mSlot) * TnlGem::kSlotWidth) + TnlGem::kSlotOffset;
    pTunnel->mGeom->CellXfm(mTrack, &mXfm, false, true, mTick, flLateral);
    const float flScale = TheGfxManager.mScrollSpeed;
    Vector3 *rows[] = {&mXfm.mBasisX, &mXfm.mBasisY, &mXfm.mBasisZ};
    for (Vector3 *pRow : rows) {
        pRow->x *= flScale;
        pRow->y *= flScale;
        pRow->z *= flScale;
    }
}
