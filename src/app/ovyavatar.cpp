#include "app/ovyavatar.h"

#include "app/overlay.h"
#include "game/avatar.h"
#include "game/gamedb.h"
#include "game/gameoptions.h"
#include "gfx/gfxmanager.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/rndmanager.h"

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

// Stop the animations of every avatar of the game.
void ClearAllAnims(AvatarPartSet *pSoloAvatar) {
    if (TheGameDb->GetCommunity() == GameDb::kCommunitySolo) {
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

OvyAvatar::OvyAvatar(int nPlayer, int nIndex, RndView *pHudView)
    : HideablePanel(FormatString("%s freq %d.tnm", Overlay::sHudPrefix, nIndex), nullptr, false),
      mWanted(0), mPlayer(kNoPlayer), mAvatar(nullptr), mView(nullptr), mName(nullptr),
      mNameColor(kNoNameColor), mBackgroundColor(kNoBackgroundColor), mNameMat(nullptr),
      mSwing(0.0f, nullptr, nullptr, kInitialSwingSpeed), mNextPlayer(kNoChange) {
    mPlaceholder =
        dynamic_cast<RndMesh *>(TheManager.Find(FormatString("freq placeholder %d.mesh", nIndex)));
    mView = dynamic_cast<RndView *>(
        TheManager.Find(FormatString("%s freq %d.view", Overlay::sHudPrefix, nIndex)));
    if (TheGameDb->GetRuleSet() == GameDb::kRuleSetDuel ||
        TheGameDb->GetCommunity() == GameDb::kCommunitySolo) {
        mName = dynamic_cast<RndText *>(
            TheManager.Find(FormatString("%s track%d.txt", Overlay::sHudPrefix, nIndex)));
    }
    mBackgroundMat = dynamic_cast<RndMat *>(TheManager.Find("HUD freq_background.mat"));
    if (nPlayer < 0 || TheGameDb->GetCommunity() == GameDb::kCommunitySolo) {
        mBackgroundColor = mBackgroundMat->GetBaseColor();
    } else {
        mBackgroundColor = *TheGfxManager.GetPlayerColor(nPlayer);
    }
    mNameMat = dynamic_cast<RndMat *>(TheManager.Find("HUD freq track.mat"));
    if (mName != nullptr) {
        mName->SetText("");
        mNameColor = mNameMat->GetBaseColor();
        mNameColor.a = kNameAlpha;
    }
    static_cast<RndDrawable *>(pHudView)->RemoveDraw(mView);
    static_cast<RndDrawable *>(mView)->SetShowing(true);
    if (TheGameDb->GetCommunity() != GameDb::kCommunitySolo &&
        TheGameDb->GetRuleSet() == GameDb::kRuleSetGame) {
        auto *pSwingAnim = dynamic_cast<RndAnimatable *>(
            TheManager.Find(FormatString("%s freq %d fx.tnm", Overlay::sHudPrefix, nIndex)));
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
        auto *pLeaderLabel = dynamic_cast<RndText *>(TheManager.Find("HUDm lead0.txt"));
        pLeaderLabel->SetText(TheLocale.Localize("LEADER_AVATAR_LABEL", true));
        mNameColor.a = kLeaderLabelAlpha;
    }
    Reset();
    Avatar::SetDepthRange(sDepthRange.x, sDepthRange.y);
}

OvyAvatar::~OvyAvatar() {
    ClearAllAnims(mAvatar);
    Avatar::SnapCamera("bust");
}

void OvyAvatar::UpdateShown() {
    const bool bAvatarShown = Avatar::IsShown();
    HideablePanel::Show(mWanted != 0 && bAvatarShown);
}

void OvyAvatar::Draw() {
    if (mSlide.GetValue() == 0.0f) {
        if (mPlayer >= 0 && TheGfxManager.GetWinner() == mPlayer) {
            mAvatar->Render(nullptr);
            TheGfxManager.SetWinnerDrawn(1);
        }
        return;
    }
    if (mAvatar != nullptr) {
        mAvatar->Render(nullptr);
        TheGfxManager.SetWinnerDrawn(TheGfxManager.GetWinner() == mPlayer);
    }
    if (mNameMat != nullptr) {
        mNameMat->SetBaseColor(mNameColor);
        mNameMat->SetAlpha(mNameColor.a);
    }
    mBackgroundMat->SetBaseColor(mBackgroundColor);
    static_cast<RndDrawable *>(mView)->Draw();
}

void OvyAvatar::Poll(float fDelta) {
    HideablePanel::Poll();
    if (!mSwing.Update(fDelta, false) || mNextPlayer == kNoChange ||
        !(kSwingSwap <= mSwing.GetValue())) {
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
    Avatar::SetFreqSize(nFreqSize, false);
    UpdateShown();
}

void OvyAvatar::Reset() {
    ClearAllAnims(mAvatar);
    SetFreqSize(TheGameDb->GetOptions()->GetFreqSize());
    if (mSwing.GetAnim() == nullptr) {
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
    const char *pszLabel;
    switch (nInstrument) {
    case GfxManager::kInstrumentDrum:
        pszLabel = TheLocale.Localize("DRUM_LABEL", true);
        break;
    case GfxManager::kInstrumentBass:
        pszLabel = TheLocale.Localize("BASS_LABEL", true);
        break;
    case GfxManager::kInstrumentSynth:
        pszLabel = TheLocale.Localize("SYNTH_LABEL", true);
        break;
    case GfxManager::kInstrumentGuitar:
        pszLabel = TheLocale.Localize("GUITAR_LABEL", true);
        break;
    case GfxManager::kInstrumentVocal:
        pszLabel = TheLocale.Localize("VOCAL_LABEL", true);
        break;
    case GfxManager::kInstrumentFx:
        pszLabel = TheLocale.Localize("FX_LABEL", true);
        break;
    default:
        pszLabel = "";
        break;
    }
    mNameColor = *TheGfxManager.GetInstrumentBgColor(nInstrument);
    mName->SetText(pszLabel);
}
