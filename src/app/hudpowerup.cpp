#include "app/hudpowerup.h"

#include "app/overlay.h"
#include "game/gamedb.h"
#include "game/gamelogic.h"
#include "gfx/gfxmanager.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/rndmanager.h"

namespace {

// The speed of the slide, in frames per unit of time.
constexpr float kSlideSpeed = 1.0f;

// The head-up display prefix of an online game.
constexpr char kOnlineHudPrefix[] = "HUD1";

template <typename T>
T *FindIndexed(const char *pszFormat, const char *pszHud, int nIndex) {
    return dynamic_cast<T *>(TheManager.Find(FormatString(pszFormat, pszHud, nIndex)));
}

} // namespace

HudPowerup::HudPowerup(int nIndex, RndView *pHudView)
    : mView(nullptr), mSlide(kHiddenFrame, nullptr, nullptr, kSlideSpeed) {
    mEnabled = 1;
    mHolding = TheGameDb->GetTutorial();
    const char *pszHud = TheGameDb->GetCommunity() == GameDb::kCommunityOnline ?
                             kOnlineHudPrefix :
                             Overlay::sHudPrefix;
    mSlide.SetAnim(FindIndexed<RndAnimatable>("%s pup%d hide.tnm", pszHud, nIndex));
    mMesh = FindIndexed<RndMesh>("%s pup%d.mesh", pszHud, nIndex);
    mView = FindIndexed<RndView>("%s pup%d.view", pszHud, nIndex);
    mName = FindIndexed<RndText>("%s pup%d name.txt", pszHud, nIndex);
    mIcons[kIconAutocatcher] = FindIndexed<RndView>("%s pup auto%d.view", pszHud, nIndex);
    mIcons[kIconSlowdown] = FindIndexed<RndView>("%s pup slow%d.view", pszHud, nIndex);
    mIcons[kIconBumper] = FindIndexed<RndView>("%s pup bump%d.view", pszHud, nIndex);
    mIcons[kIconCrippler] = FindIndexed<RndView>("%s pup crip%d.view", pszHud, nIndex);
    mIcons[kIconFreestyle] = FindIndexed<RndView>("%s pup free%d.view", pszHud, nIndex);
    mIcons[kIconMultiplier] = FindIndexed<RndView>("%s pup mult%d.view", pszHud, nIndex);
    mMesh->SetShowing(false);
    if (TheGameDb->GetCommunity() == GameDb::kCommunityOnline) {
        auto *pFirstMesh = dynamic_cast<RndMesh *>(
            TheManager.Find(FormatString("%s pup0.mesh", Overlay::sHudPrefix)));
        auto *pLetterbox = dynamic_cast<RndTransformable *>(
            TheManager.Find(FormatString("%s letterbox scale all.view", Overlay::sHudPrefix)));
        auto *pPanel = FindIndexed<RndTransformable>("%s pup%d panel.mesh", pszHud, nIndex);
        static_cast<RndDrawable *>(pHudView)->AddDraw(mMesh, pFirstMesh);
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
    static_cast<RndDrawable *>(mView)->ClearDraws();
    mView->RemoveAllAnims();
    static_cast<RndTransformable *>(mView)->ClearTrans();
    RndView *pIcon = nullptr;
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
        if (TheGameDb->GetTutorial() == 0) {
            mHolding = 0;
            UpdateSlide();
        }
        mName->SetShowing(false);
        return;
    }
    if (TheGameDb->GetTutorial() == 0) {
        mHolding = 1;
        UpdateSlide();
    }
    RndDrawable *pIconDrawable = pIcon;
    pIconDrawable->SetShowing(true);
    static_cast<RndDrawable *>(mView)->AddDraw(pIconDrawable, nullptr);
    static_cast<RndTransformable *>(mView)->AddTrans(pIcon);
    mView->AddAnim(pIcon);
    mName->SetText(TheLocale.Localize(pszLabel, true));
    mName->SetShowing(true);
}

void HudPowerup::Poll(float fDelta, [[maybe_unused]] float fUnused, float fDeltaTicks) {
    if (fDelta == 0.0f) {
        fDelta = fDeltaTicks / *TheGfxManager.GetMsPerTick();
    }
    mSlide.Update(fDelta, false);
    mMesh->SetShowing(mSlide.GetValue() != kHiddenFrame);
}

void HudPowerup::UpdateSlide() {
    mSlide.SetTarget(mHolding != 0 && mEnabled != 0 ? 0.0f : kHiddenFrame);
}
