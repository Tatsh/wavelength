#include "app/hudpowerup.h"

#include "app/overlay.h"
#include "game/gamedb.h"
#include "game/gamelogic.h"
#include "gfx/gfxmanager.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/manager.h"

namespace {

// The speed of the slide, in frames per unit of time.
constexpr float kSlideSpeed = 1.0f;

// The head-up display prefix of an online game.
constexpr char kOnlineHudPrefix[] = "HUD1";

template <typename T>
T *FindIndexed(const char *pszFormat, const char *pszHud, int nIndex) {
    return dynamic_cast<T *>(Rnd::TheManager.Find(FormatString(pszFormat, pszHud, nIndex)));
}

} // namespace

HudPowerup::HudPowerup(int nIndex, Rnd::View *pHudView)
    : mView(nullptr), mSlide(kHiddenFrame, nullptr, nullptr, kSlideSpeed) {
    mEnabled = 1;
    mHolding = TheGameDb->mTutorial;
    const char *pszHud =
        TheGameDb->mCommunity == GameDb::kCommunityOnline ? kOnlineHudPrefix : Overlay::sHudPrefix;
    mSlide.SetAnim(FindIndexed<Rnd::Animatable>("%s pup%d hide.tnm", pszHud, nIndex));
    mMesh = FindIndexed<Rnd::Mesh>("%s pup%d.mesh", pszHud, nIndex);
    mView = FindIndexed<Rnd::View>("%s pup%d.view", pszHud, nIndex);
    mName = FindIndexed<Rnd::Text>("%s pup%d name.txt", pszHud, nIndex);
    mIcons[kIconAutocatcher] = FindIndexed<Rnd::View>("%s pup auto%d.view", pszHud, nIndex);
    mIcons[kIconSlowdown] = FindIndexed<Rnd::View>("%s pup slow%d.view", pszHud, nIndex);
    mIcons[kIconBumper] = FindIndexed<Rnd::View>("%s pup bump%d.view", pszHud, nIndex);
    mIcons[kIconCrippler] = FindIndexed<Rnd::View>("%s pup crip%d.view", pszHud, nIndex);
    mIcons[kIconFreestyle] = FindIndexed<Rnd::View>("%s pup free%d.view", pszHud, nIndex);
    mIcons[kIconMultiplier] = FindIndexed<Rnd::View>("%s pup mult%d.view", pszHud, nIndex);
    mMesh->SetShowing(false);
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        auto *pFirstMesh = dynamic_cast<Rnd::Mesh *>(
            Rnd::TheManager.Find(FormatString("%s pup0.mesh", Overlay::sHudPrefix)));
        auto *pLetterbox = dynamic_cast<Rnd::Transformable *>(
            Rnd::TheManager.Find(FormatString("%s letterbox scale all.view", Overlay::sHudPrefix)));
        auto *pPanel = FindIndexed<Rnd::Transformable>("%s pup%d panel.mesh", pszHud, nIndex);
        static_cast<Rnd::Drawable *>(pHudView)->AddDraw(mMesh, pFirstMesh);
        pHudView->AddAnim(mView);
        pLetterbox->AddTrans(pPanel);
    }
    Reset();
}

void HudPowerup::Reset() {
    ShowPowerup(GameLogic::kPowerupNone);
    mSlide.Jump(kHiddenFrame, kHiddenFrame);
    mSlide.Update(0.0f, true);
}

void HudPowerup::ShowPowerup(int nPowerup) {
    static_cast<Rnd::Drawable *>(mView)->RemoveAllDraws();
    mView->RemoveAllAnims();
    static_cast<Rnd::Transformable *>(mView)->RemoveAllTranses();
    Rnd::View *pIcon = nullptr;
    const char *pszLabel = nullptr;
    switch (nPowerup) {
    case GameLogic::kPowerupAutocatcher:
        pIcon = mIcons[kIconAutocatcher];
        pszLabel = "AUTOCATCHER_LABEL";
        break;
    case GameLogic::kPowerupMultiplier:
        pIcon = mIcons[kIconMultiplier];
        pszLabel = "MULTIPLIER_LABEL";
        break;
    case GameLogic::kPowerupSlowdown:
        pIcon = mIcons[kIconSlowdown];
        pszLabel = "SLOWDOWN_LABEL";
        break;
    case GameLogic::kPowerupFreestyle:
        pIcon = mIcons[kIconFreestyle];
        pszLabel = "FREESTYLE_LABEL";
        break;
    case GameLogic::kPowerupBumper:
        pIcon = mIcons[kIconBumper];
        pszLabel = "BUMPER_LABEL";
        break;
    case GameLogic::kPowerupCrippler:
        pIcon = mIcons[kIconCrippler];
        pszLabel = "CRIPPLER_LABEL";
        break;
    default:
        break;
    }
    if (pIcon == nullptr) {
        if (TheGameDb->mTutorial == 0) {
            mHolding = 0;
            UpdateSlide();
        }
        mName->SetShowing(false);
        return;
    }
    if (TheGameDb->mTutorial == 0) {
        mHolding = 1;
        UpdateSlide();
    }
    Rnd::Drawable *pIconDrawable = pIcon;
    pIconDrawable->SetShowing(true);
    static_cast<Rnd::Drawable *>(mView)->AddDraw(pIconDrawable, nullptr);
    static_cast<Rnd::Transformable *>(mView)->AddTrans(pIcon);
    mView->AddAnim(pIcon);
    mName->SetText(TheLocale.Localize(pszLabel, true));
    mName->SetShowing(true);
}

void HudPowerup::Poll(float fDelta, [[maybe_unused]] float fUnused, float fDeltaTicks) {
    if (fDelta == 0.0f) {
        fDelta = fDeltaTicks / *TheGfxManager.mMsPerTick;
    }
    mSlide.Update(fDelta, false);
    mMesh->SetShowing(mSlide.mValue != kHiddenFrame);
}

void HudPowerup::UpdateSlide() {
    mSlide.SetTarget(mHolding != 0 && mEnabled != 0 ? 0.0f : kHiddenFrame);
}
