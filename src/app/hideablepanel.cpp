#include "app/hideablepanel.h"

#include "os/system.h"
#include "rnd/manager.h"
#include "script/dataarray.h"

namespace {

// The speed of a slide, in frames per millisecond.
constexpr float kSlideSpeed = 0.5f;

} // namespace

int HideablePanel::sSnap = 0;
float HideablePanel::sDeltaTime = 0.0f;
Rnd::Animatable *HideablePanel::sMaterialAnim = nullptr;
float HideablePanel::sMaterialFrame = 0.0f;

HideablePanel::HideablePanel(const char *pszAnim, const char *pszDrawable, bool bShown)
    : mSlide(0.0f, nullptr, nullptr, kSlideSpeed) {
    mDrawable = nullptr;
    SetObjects(pszAnim, pszDrawable, bShown);
}

void HideablePanel::SetObjects(const char *pszAnim, const char *pszDrawable, bool bShown) {
    if (pszAnim != nullptr) {
        mSlide.SetAnim(dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find(pszAnim)));
    }
    if (pszDrawable != nullptr) {
        mDrawable = dynamic_cast<Rnd::Drawable *>(Rnd::TheManager.Find(pszDrawable));
    }
    ShowNow(bShown);
}

void HideablePanel::Show(bool bShow) {
    if (sSnap != 0) {
        ShowNow(bShow);
        return;
    }
    mSlide.SetTarget(bShow ? kShownFrame : 0.0f);
}

void HideablePanel::ShowNow(bool bShow) {
    const float fFrame = bShow ? kShownFrame : 0.0f;
    mSlide.Jump(fFrame, fFrame);
}

void HideablePanel::Poll() {
    mSlide.Update(sDeltaTime, false);
    if (mDrawable != nullptr) {
        mDrawable->SetShowing(mSlide.mValue != 0.0f);
    }
}

void HideablePanel::ClearSnap() {
    sSnap = 0;
}

void HideablePanel::SetDeltaTime(float fDeltaTime) {
    sDeltaTime = fDeltaTime;
}

void HideablePanel::Init() {
    DataArray *pUi = SystemConfig()->FindArray("ui", false);
    if (pUi != nullptr) {
        pUi->FindArray("panel_enter_exit", true)
            ->FindFloat("enter_stop_frame", &sMaterialFrame, true);
    }
    sMaterialAnim = dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find("mat_2d_EE.anim"));
    ClearSnap();
}

void HideablePanel::Terminate() {
    sMaterialAnim = nullptr;
}

void HideablePanel::PoseMaterials() {
    if (sMaterialAnim != nullptr) {
        sMaterialAnim->SetFrame(sMaterialFrame);
    }
}
