#include "app/ovyavatar.h"

#include "app/overlay.h"
#include "game/avatarcam.h"
#include "game/gamedb.h"
#include "game/gameoptions.h"
#include "gfx/gfxmanager.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/manager.h"

namespace {

// The swing runs from 0 to kSwingEnd, and the avatar swaps once it passes kSwingSwap.
constexpr float kSwingEnd = 500.0f;
constexpr float kSwingSwap = 300.0f;

// The speed of the swing before Reset() reads sLeaderSwingTime.
constexpr float kInitialSwingSpeed = 0.1f;

// The alpha of the name under an avatar, and of the leader label.
constexpr float kNameAlpha = 0.75f;
constexpr float kLeaderLabelAlpha = 1.0f;

// The name and colour a display shows with no player.
const Color kNoNameColor{0.0f, 0.0f, 0.0f, 1.0f};
const Color kNoBackgroundColor{1.0f, 1.0f, 1.0f, 1.0f};

// The labels of the instruments, indexed by GfxManager::Instrument.
const char *const kInstrumentLabels[] = {
    "DRUM_LABEL",
    "BASS_LABEL",
    "SYNTH_LABEL",
    "GUITAR_LABEL",
    "VOCAL_LABEL",
    "FX_LABEL",
};

// Stop the animations of every avatar of the game.
void ClearAllAnims(AvatarPartSet *pSoloAvatar) {
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        pSoloAvatar->ClearAnims();
        return;
    }
    for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
        TheGameDb->GetAvatar(i)->ClearAnims();
    }
}

} // namespace

float OvyAvatar::sLeaderSwingTime = 250.0f;
Vector2 OvyAvatar::sDepthRange{0.02f, 0.05f};

OvyAvatar::OvyAvatar(int nPlayer, int nIndex, Rnd::View *pHudView)
    : HideablePanel(FormatString("%s freq %d.tnm", Overlay::sHudPrefix, nIndex), nullptr, false),
      mWanted(0), mPlayer(kNoPlayer), mAvatar(nullptr), mView(nullptr), mName(nullptr),
      mNameColor(kNoNameColor), mBackgroundColor(kNoBackgroundColor), mNameMat(nullptr),
      mSwing(0.0f, nullptr, nullptr, kInitialSwingSpeed), mNextPlayer(kNoChange) {
    mPlaceholder = dynamic_cast<Rnd::Mesh *>(
        Rnd::TheManager.Find(FormatString("freq placeholder %d.mesh", nIndex)));
    mView = dynamic_cast<Rnd::View *>(
        Rnd::TheManager.Find(FormatString("%s freq %d.view", Overlay::sHudPrefix, nIndex)));
    if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel ||
        TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        mName = dynamic_cast<Rnd::Text *>(
            Rnd::TheManager.Find(FormatString("%s track%d.txt", Overlay::sHudPrefix, nIndex)));
    }
    mBackgroundMat = dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find("HUD freq_background.mat"));
    if (nPlayer < 0 || TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        mBackgroundColor = mBackgroundMat->mAmbient;
    } else {
        mBackgroundColor = *TheGfxManager.GetPlayerColor(nPlayer);
    }
    mNameMat = dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find("HUD freq track.mat"));
    if (mName != nullptr) {
        mName->SetText("");
        mNameColor = mNameMat->mAmbient;
        mNameColor.a = kNameAlpha;
    }
    static_cast<Rnd::Drawable *>(pHudView)->RemoveDraw(mView);
    static_cast<Rnd::Drawable *>(mView)->SetShowing(true);
    if (TheGameDb->mCommunity != GameDb::kCommunitySolo &&
        TheGameDb->mRuleSet == GameDb::kRuleSetGame) {
        auto *pSwingAnim = dynamic_cast<Rnd::Animatable *>(
            Rnd::TheManager.Find(FormatString("%s freq %d fx.tnm", Overlay::sHudPrefix, nIndex)));
        if (pSwingAnim != nullptr) {
            mSwing.SetAnim(pSwingAnim);
            mSwing.SetSpeed(kSwingEnd / sLeaderSwingTime);
            mSwing.Jump(kSwingEnd, kSwingEnd);
        }
    }
    if (nPlayer >= 0) {
        mAvatar = TheGameDb->GetAvatar(nPlayer);
        mPlayer = static_cast<signed char>(nPlayer);
    } else {
        mPlaceholder->SetShowing(false);
        auto *pLeaderLabel = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find("HUDm lead0.txt"));
        pLeaderLabel->SetText(TheLocale.Localize("LEADER_AVATAR_LABEL", true));
        mNameColor.a = kLeaderLabelAlpha;
    }
    Reset();
    SetAvatarDepthRange(sDepthRange.x, sDepthRange.y);
}

OvyAvatar::~OvyAvatar() {
    ClearAllAnims(mAvatar);
    SnapAvatarCam("bust");
}

void OvyAvatar::UpdateShown() {
    HideablePanel::Show(mWanted != 0 && IsAvatarShown());
}

void OvyAvatar::Draw() {
    if (mSlide.mValue == 0.0f) {
        if (mPlayer >= 0 && TheGfxManager.mWinner == mPlayer) {
            mAvatar->Render(nullptr);
            TheGfxManager.mWinnerDrawn = 1;
        }
        return;
    }
    if (mAvatar != nullptr) {
        mAvatar->Render(nullptr);
        TheGfxManager.mWinnerDrawn = TheGfxManager.mWinner == mPlayer;
    }
    if (mNameMat != nullptr) {
        mNameMat->SetAmbient(mNameColor);
        mNameMat->SetAlpha(mNameColor.a);
    }
    mBackgroundMat->SetAmbient(mBackgroundColor);
    static_cast<Rnd::Drawable *>(mView)->Draw();
}

void OvyAvatar::Poll(float fDelta) {
    HideablePanel::Poll();
    if (!mSwing.Update(fDelta, false) || mNextPlayer == kNoChange ||
        !(kSwingSwap <= mSwing.mValue)) {
        return;
    }
    mPlayer = mNextPlayer;
    if (mNextPlayer < 0) {
        mPlaceholder->SetShowing(false);
        if (mName != nullptr) {
            mName->SetText("");
        }
        mAvatar = nullptr;
        mNameColor = kNoNameColor;
        mBackgroundColor = kNoBackgroundColor;
    } else {
        mAvatar = TheGameDb->GetAvatar(mNextPlayer);
        mPlaceholder->SetShowing(mAvatar != nullptr);
        mNameColor = *TheGfxManager.GetPlayerBgColor(mPlayer);
        mBackgroundColor = *TheGfxManager.GetPlayerColor(mPlayer);
        if (mName != nullptr) {
            mName->SetText(TheGameDb->GetPlayerName(mPlayer));
        }
    }
    mNextPlayer = kNoChange;
}

void OvyAvatar::SetPlayer(int nPlayer) {
    const signed char nNext = nPlayer < 0 ? kNoPlayer : static_cast<signed char>(nPlayer);
    if (nNext == mPlayer) {
        return;
    }
    mNextPlayer = nNext;
    mSwing.Jump(0.0f, kSwingEnd);
}

void OvyAvatar::SetFreqSize(int nFreqSize) {
    SetAvatarFreqSize(nFreqSize, false);
    UpdateShown();
}

void OvyAvatar::Reset() {
    ClearAllAnims(mAvatar);
    SetFreqSize(TheGameDb->GetOptions()->mFreqSize);
    if (mSwing.mAnim == nullptr) {
        return;
    }
    mNextPlayer = kNoPlayer;
    mPlayer = kNoPlayer;
    mSwing.Jump(kSwingEnd, kSwingEnd);
    mPlaceholder->SetShowing(false);
    if (mName != nullptr) {
        mName->SetText("");
    }
    mAvatar = nullptr;
    mNameColor = kNoNameColor;
    mBackgroundColor = kNoBackgroundColor;
}

void OvyAvatar::SetInstrument([[maybe_unused]] int nPlayer,
                              int nInstrument,
                              [[maybe_unused]] int nTrackKind) {
    if (mName == nullptr) {
        return;
    }
    const char *pszLabel = "";
    if (nInstrument >= GfxManager::kInstrumentDrum && nInstrument < GfxManager::kNumInstruments) {
        pszLabel = TheLocale.Localize(kInstrumentLabels[nInstrument], true);
    }
    mNameColor = *TheGfxManager.GetInstrumentBgColor(nInstrument);
    mName->SetText(pszLabel);
}
