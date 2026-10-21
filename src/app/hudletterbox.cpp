#include "app/hudletterbox.h"

#include "gfx/gfxmanager.h"
#include "rnd/rndmanager.h"

namespace {

// The speed of the bars, in frames per unit of time.
constexpr float kSlideSpeed = 1.0f / 4.8f;

// The frames of the slide per part of the way closed.
constexpr float kFramesPerPart = 100.0f;

} // namespace

HudLetterbox::HudLetterbox(RndView *pHudView) : mSlide(0.0f, nullptr, nullptr, kSlideSpeed) {
    mView = dynamic_cast<RndView *>(TheManager.Find("HUD letterbox.view"));
    static_cast<RndDrawable *>(pHudView)->RemoveDraw(mView);
    mSlide.SetAnim(mView);
    static_cast<RndDrawable *>(mView)->SetShowing(false);
}

void HudLetterbox::Poll(float fDelta, float fDeltaTicks) {
    if (fDelta == 0.0f) {
        fDelta = fDeltaTicks / *TheGfxManager.GetMsPerTick();
    }
    if (mSlide.Update(fDelta, false)) {
        static_cast<RndDrawable *>(mView)->SetShowing(mSlide.GetValue() != 0.0f);
    }
}

void HudLetterbox::SetClosed(float fClosed) {
    mSlide.SetTarget(fClosed * kFramesPerPart);
}
