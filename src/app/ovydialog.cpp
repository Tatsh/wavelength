#include "app/ovydialog.h"

#include "app/overlay.h"
#include "os/string.h"
#include "rnd/manager.h"

OvyDialog::OvyDialog(Rnd::View *pHudView) : HideablePanel(nullptr, nullptr, false) {
    auto *pView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find("HUD dialog.view"));
    mText = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find("HUD dialog.txt"));
    mSizeAnim = dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find("HUD dialog panel size.msnm"));
    mPosAnim = dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find("HUD dialog pos.tnm"));
    auto *pMessage = dynamic_cast<Rnd::Drawable *>(Rnd::TheManager.Find("HUD genmsg.txt"));
    auto *pLetterbox = dynamic_cast<Rnd::Transformable *>(
        Rnd::TheManager.Find(FormatString("%s letterbox scale all.view", Overlay::sHudPrefix)));
    static_cast<Rnd::Drawable *>(pHudView)->AddDraw(pView, pMessage);
    pLetterbox->AddTrans(pView);
    SetObjects("HUD dialog.tnm", "HUD dialog.view", false);
    ShowNow(false);
    SetPosition(0.0f);
}

void OvyDialog::Open(const char *pszText, float fSize, float fPosition) {
    mText->SetText(pszText);
    mSizeAnim->SetFrame(fSize);
    SetPosition(fPosition);
    Show(true);
}

void OvyDialog::Close() {
    Show(false);
}
