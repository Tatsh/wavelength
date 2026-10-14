#include "app/ovycontroller.h"

#include "app/overlay.h"
#include "os/string.h"
#include "rnd/rndmanager.h"

namespace {

// The scene names of the buttons, indexed by JoypadButton.
const char *const kButtonNames[] = {
    "l2",
    "r2",
    "l1",
    "r1",
    "triangle",
    "circle",
    "x",
    "square",
    "select",
    "l3",
    "r3",
    "start",
    "dpadu",
    "dpadr",
    "dpadd",
    "dpadl",
};

} // namespace

void OvyController::Button::Load(const char *pszName) {
    mAnim = dynamic_cast<RndAnimatable *>(
        TheManager.Find(FormatString("HUD controller map %s.tnm", pszName)));
    mMesh = dynamic_cast<RndDrawable *>(
        TheManager.Find(FormatString("HUD controller map %s.mesh", pszName)));
}

OvyController::OvyController(RndView *pHudView) : HideablePanel(nullptr, nullptr, false) {
    for (Button &button : mButtons) {
        button.mAnim = nullptr;
        button.mMesh = nullptr;
    }
    mView = dynamic_cast<RndView *>(TheManager.Find("HUD controller map.view"));
    mMats[kHighlightOff] = dynamic_cast<RndMat *>(TheManager.Find("HUD controller map no.mat"));
    mMats[kHighlightOn] = dynamic_cast<RndMat *>(TheManager.Find("HUD controller map hi.mat"));
    mMesh = dynamic_cast<RndMesh *>(TheManager.Find("HUD controller map.mesh"));
    mPosAnim = dynamic_cast<RndAnimatable *>(TheManager.Find("HUD controller map pos.tnm"));
    SetObjects("HUD controller map.tnm", "HUD controller map.view", false);
    auto *pMessage = dynamic_cast<RndDrawable *>(TheManager.Find("HUD genmsg.txt"));
    auto *pLetterbox = dynamic_cast<RndTransformable *>(
        TheManager.Find(FormatString("%s letterbox scale all.view", Overlay::sHudPrefix)));
    static_cast<RndDrawable *>(pHudView)->AddDraw(mView, pMessage);
    pLetterbox->AddTrans(mView);
    pHudView->AddAnim(mView);
    for (int i = 0; i < kNumButtons; ++i) {
        mButtons[i].Load(kButtonNames[i]);
    }
    Reset();
}

void OvyController::Reset() {
    mView->RemoveAllAnims();
    SetHighlight(false);
    ShowNow(false);
    for (Button &button : mButtons) {
        button.mMesh->SetShowing(false);
    }
    mPosAnim->SetFrame(0.0f);
}

void OvyController::SetHighlight(bool bHighlight) {
    mMesh->SetMat(mMats[bHighlight ? kHighlightOn : kHighlightOff]);
}

void OvyController::ShowButton(int nButton, bool bShow) {
    Button &button = mButtons[nButton];
    button.mMesh->SetShowing(bShow);
    mView->RemoveAnim(button.mAnim);
    if (bShow) {
        mView->AddAnim(button.mAnim);
    }
}
